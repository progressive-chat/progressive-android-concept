#include "ui_input_handler.hpp"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "UIInput"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

UIInputHandler::UIInputHandler() {
    LOGI("UIInputHandler created");
}

UIInputHandler::~UIInputHandler() = default;

int32_t UIInputHandler::processEvent(AInputEvent* event) {
    if (!event) return 0;

    int32_t type = AInputEvent_getType(event);

    switch (type) {
        case AINPUT_EVENT_TYPE_MOTION:
            return handleMotionEvent(event);
        case AINPUT_EVENT_TYPE_KEY:
            return handleKeyEvent(event);
        default:
            return 0;
    }
}

int32_t UIInputHandler::handleMotionEvent(AInputEvent* event) {
    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                           >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    int32_t pointerCount = AMotionEvent_getPointerCount(event);

    switch (actionMasked) {
        case AMOTION_EVENT_ACTION_DOWN: {
            m_touching = true;
            m_longPressFired = false;
            m_scrolling = false;
            m_touchStartX = AMotionEvent_getX(event, 0);
            m_touchStartY = AMotionEvent_getY(event, 0);
            m_touchX = m_touchStartX;
            m_touchY = m_touchStartY;
            m_touchStartTime = std::chrono::steady_clock::now();
            return 1;
        }

        case AMOTION_EVENT_ACTION_MOVE: {
            if (!m_touching) return 0;
            m_touchX = AMotionEvent_getX(event, 0);
            m_touchY = AMotionEvent_getY(event, 0);

            float dx = m_touchX - m_touchStartX;
            float dy = m_touchY - m_touchStartY;

            // Multi-touch pinch detection
            if (pointerCount >= 2) {
                float dist = pointerDistance(event, 0, 1);
                if (!m_multiTouch) {
                    m_multiTouch = true;
                    m_initialPinchDist = dist;
                    pointerCenter(event, 0, 1, m_pinchCenterX, m_pinchCenterY);
                } else {
                    float scale = dist / m_initialPinchDist;
                    if (std::abs(dist - m_initialPinchDist) > PINCH_MIN_DELTA && m_onPinch) {
                        float cx, cy;
                        pointerCenter(event, 0, 1, cx, cy);
                        m_onPinch(scale, cx, cy);
                    }
                }
                return 1;
            }

            // Scroll detection
            if (std::abs(dx) > TAP_MAX_MOVE || std::abs(dy) > TAP_MAX_MOVE) {
                m_scrolling = true;
                if (m_onScroll) {
                    m_onScroll(dx - m_lastScrollX, dy - m_lastScrollY);
                }
                m_lastScrollX = dx;
                m_lastScrollY = dy;
            }

            // Long press check
            if (!m_longPressFired && !m_scrolling) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - m_touchStartTime).count();
                if (elapsed >= LONG_PRESS_MS && m_onLongPress) {
                    m_longPressFired = true;
                    m_onLongPress(m_touchStartX, m_touchStartY);
                }
            }

            return 1;
        }

        case AMOTION_EVENT_ACTION_UP: {
            if (!m_touching) return 0;

            if (!m_scrolling && !m_longPressFired && m_onTap) {
                m_onTap(m_touchStartX, m_touchStartY);
            }

            m_touching = false;
            m_multiTouch = false;
            m_scrolling = false;
            m_lastScrollX = 0;
            m_lastScrollY = 0;
            return 1;
        }

        case AMOTION_EVENT_ACTION_CANCEL: {
            m_touching = false;
            m_multiTouch = false;
            m_scrolling = false;
            return 1;
        }

        case AMOTION_EVENT_ACTION_POINTER_DOWN: {
            if (pointerCount >= 2 && !m_multiTouch) {
                m_multiTouch = true;
                m_initialPinchDist = pointerDistance(event, 0, 1);
                pointerCenter(event, 0, 1, m_pinchCenterX, m_pinchCenterY);
            }
            return 1;
        }

        case AMOTION_EVENT_ACTION_POINTER_UP: {
            if (pointerCount <= 1) {
                m_multiTouch = false;
            }
            return 1;
        }

        default:
            return 0;
    }
}

int32_t UIInputHandler::handleKeyEvent(AInputEvent* event) {
    int32_t action = AKeyEvent_getAction(event);
    int32_t keyCode = AKeyEvent_getKeyCode(event);
    int32_t metaState = AKeyEvent_getMetaState(event);

    // Handle back button
    if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_UP) {
        if (m_onBack && m_onBack()) {
            return 1; // Consumed
        }
    }

    // General key handling
    if (m_onKey) {
        KeyEvent kev;
        kev.keyCode = keyCode;
        kev.action = (action == AKEY_EVENT_ACTION_DOWN) ? KeyEvent::DOWN :
                     (action == AKEY_EVENT_ACTION_UP) ? KeyEvent::UP : KeyEvent::MULTIPLE;
        kev.metaState = metaState;
        kev.timestamp = AKeyEvent_getEventTime(event);
        if (m_onKey(kev)) {
            return 1;
        }
    }

    return 0;
}

float UIInputHandler::pointerDistance(AInputEvent* event, int32_t p0, int32_t p1) {
    float x0 = AMotionEvent_getX(event, p0);
    float y0 = AMotionEvent_getY(event, p0);
    float x1 = AMotionEvent_getX(event, p1);
    float y1 = AMotionEvent_getY(event, p1);
    float dx = x1 - x0;
    float dy = y1 - y0;
    return std::sqrt(dx * dx + dy * dy);
}

void UIInputHandler::pointerCenter(AInputEvent* event, int32_t p0, int32_t p1,
                                    float& cx, float& cy) {
    cx = (AMotionEvent_getX(event, p0) + AMotionEvent_getX(event, p1)) * 0.5f;
    cy = (AMotionEvent_getY(event, p0) + AMotionEvent_getY(event, p1)) * 0.5f;
}
