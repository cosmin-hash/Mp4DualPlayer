#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>
#include <QTimer>
#include <QFileDialog>
#include <QStyle>
#include <memory>
#include "videowidget_qt.h"
#include "advanced_cpp_features.h"

class MediaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MediaWidget(QString displayName, QWidget *parent = nullptr);
    ~MediaWidget();

    // Move semantics for performance
    MediaWidget(MediaWidget &&other) noexcept = default;
    MediaWidget &operator=(MediaWidget &&other) noexcept = default;

    // Disable copy semantics (Qt objects should not be copied)
    MediaWidget(const MediaWidget &) = delete;
    MediaWidget &operator=(const MediaWidget &) = delete;

    // Public interface
    void loadVideo(QString fileName);
    void play();
    void pause();
    void stop();
    void seek(double position);

    double getCurrentPosition() const;
    double getDuration() const;
    bool isPlaying() const;
    bool isPaused() const;

    // Quality settings
    void setMaxFrameRate(int fps);
    void setVideoQuality(int quality);

    // Video widget type selection (simplified - only software now)
    enum class VideoWidgetType
    {
        Software // Force software widget
    };
    void setVideoWidgetType(VideoWidgetType type);
    VideoWidgetType currentVideoWidgetType() const;

    // Template metaprogramming: Generic quality setting with compile-time validation
    template <AdvancedCpp::QualityLevel Level>
    void setQualityTemplate()
    {
        static_assert(Level >= AdvancedCpp::QualityLevel::Low && Level <= AdvancedCpp::QualityLevel::High,
                      "Quality level must be Low, Medium, or High");
        m_qualityCombo->setCurrentIndex(static_cast<int>(Level));
        if (m_videoWidgetQt)
        {
            m_videoWidgetQt->setVideoQualityTemplate<Level>();
        }
    }

    // Template metaprogramming: Generic frame rate setting with compile-time validation
    template <int FPS>
    void setFrameRateTemplate()
    {
        static_assert(FPS >= 5 && FPS <= 120, "Frame rate must be between 5 and 120 FPS");
        if (m_videoWidgetQt)
        {
            m_videoWidgetQt->setMaxFrameRateTemplate<FPS>();
        }
    }

signals:
    void videoLoaded();
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void positionChanged(double position);
    void durationChanged(double duration);

private slots:
    void openFile();
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onSeekSliderChanged(int position);
    void onQualityChanged(int index);
    void updatePosition();
    void onVideoLoaded();
    void onPlaybackStarted();
    void onPlaybackPaused();
    void onPositionChanged(double position);
    void onDurationChanged(double duration);
    void onFpsChanged(double fps);
    void onResolutionChanged(const QSize &resolution);

private:
    void setupUI();
    void setupConnections();
    void updateControls();

    // Video widget management
    void createVideoWidget();
    void destroyVideoWidget();
    QWidget *getCurrentVideoWidget() const;
    VideoWidgetQt *getVideoWidgetQt() const;

    // Helper methods for video widget operations
    void loadVideoOnCurrentWidget(const QString &fileName);
    void playCurrentWidget();
    void pauseCurrentWidget();
    void stopCurrentWidget();
    void seekCurrentWidget(double position);
    double getCurrentPositionFromWidget() const;
    double getDurationFromWidget() const;
    void setMaxFrameRateOnWidget(int fps);
    void setVideoQualityOnWidget(int quality);
    void updateVideoWidgetInUI();

    std::unique_ptr<QVBoxLayout> m_mainLayout;
    std::unique_ptr<QLabel> m_displayLabel;

    // Video widget management - only software now
    std::unique_ptr<VideoWidgetQt> m_videoWidgetQt;
    VideoWidgetType m_videoWidgetType;

    // Performance controls
    std::unique_ptr<QHBoxLayout> m_performanceLayout;
    std::unique_ptr<QLabel> m_qualityLabel;
    std::unique_ptr<QComboBox> m_qualityCombo;

    // Rendering mode controls (removed - only software now)

    // Media controls
    std::unique_ptr<QHBoxLayout> m_controlsLayout;
    std::unique_ptr<QPushButton> m_openButton;
    std::unique_ptr<QPushButton> m_playButton;
    std::unique_ptr<QPushButton> m_pauseButton;
    std::unique_ptr<QPushButton> m_stopButton;

    // Progress controls
    std::unique_ptr<QSlider> m_positionSlider;
    std::unique_ptr<QLabel> m_timeLabel;
    std::unique_ptr<QProgressBar> m_progressBar;
    std::unique_ptr<QTimer> m_positionTimer;

    // Status displays
    std::unique_ptr<QLabel> m_fpsLabel;
    std::unique_ptr<QLabel> m_resolutionLabel;

    // State
    QString m_currentFile;
    QString m_displayName;
    bool m_isPlaying;
    bool m_isPaused;
    bool m_fileDialogOpen = false;
    bool m_connectionsSetup = false;
};
