#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QVBoxLayout>
#include <QUrl>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include "advanced_cpp_features.h"

class VideoProcessingThread : public QThread
{
    Q_OBJECT

public:
    VideoProcessingThread(QObject *parent = nullptr);
    ~VideoProcessingThread();

    void processFrame(QVideoFrame &&frame, const QSize &targetSize, int quality);

    // Generic frame processing with perfect forwarding, constrained by the
    // VideoFrame concept (rejects anything that is not a QVideoFrame).
    template <AdvancedCpp::VideoFrame Frame, typename Size, typename Quality>
    void processFrameGeneric(Frame &&frame, Size &&targetSize, Quality &&quality)
    {
        processFrame(std::forward<Frame>(frame), std::forward<Size>(targetSize), std::forward<Quality>(quality));
    }
    void stop();

signals:
    void frameProcessed(const QPixmap &pixmap);

protected:
    void run() override;

private:
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_stopRequested;

    struct FrameData
    {
        QVideoFrame frame;
        QSize targetSize;
        int quality;
    };

    FrameData m_pendingFrame;
    bool m_hasPendingFrame;
};

class VideoWidgetQt : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidgetQt(QWidget *parent = nullptr);
    ~VideoWidgetQt();

    // Move semantics for performance
    VideoWidgetQt(VideoWidgetQt &&other) noexcept = default;
    VideoWidgetQt &operator=(VideoWidgetQt &&other) noexcept = default;

    // Disable copy semantics (Qt objects should not be copied)
    VideoWidgetQt(const VideoWidgetQt &) = delete;
    VideoWidgetQt &operator=(const VideoWidgetQt &) = delete;

    void loadVideo(QString fileName);
    void play();
    void pause();
    void stop();
    void seek(double position);

    double getCurrentPosition() const;
    double getDuration() const;
    bool isPlaying() const;

    // Performance settings
    void setMaxFrameRate(int fps);
    void setVideoQuality(int quality);

    // Template metaprogramming: Generic frame rate setting with compile-time validation
    template <int FPS>
    void setMaxFrameRateTemplate()
    {
        static_assert(FPS >= 5 && FPS <= 120, "Frame rate must be between 5 and 120 FPS");
        setMaxFrameRate(FPS);
    }

    // Template metaprogramming: Generic quality setting with compile-time validation
    template <AdvancedCpp::QualityLevel Level>
    void setVideoQualityTemplate()
    {
        setVideoQuality(static_cast<int>(Level));
        setMaxFrameRate(AdvancedCpp::QualitySettings<Level>::fps);
    } // 0=low, 1=medium, 2=high

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void videoLoaded();
    void playbackFinished();
    void positionChanged(double position);
    void durationChanged(double duration);
    void playbackStarted();
    void playbackPaused();
    void fpsChanged(double fps);
    void resolutionChanged(const QSize &resolution);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onErrorOccurred(QMediaPlayer::Error error, QString errorString);

private:
    std::unique_ptr<QMediaPlayer> m_mediaPlayer;
    std::unique_ptr<QVideoSink> m_videoSink;
    std::unique_ptr<QLabel> m_videoLabel;
    std::unique_ptr<QVBoxLayout> m_layout;
    std::unique_ptr<VideoProcessingThread> m_processingThread;

    double m_duration;
    bool m_isLoaded;

    // Performance optimization
    qint64 m_lastFrameTime;
    int m_maxFrameRate;
    int m_videoQuality;
    QSize m_lastVideoSize;
    static const int DEFAULT_FRAME_SKIP_MS = 33; // ~30 FPS max for high-res videos

    // FPS tracking
    QElapsedTimer m_fpsTimer;
    int m_frameCount;
    double m_currentFps;

    // Resolution tracking
    QSize m_videoResolution;
};
