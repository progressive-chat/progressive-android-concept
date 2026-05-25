#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "progressive/session_manager.hpp"
#include "progressive/sync_handler.hpp"
#include "progressive/room_list.hpp"
#include "progressive/notification.hpp"
#include "progressive/crash_reporter.hpp"
#include "progressive/profiler.hpp"
#include "progressive/config_loader.hpp"
#include "progressive/theme_manager.hpp"

#include "matrix_sdk/matrix_client.hpp"
#include "matrix_sdk/matrix_session.hpp"
#include "matrix_sdk/matrix_sync.hpp"

#include "ui_renderer.hpp"
#include "ui_input_handler.hpp"
#include "android_bridge.hpp"

#define LOG_TAG "ProgressiveApp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// ── Global application state ──
static struct AppState {
    android_app* androidApp = nullptr;
    bool initialized = false;
    bool animating = false;
    bool hasFocus = false;
    bool hasSurface = false;

    // Rendering
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    int32_t width = 0;
    int32_t height = 0;

    // Core modules
    std::unique_ptr<progressive::SessionManager> sessionManager;
    std::unique_ptr<progressive::SyncHandler> syncHandler;
    std::unique_ptr<progressive::NotificationManager> notificationManager;
    std::unique_ptr<progressive::CrashReporter> crashReporter;
    std::unique_ptr<progressive::ConfigLoader> configLoader;

    // UI
    std::unique_ptr<UIRenderer> renderer;
    std::unique_ptr<UIInputHandler> inputHandler;

    // Thread safety
    std::mutex mutex;
    std::condition_variable cv;
    bool destroyRequested = false;

    // Frame timing
    std::chrono::steady_clock::time_point lastFrameTime;
    double frameAccumulator = 0.0;
    const double frameTimeStep = 1.0 / 60.0;

} gApp;

// ── Forward declarations ──
static void handleCmd(struct android_app* app, int32_t cmd);
static int32_t handleInput(struct android_app* app, AInputEvent* event);
static bool initDisplay();
static void termDisplay();
static void renderFrame();
static void mainLoop();
static void initCoreModules();

// ── Native activity entry point ──
void android_main(struct android_app* app) {
    LOGI("android_main: Progressive Chat starting...");

    gApp.androidApp = app;
    app->userData = &gApp;
    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    // Wait for the window to be ready
    while (!gApp.destroyRequested) {
        int events;
        struct android_poll_source* source;
        int ident = ALooper_pollAll(gApp.hasSurface ? 0 : -1,
                                     nullptr, &events, (void**)&source);
        if (ident >= 0 && source) {
            source->process(app, source);
        }

        if (app->destroyRequested) {
            termDisplay();
            return;
        }

        if (gApp.hasSurface && !gApp.initialized) {
            if (initDisplay()) {
                initCoreModules();
                gApp.renderer = std::make_unique<UIRenderer>(gApp.width, gApp.height);
                gApp.inputHandler = std::make_unique<UIInputHandler>();
                gApp.initialized = true;
                gApp.lastFrameTime = std::chrono::steady_clock::now();
                LOGI("Application initialized. Starting main loop.");
            }
        }

        if (gApp.initialized && gApp.hasSurface && gApp.hasFocus) {
            mainLoop();
        }
    }

    termDisplay();
    LOGI("android_main: exiting.");
}

// ── Command handler ──
static void handleCmd(struct android_app* app, int32_t cmd) {
    AppState* state = reinterpret_cast<AppState*>(app->userData);

    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            LOGI("APP_CMD_INIT_WINDOW");
            state->hasSurface = true;
            break;

        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW");
            state->hasSurface = false;
            state->animating = false;
            break;

        case APP_CMD_GAINED_FOCUS:
            LOGI("APP_CMD_GAINED_FOCUS");
            state->hasFocus = true;
            state->animating = true;
            if (state->syncHandler) {
                state->syncHandler->resume();
            }
            break;

        case APP_CMD_LOST_FOCUS:
            LOGI("APP_CMD_LOST_FOCUS");
            state->hasFocus = false;
            state->animating = false;
            if (state->syncHandler) {
                state->syncHandler->pause();
            }
            break;

        case APP_CMD_SAVE_STATE:
            LOGI("APP_CMD_SAVE_STATE");
            if (state->sessionManager) {
                state->sessionManager->saveState();
            }
            break;

        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY");
            state->destroyRequested = true;
            break;

        case APP_CMD_START:
            LOGI("APP_CMD_START");
            break;

        case APP_CMD_STOP:
            LOGI("APP_CMD_STOP");
            break;

        case APP_CMD_RESUME:
            LOGI("APP_CMD_RESUME");
            break;

        case APP_CMD_PAUSE:
            LOGI("APP_CMD_PAUSE");
            break;

        case APP_CMD_LOW_MEMORY:
            LOGW("APP_CMD_LOW_MEMORY");
            if (state->sessionManager) {
                state->sessionManager->flushCaches();
            }
            break;

        case APP_CMD_CONFIG_CHANGED:
            LOGI("APP_CMD_CONFIG_CHANGED");
            break;

        default:
            break;
    }
}

// ── Input handler ──
static int32_t handleInput(struct android_app* app, AInputEvent* event) {
    if (gApp.inputHandler) {
        return gApp.inputHandler->processEvent(event);
    }
    return 0;
}

// ── EGL / OpenGL ES display initialization ──
static bool initDisplay() {
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        LOGE("eglInitialize failed");
        return false;
    }
    LOGI("EGL version %d.%d", major, minor);

    EGLint numConfigs;
    EGLConfig config;
    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || numConfigs < 1) {
        LOGE("eglChooseConfig failed");
        return false;
    }

    EGLSurface surface = eglCreateWindowSurface(display, config,
        gApp.androidApp->window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return false;
    }

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOGE("eglMakeCurrent failed");
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &gApp.width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &gApp.height);
    LOGI("Surface: %dx%d", gApp.width, gApp.height);

    glViewport(0, 0, gApp.width, gApp.height);
    glClearColor(0.059f, 0.071f, 0.106f, 1.0f);  // Dark theme background

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gApp.display = display;
    gApp.surface = surface;
    gApp.context = context;

    return true;
}

static void termDisplay() {
    if (gApp.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(gApp.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (gApp.context != EGL_NO_CONTEXT) {
            eglDestroyContext(gApp.display, gApp.context);
            gApp.context = EGL_NO_CONTEXT;
        }
        if (gApp.surface != EGL_NO_SURFACE) {
            eglDestroySurface(gApp.display, gApp.surface);
            gApp.surface = EGL_NO_SURFACE;
        }

        eglTerminate(gApp.display);
        gApp.display = EGL_NO_DISPLAY;
    }
}

// ── Core module initialization ──
static void initCoreModules() {
    LOGI("Initializing core modules...");

    // Config loader
    gApp.configLoader = std::make_unique<progressive::ConfigLoader>();
    gApp.configLoader->load(progressive::ConfigPaths::defaultPaths());

    // Session manager
    gApp.sessionManager = std::make_unique<progressive::SessionManager>();
    gApp.sessionManager->initialize(gApp.configLoader->getDatabasePath());

    // Sync handler
    gApp.syncHandler = std::make_unique<progressive::SyncHandler>(*gApp.sessionManager);
    gApp.syncHandler->setHomeserver(gApp.configLoader->getDefaultHomeserver());

    // Notification manager
    gApp.notificationManager = std::make_unique<progressive::NotificationManager>();

    // Crash reporter
    gApp.crashReporter = std::make_unique<progressive::CrashReporter>();
    gApp.crashReporter->install();

    LOGI("Core modules initialized.");
}

// ── Main loop ──
static void mainLoop() {
    auto now = std::chrono::steady_clock::now();
    double deltaTime = std::chrono::duration<double>(now - gApp.lastFrameTime).count();
    gApp.lastFrameTime = now;

    // Clamp delta to avoid spiral of death
    if (deltaTime > 0.25) deltaTime = 0.25;

    gApp.frameAccumulator += deltaTime;

    // Fixed timestep update
    while (gApp.frameAccumulator >= gApp.frameTimeStep) {
        gApp.frameAccumulator -= gApp.frameTimeStep;

        if (gApp.syncHandler) {
            gApp.syncHandler->update(gApp.frameTimeStep);
        }
        if (gApp.renderer) {
            gApp.renderer->update(gApp.frameTimeStep);
        }
        if (gApp.notificationManager) {
            gApp.notificationManager->update(gApp.frameTimeStep);
        }
    }

    // Render
    renderFrame();
}

static void renderFrame() {
    if (!gApp.hasSurface || !gApp.initialized) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (gApp.renderer) {
        gApp.renderer->render();
    }

    eglSwapBuffers(gApp.display, gApp.surface);
}

// ── JNI exports ──
extern "C" {

JNIEXPORT void JNICALL
Java_chat_progressive_app_ProgressiveApplication_nativeOnCreate(JNIEnv*, jobject) {
    LOGI("ProgressiveApplication nativeOnCreate");
}

JNIEXPORT void JNICALL
Java_chat_progressive_app_ProgressiveApplication_nativeOnTerminate(JNIEnv*, jobject) {
    LOGI("ProgressiveApplication nativeOnTerminate");
    if (gApp.sessionManager) {
        gApp.sessionManager->saveState();
    }
}

} // extern "C"
