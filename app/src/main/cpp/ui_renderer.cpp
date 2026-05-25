#include "ui_renderer.hpp"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOG_TAG "UIRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ── Shader sources ──

static const char* gBasicVS = R"glsl(
#version 300 es
in vec2 aPosition;
in vec4 aColor;
uniform mat4 uMVP;
out vec4 vColor;
void main() {
    gl_Position = uMVP * vec4(aPosition, 0.0, 1.0);
    vColor = aColor;
}
)glsl";

static const char* gBasicFS = R"glsl(
#version 300 es
precision mediump float;
in vec4 vColor;
out vec4 fragColor;
void main() {
    fragColor = vColor;
}
)glsl";

static const char* gTextVS = R"glsl(
#version 300 es
in vec2 aPosition;
in vec2 aTexCoord;
uniform mat4 uMVP;
out vec2 vTexCoord;
void main() {
    gl_Position = uMVP * vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)glsl";

static const char* gTextFS = R"glsl(
#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uColor;
out vec4 fragColor;
void main() {
    float alpha = texture(uTexture, vTexCoord).r;
    fragColor = vec4(uColor.rgb, uColor.a * alpha);
}
)glsl";

static const char* gTextureFS = R"glsl(
#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    fragColor = texture(uTexture, vTexCoord);
}
)glsl";

// ── Helper: orthographic projection ──
static void ortho(float* m, float left, float right,
                   float bottom, float top, float nearZ, float farZ) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (farZ - nearZ);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(farZ + nearZ) / (farZ - nearZ);
    m[15] = 1.0f;
}

// ── Quad geometry ──
static const float gQuadVerts[] = {
    // Position     // Color (RGBA)
    0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,
};

static const uint16_t gQuadIndices[] = {
    0, 1, 2,
    0, 2, 3,
};

// ── Implementation ──

UIRenderer::UIRenderer(int32_t screenWidth, int32_t screenHeight)
    : m_width(screenWidth), m_height(screenHeight) {
    LOGI("UIRenderer created: %dx%d", m_width, m_height);
    initShaders();
    initGeometry();
}

UIRenderer::~UIRenderer() {
    if (m_basicProgram) glDeleteProgram(m_basicProgram);
    if (m_textProgram) glDeleteProgram(m_textProgram);
    if (m_textureProgram) glDeleteProgram(m_textureProgram);
    if (m_rectVAO) glDeleteVertexArrays(1, &m_rectVAO);
    if (m_rectVBO) glDeleteBuffers(1, &m_rectVBO);
    if (m_rectEBO) glDeleteBuffers(1, &m_rectEBO);
}

void UIRenderer::initShaders() {
    m_basicProgram = createProgram(gBasicVS, gBasicFS);
    m_textProgram = createProgram(gTextVS, gTextFS);
    m_textureProgram = createProgram(gTextVS, gTextureFS);

    LOGI("Shaders initialized: basic=%u text=%u texture=%u",
         m_basicProgram, m_textProgram, m_textureProgram);
}

GLuint UIRenderer::loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            LOGE("Shader compile error: %s", infoLog);
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint UIRenderer::createProgram(const char* vsSource, const char* fsSource) {
    GLuint vs = loadShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = loadShader(GL_FRAGMENT_SHADER, fsSource);
    if (!vs || !fs) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
            LOGE("Program link error: %s", infoLog);
            delete[] infoLog;
        }
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void UIRenderer::initGeometry() {
    glGenVertexArrays(1, &m_rectVAO);
    glGenBuffers(1, &m_rectVBO);
    glGenBuffers(1, &m_rectEBO);

    glBindVertexArray(m_rectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gQuadVerts), gQuadVerts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gQuadIndices), gQuadIndices, GL_STATIC_DRAW);

    // Position (location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color (location 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void UIRenderer::update(double /*deltaTime*/) {
    // Update animations, layouts, etc.
}

void UIRenderer::resize(int32_t w, int32_t h) {
    m_width = w;
    m_height = h;
    glViewport(0, 0, w, h);
}

void UIRenderer::setBackgroundColor(float r, float g, float b, float a) {
    m_bgR = r; m_bgG = g; m_bgB = b; m_bgA = a;
}

void UIRenderer::render() {
    glClearColor(m_bgR, m_bgG, m_bgB, m_bgA);

    // Set up orthographic projection
    float mvp[16];
    ortho(mvp, 0.0f, static_cast<float>(m_width),
          static_cast<float>(m_height), 0.0f, -1.0f, 1.0f);

    glUseProgram(m_basicProgram);
    m_mvpLoc = glGetUniformLocation(m_basicProgram, "uMVP");
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, mvp);

    // Bind geometry
    glBindVertexArray(m_rectVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectEBO);

    // ── Draw UI elements ──
    // This is where the actual UI rendering happens.
    // In a full implementation, the UI tree would be traversed here
    // and each widget drawn using drawRect/drawRoundedRect.
    // For now, the stub draws a placeholder.

    // ... UI rendering calls go here ...
}

void UIRenderer::drawRect(float x, float y, float w, float h,
                           float r, float g, float b, float a) {
    float mvp[16];
    ortho(mvp, x, x + w, y + h, y, -1.0f, 1.0f);
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, mvp);

    // Set color via shader or per-vertex
    // In a full implementation, this would use instancing or uniform
    glUniform4f(glGetUniformLocation(m_basicProgram, "uColor"), r, g, b, a);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void UIRenderer::drawRoundedRect(float x, float y, float w, float h,
                                  float radius, float r, float g, float b, float a) {
    // Rounded rects require more complex geometry.
    // For now, draw as plain rect.
    drawRect(x, y, w, h, r, g, b, a);
    (void)radius;
}
