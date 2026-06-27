#pragma once

#include <QObject>
#include <QWidget>
#include <QString>
#include <QSize>
#include <QVideoFrame>
#include <QImage>
#include <QPixmap>
#include <type_traits>
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
// TEMPLATE METAPROGRAMMING UTILITIES
// ============================================================================

/**
 * @brief Type trait to check if a type is a Qt widget
 */
template<typename T>
struct is_qt_widget : std::is_base_of<QWidget, std::remove_pointer_t<T>> {};

template<typename T>
constexpr bool is_qt_widget_v = is_qt_widget<T>::value;

/**
 * @brief Type trait to check if a type is a Qt object
 */
template<typename T>
struct is_qt_object : std::is_base_of<QObject, std::remove_pointer_t<T>> {};

template<typename T>
constexpr bool is_qt_object_v = is_qt_object<T>::value;

/**
 * @brief Type trait to check if a type is a video frame type
 */
template<typename T>
struct is_video_frame : std::is_same<T, QVideoFrame> {};

template<typename T>
constexpr bool is_video_frame_v = is_video_frame<T>::value;

/**
 * @brief Type trait to check if a type is an image type
 */
template<typename T>
struct is_image_type : std::disjunction<
    std::is_same<T, QImage>,
    std::is_same<T, QPixmap>
> {};

template<typename T>
constexpr bool is_image_type_v = is_image_type<T>::value;

/**
 * @brief SFINAE helper for Qt widget operations
 */
template<typename T>
using enable_if_qt_widget = std::enable_if_t<is_qt_widget_v<T>, bool>;

template<typename T>
using enable_if_qt_object = std::enable_if_t<is_qt_object_v<T>, bool>;

template<typename T>
using enable_if_video_frame = std::enable_if_t<is_video_frame_v<T>, bool>;

template<typename T>
using enable_if_image_type = std::enable_if_t<is_image_type_v<T>, bool>;

// ============================================================================
// ADVANCED TEMPLATE FUNCTIONS
// ============================================================================

/**
 * @brief Generic widget configuration with perfect forwarding and type safety
 * @tparam Widget Widget type
 * @tparam ConfigFunc Configuration function type
 * @tparam Args... Configuration arguments
 */
template<typename Widget, typename ConfigFunc, typename... Args>
enable_if_qt_widget<Widget> configureWidget(Widget&& widget, ConfigFunc&& configFunc, Args&&... args) {
    static_assert(is_qt_widget_v<std::decay_t<Widget>>, "Widget must be a Qt widget type");
    configFunc(std::forward<Widget>(widget), std::forward<Args>(args)...);
}

/**
 * @brief Generic signal connection with type safety
 * @tparam Sender Sender type
 * @tparam Signal Signal type
 * @tparam Receiver Receiver type
 * @tparam Slot Slot type
 */
template<typename Sender, typename Signal, typename Receiver, typename Slot>
enable_if_qt_object<std::decay_t<Sender>> safeConnect(Sender&& sender, Signal&& signal, Receiver&& receiver, Slot&& slot) {
    static_assert(is_qt_object_v<std::decay_t<Sender>>, "Sender must be a Qt object");
    static_assert(is_qt_object_v<std::decay_t<Receiver>>, "Receiver must be a Qt object");

    return QObject::connect(
        std::forward<Sender>(sender),
        std::forward<Signal>(signal),
        std::forward<Receiver>(receiver),
        std::forward<Slot>(slot)
    );
}

/**
 * @brief Generic frame processing with perfect forwarding
 * @tparam Frame Frame type
 * @tparam Processor Processing function type
 * @tparam Args... Processing arguments
 */
template<typename Frame, typename Processor, typename... Args>
enable_if_video_frame<std::decay_t<Frame>> processFrame(Frame&& frame, Processor&& processor, Args&&... args) {
    static_assert(is_video_frame_v<std::decay_t<Frame>>, "Frame must be a QVideoFrame");
    processor(std::forward<Frame>(frame), std::forward<Args>(args)...);
    return true;
}

/**
 * @brief Generic image processing with perfect forwarding
 * @tparam Image Image type
 * @tparam Processor Processing function type
 * @tparam Args... Processing arguments
 */
template<typename Image, typename Processor, typename... Args>
enable_if_image_type<std::decay_t<Image>> processImage(Image&& image, Processor&& processor, Args&&... args) {
    static_assert(is_image_type_v<std::decay_t<Image>>, "Image must be QImage or QPixmap");
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
