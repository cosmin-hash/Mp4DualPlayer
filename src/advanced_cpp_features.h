#pragma once

#include <QObject>
#include <QWidget>
#include <QString>
#include <QSize>
#include <QVideoFrame>
#include <QImage>
#include <QPixmap>
#include <type_traits>
#include <concepts>
#include <utility>

namespace AdvancedCpp {

// ============================================================================
// PERFECT FORWARDING UTILITIES
// ============================================================================

/**
 * @brief Perfect forwarding wrapper for Qt signal connections
 * @tparam Sender Type of the sender object
 * @tparam Signal Signal type (member function pointer)
 * @tparam Receiver Type of the receiver object
 * @tparam Slot Slot type (member function pointer)
 * @tparam Args... Perfect forwarded arguments
 */
template<typename Sender, typename Signal, typename Receiver, typename Slot, typename... Args>
void connectWithForwarding(Sender&& sender, Signal&& signal, Receiver&& receiver, Slot&& slot, Args&&... args) {
    QObject::connect(
        std::forward<Sender>(sender),
        std::forward<Signal>(signal),
        std::forward<Receiver>(receiver),
        std::forward<Slot>(slot),
        std::forward<Args>(args)...
    );
}

/**
 * @brief Perfect forwarding wrapper for widget property setting
 * @tparam Widget Widget type
 * @tparam Property Property type
 * @tparam Value Value type
 */
template<typename Widget, typename Property, typename Value>
void setPropertyWithForwarding(Widget&& widget, Property&& property, Value&& value) {
    widget->setProperty(std::forward<Property>(property), std::forward<Value>(value));
}

/**
 * @brief Perfect forwarding wrapper for generic function calls
 * @tparam Func Function type
 * @tparam Args... Argument types
 */
template<typename Func, typename... Args>
auto callWithForwarding(Func&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
    return func(std::forward<Args>(args)...);
}

// ============================================================================
// C++20 CONCEPTS
// ============================================================================
//
// These concepts replace the previous SFINAE/enable_if type-traits. They are
// written to accept either a value type or a pointer to it (Qt connect/widget
// helpers are normally called with QObject* pointers), and they ignore cv-ref
// qualifiers so they work with forwarding references.

/**
 * @brief Satisfied by Qt widget types (or pointers to them).
 */
template <typename T>
concept QtWidget = std::is_base_of_v<QWidget, std::remove_pointer_t<std::decay_t<T>>>;

/**
 * @brief Satisfied by Qt object types (or pointers to them).
 */
template <typename T>
concept QtObject = std::is_base_of_v<QObject, std::remove_pointer_t<std::decay_t<T>>>;

/**
 * @brief Satisfied only by QVideoFrame (ignoring cv-ref qualifiers).
 */
template <typename T>
concept VideoFrame = std::is_same_v<std::decay_t<T>, QVideoFrame>;

/**
 * @brief Satisfied by Qt image container types (QImage or QPixmap).
 */
template <typename T>
concept ImageType = std::is_same_v<std::decay_t<T>, QImage> ||
                    std::is_same_v<std::decay_t<T>, QPixmap>;

// ============================================================================
// ADVANCED TEMPLATE FUNCTIONS
// ============================================================================

/**
 * @brief Generic widget configuration with perfect forwarding and type safety
 * @tparam Widget Widget type (constrained by the QtWidget concept)
 * @tparam ConfigFunc Configuration function type
 * @tparam Args... Configuration arguments
 */
template<QtWidget Widget, typename ConfigFunc, typename... Args>
void configureWidget(Widget&& widget, ConfigFunc&& configFunc, Args&&... args) {
    configFunc(std::forward<Widget>(widget), std::forward<Args>(args)...);
}

/**
 * @brief Generic signal connection with type safety
 *
 * Constrained with the QtObject concept, so a non-QObject sender or receiver
 * is rejected at the call site with a clear diagnostic.
 * @tparam Sender Sender type (constrained by the QtObject concept)
 * @tparam Signal Signal type
 * @tparam Receiver Receiver type (constrained by the QtObject concept)
 * @tparam Slot Slot type
 */
template<QtObject Sender, typename Signal, QtObject Receiver, typename Slot>
bool safeConnect(Sender&& sender, Signal&& signal, Receiver&& receiver, Slot&& slot) {
    return QObject::connect(
        std::forward<Sender>(sender),
        std::forward<Signal>(signal),
        std::forward<Receiver>(receiver),
        std::forward<Slot>(slot)
    );
}

/**
 * @brief Generic frame processing with perfect forwarding
 * @tparam Frame Frame type (constrained by the VideoFrame concept)
 * @tparam Processor Processing function type
 * @tparam Args... Processing arguments
 */
template<VideoFrame Frame, typename Processor, typename... Args>
bool processFrame(Frame&& frame, Processor&& processor, Args&&... args) {
    processor(std::forward<Frame>(frame), std::forward<Args>(args)...);
    return true;
}

/**
 * @brief Generic image processing with perfect forwarding
 * @tparam Image Image type (constrained by the ImageType concept)
 * @tparam Processor Processing function type
 * @tparam Args... Processing arguments
 */
template<ImageType Image, typename Processor, typename... Args>
void processImage(Image&& image, Processor&& processor, Args&&... args) {
    processor(std::forward<Image>(image), std::forward<Args>(args)...);
}


// ============================================================================
// COMPILE-TIME CONSTANTS AND UTILITIES
// ============================================================================

/**
 * @brief Compile-time frame rate limits
 */
template<int FPS>
struct FrameRateLimits {
    static_assert(FPS >= 5 && FPS <= 120, "Frame rate must be between 5 and 120 FPS");
    static constexpr int minFPS = 5;
    static constexpr int maxFPS = 120;
    static constexpr int currentFPS = FPS;
    static constexpr int frameSkipMs = 1000 / FPS;
};

/**
 * @brief Compile-time quality settings
 */
enum class QualityLevel : int {
    Low = 0,
    Medium = 1,
    High = 2
};

template<QualityLevel Level>
struct QualitySettings {
    static constexpr QualityLevel level = Level;
    static constexpr int fps = (Level == QualityLevel::Low) ? 15 :
                              (Level == QualityLevel::Medium) ? 30 : 60;
    static constexpr Qt::TransformationMode transformMode =
        (Level == QualityLevel::Low) ? Qt::FastTransformation : Qt::SmoothTransformation;
};

// ============================================================================
// ADVANCED LAMBDA UTILITIES
// ============================================================================

/**
 * @brief Perfect forwarding lambda wrapper
 * @tparam Func Lambda function type
 * @tparam Args... Argument types
 */
template<typename Func, typename... Args>
auto forwardLambda(Func&& func, Args&&... args) {
    return [func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() mutable {
        return func(std::forward<Args>(args)...);
    };
}

/**
 * @brief Type-safe lambda wrapper for Qt signals
 * @tparam Func Lambda function type
 * @tparam Args... Captured argument types
 */
template<typename Func, typename... Args>
auto qtSignalLambda(Func&& func, Args&&... args) {
    static_assert(std::is_invocable_v<Func, Args...>, "Function must be invocable with provided arguments");
    return [func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() mutable {
        return func(std::forward<Args>(args)...);
    };
}

} // namespace AdvancedCpp
