#pragma once

#include <android/input.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <chrono>

// ── Touch/input handler for the C++ UI ──

struct TouchEvent {
    enum Type { DOWN, MOVE, UP, CANCEL, POINTER_DOWN, POINTER_UP };

    Type type;
    int32_t pointerId;
    float x, y;
    float pressure;
    float size;
    int64_t timestamp;
};

struct KeyEvent {
    enum Action { DOWN, UP, MULTIPLE };

    int32_t keyCode;
    Action action;
    int32_t metaState;
    int64_t timestamp;
};

class UIInputHandler {
public:
    UIInputHandler();
    ~UIInputHandler();

    // Process raw AInputEvent
    int32_t processEvent(AInputEvent* event);

    // Register tap handler (position in dp)
    using TapCallback = std::function<void(float x, float y)>;
    void setOnTap(TapCallback cb) { m_onTap = std::move(cb); }

    // Register long press handler
    using LongPressCallback = std::function<void(float x, float y)>;
    void setOnLongPress(LongPressCallback cb) { m_onLongPress = std::move(cb); }

    // Register scroll/fling handler
    using ScrollCallback = std::function<void(float dx, float dy)>;
    void setOnScroll(ScrollCallback cb) { m_onScroll = std::move(cb); }

    // Register pinch handler
    using PinchCallback = std::function<void(float scale, float cx, float cy)>;
    void setOnPinch(PinchCallback cb) { m_onPinch = std::move(cb); }

    // Register key handler (for hardware keyboards)
    using KeyCallback = std::function<bool(const KeyEvent&)>;
    void setOnKey(KeyCallback cb) { m_onKey = std::move(cb); }

    // Register back button handler
    using BackCallback = std::function<bool()>;
    void setOnBack(BackCallback cb) { m_onBack = std::move(cb); }

    // Current touch state
    bool isTouching() const { return m_touching; }
    float getTouchX() const { return m_touchX; }
    float getTouchY() const { return m_touchY; }

private:
    int32_t handleMotionEvent(AInputEvent* event);
    int32_t handleKeyEvent(AInputEvent* event);

    // Gesture detection state
    bool m_touching = false;
    float m_touchX = 0, m_touchY = 0;
    float m_touchStartX = 0, m_touchStartY = 0;
    std::chrono::steady_clock::time_point m_touchStartTime;
    bool m_longPressFired = false;

    // Multi-touch for pinch
    bool m_multiTouch = false;
    float m_initialPinchDist = 0;
    float m_pinchCenterX = 0, m_pinchCenterY = 0;

    // Scroll tracking
    float m_lastScrollX = 0, m_lastScrollY = 0;
    bool m_scrolling = false;

    // Thresholds (dp)
    static constexpr float TAP_MAX_MOVE = 10.0f;
    static constexpr int64_t LONG_PRESS_MS = 500;
    static constexpr float PINCH_MIN_DELTA = 5.0f;

    // Callbacks
    TapCallback m_onTap;
    LongPressCallback m_onLongPress;
    ScrollCallback m_onScroll;
    PinchCallback m_onPinch;
    KeyCallback m_onKey;
    BackCallback m_onBack;

    // Compute distance between two pointers
    static float pointerDistance(AInputEvent* event, int32_t p0, int32_t p1);
    static void pointerCenter(AInputEvent* event, int32_t p0, int32_t p1,
                              float& cx, float& cy);
};
