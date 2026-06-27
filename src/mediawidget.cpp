#include "mediawidget.h"
#include <QDebug>
#include <QTime>
#include <QFileInfo>
#include <QTimer>
#include <QApplication>

MediaWidget::MediaWidget(QString displayName, QWidget *parent)
    : QWidget(parent), m_mainLayout(std::make_unique<QVBoxLayout>(this)), m_displayLabel(std::make_unique<QLabel>(this)), m_videoWidgetType(VideoWidgetType::Software), m_performanceLayout(std::make_unique<QHBoxLayout>()), m_qualityLabel(std::make_unique<QLabel>(this)), m_qualityCombo(std::make_unique<QComboBox>(this)), m_controlsLayout(std::make_unique<QHBoxLayout>()), m_openButton(std::make_unique<QPushButton>(this)), m_playButton(std::make_unique<QPushButton>(this)), m_pauseButton(std::make_unique<QPushButton>(this)), m_stopButton(std::make_unique<QPushButton>(this)), m_positionSlider(std::make_unique<QSlider>(Qt::Horizontal, this)), m_timeLabel(std::make_unique<QLabel>(this)), m_progressBar(std::make_unique<QProgressBar>(this)), m_positionTimer(std::make_unique<QTimer>(this)), m_fpsLabel(std::make_unique<QLabel>(this)), m_resolutionLabel(std::make_unique<QLabel>(this)), m_currentFile(""), m_displayName(std::move(displayName)), m_isPlaying(false), m_isPaused(false)
{
    // Set size policy to expand and fill available space
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    createVideoWidget();
    setupUI();
    setupConnections();
    updateControls();
}

MediaWidget::~MediaWidget()
{
    // Stop position timer
    if (m_positionTimer)
    {
        qDebug() << "Stopping position timer for:" << m_displayName;
        m_positionTimer->stop();
    }

    // Stop video widget (this will also stop the processing thread)
    if (m_videoWidgetQt)
    {
        qDebug() << "Stopping Qt video widget for:" << m_displayName;
        m_videoWidgetQt->stop();
        QApplication::processEvents(); // Process any pending events
        QThread::msleep(100);          // Give time for cleanup
    }

    // Force process any remaining events before destruction
    for (int i = 0; i < 3; ++i)
    {
        QApplication::processEvents();
        QThread::msleep(50);
    }

    // Smart pointers will automatically clean up in reverse order of declaration
    qDebug() << "MediaWidget destructor completed for:" << m_displayName;
}

void MediaWidget::setupUI()
{
    qDebug() << "Setting up UI for:" << m_displayName;

    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);

    // Configure display label
    m_displayLabel->setText(m_displayName + ":");
    m_displayLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    m_mainLayout->addWidget(m_displayLabel.get());

    // Configure video widget
    QWidget *currentVideoWidget = getCurrentVideoWidget();
    if (currentVideoWidget)
    {
        // Set size policy to expand and fill available space
        currentVideoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        currentVideoWidget->setMinimumSize(300, 200);
        currentVideoWidget->setVisible(true);
        m_mainLayout->addWidget(currentVideoWidget, 1); // Stretch factor of 1 to take available space
        qDebug() << "Added video widget to layout. Widget type: Qt";
        qDebug() << "Video widget size policy:" << currentVideoWidget->sizePolicy().horizontalPolicy() << "x" << currentVideoWidget->sizePolicy().verticalPolicy();
        qDebug() << "Video widget visible:" << currentVideoWidget->isVisible();
    }
    else
    {
        qWarning() << "No video widget available to add to layout!";
    }

    // Configure performance controls
    m_qualityLabel->setText("Quality:");
    m_qualityCombo->addItem("Low Quality (15 FPS)", 0);
    m_qualityCombo->addItem("Medium Quality (30 FPS)", 1);
    m_qualityCombo->addItem("High Quality (60 FPS)", 2);
    m_qualityCombo->setCurrentIndex(1); // Default to medium
    m_qualityCombo->setMaximumWidth(150);

    // Rendering mode controls removed - only software now

    m_performanceLayout->addWidget(m_qualityLabel.get());
    m_performanceLayout->addWidget(m_qualityCombo.get());
    m_performanceLayout->addStretch();

    m_mainLayout->addLayout(m_performanceLayout.get());

    // Configure media controls
    m_controlsLayout->setSpacing(5);

    // Configure media control buttons with icons
    m_openButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_openButton->setToolTip("Open MP4 File");

    m_openButton->setFixedSize(32, 32);

    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playButton->setToolTip("Play");
    m_playButton->setFixedSize(32, 32);

    m_pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    m_pauseButton->setToolTip("Pause");
    m_pauseButton->setFixedSize(32, 32);

    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopButton->setToolTip("Stop");
    m_stopButton->setFixedSize(32, 32);

    m_controlsLayout->addWidget(m_openButton.get());
    m_controlsLayout->addWidget(m_playButton.get());
    m_controlsLayout->addWidget(m_pauseButton.get());
    m_controlsLayout->addWidget(m_stopButton.get());
    m_controlsLayout->addStretch();

    m_mainLayout->addLayout(m_controlsLayout.get());

    // Configure progress controls
    m_positionSlider->setRange(0, 1000);
    m_positionSlider->setEnabled(false);

    m_timeLabel->setText("00:00 / 00:00");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setMinimumWidth(120);

    m_progressBar->setVisible(false);

    // Configure FPS and resolution labels
    m_fpsLabel->setText("FPS: 0.0");
    m_fpsLabel->setAlignment(Qt::AlignCenter);
    m_fpsLabel->setMinimumWidth(80);

    m_resolutionLabel->setText("Resolution: 0x0");
    m_resolutionLabel->setAlignment(Qt::AlignCenter);
    m_resolutionLabel->setMinimumWidth(120);

    // Create a horizontal layout for time, FPS, and resolution
    auto statusLayout = std::make_unique<QHBoxLayout>();
    statusLayout->addWidget(m_resolutionLabel.get());
    statusLayout->addWidget(m_timeLabel.get());
    statusLayout->addWidget(m_fpsLabel.get());

    m_mainLayout->addWidget(m_positionSlider.get());
    m_mainLayout->addLayout(statusLayout.release());
    m_mainLayout->addWidget(m_progressBar.get());

    // Configure position timer
    m_positionTimer->setInterval(100); // Update every 100ms

    qDebug() << "UI setup completed for:" << m_displayName;
}

void MediaWidget::setupConnections()
{
    qDebug() << "Setting up connections for:" << m_displayName;

    // Setup button connections only once
    if (!m_connectionsSetup)
    {
        qDebug() << "Setting up button connections for:" << m_displayName;

        // Button connections using perfect forwarding
        AdvancedCpp::safeConnect(m_openButton.get(), &QPushButton::clicked, this, &MediaWidget::openFile);
        // URL button removed (network playback disabled)
        AdvancedCpp::safeConnect(m_playButton.get(), &QPushButton::clicked, this, &MediaWidget::onPlayClicked);
        AdvancedCpp::safeConnect(m_pauseButton.get(), &QPushButton::clicked, this, &MediaWidget::onPauseClicked);
        AdvancedCpp::safeConnect(m_stopButton.get(), &QPushButton::clicked, this, &MediaWidget::onStopClicked);

        // Slider and quality connections using perfect forwarding
        AdvancedCpp::safeConnect(m_positionSlider.get(), &QSlider::valueChanged, this, &MediaWidget::onSeekSliderChanged);
        AdvancedCpp::safeConnect(m_qualityCombo.get(), QOverload<int>::of(&QComboBox::currentIndexChanged),
                                 this, &MediaWidget::onQualityChanged);

        // Timer connection using perfect forwarding
        AdvancedCpp::safeConnect(m_positionTimer.get(), &QTimer::timeout, this, &MediaWidget::updatePosition);

        m_connectionsSetup = true;
    }

    // Video widget connections using perfect forwarding
    if (m_videoWidgetQt)
    {
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::videoLoaded, this, &MediaWidget::onVideoLoaded);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::playbackStarted, this, &MediaWidget::onPlaybackStarted);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::playbackPaused, this, &MediaWidget::onPlaybackPaused);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::positionChanged, this, &MediaWidget::onPositionChanged);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::durationChanged, this, &MediaWidget::onDurationChanged);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::fpsChanged, this, &MediaWidget::onFpsChanged);
        AdvancedCpp::safeConnect(m_videoWidgetQt.get(), &VideoWidgetQt::resolutionChanged, this, &MediaWidget::onResolutionChanged);
    }

    m_connectionsSetup = true;
    qDebug() << "Connections setup completed for:" << m_displayName;
}

void MediaWidget::openFile()
{
    qDebug() << "openFile() called for:" << m_displayName << "- fileDialogOpen:" << m_fileDialogOpen;

    // Prevent multiple file dialogs from opening simultaneously for this instance
    if (m_fileDialogOpen)
    {
        qDebug() << "File dialog already open, ignoring request for:" << m_displayName;
        return;
    }

    m_fileDialogOpen = true;
    qDebug() << "Opening file for:" << m_displayName;

    m_openButton->setEnabled(false);

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open MP4 Video"), "",
                                                    tr("MP4 Files (*.mp4);;All Files (*)"));

    qDebug() << "File dialog closed for:" << m_displayName << "- Selected:" << fileName;

    m_openButton->setEnabled(true);
    m_fileDialogOpen = false;

    if (!fileName.isEmpty())
    {
        loadVideo(fileName);
    }
    else
    {
        qDebug() << "No file selected for:" << m_displayName;
    }
}

void MediaWidget::loadVideo(QString fileName)
{
    qDebug() << "Loading video for:" << m_displayName << "File:" << fileName;

    if (!QFileInfo::exists(fileName))
    {
        qDebug() << "File does not exist:" << fileName;
        return;
    }

    // Prevent multiple loads of the same file
    if (m_currentFile == fileName)
    {
        qDebug() << "File already loaded, skipping:" << fileName;
        return;
    }

    // Move the filename to avoid copying
    m_currentFile = std::move(fileName);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate progress

    loadVideoOnCurrentWidget(m_currentFile);
}

void MediaWidget::play()
{
    if (!m_currentFile.isEmpty())
    {
        playCurrentWidget();
    }
}

void MediaWidget::pause()
{
    pauseCurrentWidget();
}

void MediaWidget::stop()
{
    stopCurrentWidget();
    m_isPlaying = false;
    m_isPaused = false;
    m_positionTimer->stop();
    m_positionSlider->setValue(0);
    updateControls();
}

void MediaWidget::seek(double position)
{
    if (!m_currentFile.isEmpty())
    {
        seekCurrentWidget(position);
    }
}

double MediaWidget::getCurrentPosition() const
{
    return getCurrentPositionFromWidget();
}

double MediaWidget::getDuration() const
{
    return getDurationFromWidget();
}

bool MediaWidget::isPlaying() const
{
    return m_isPlaying;
}

bool MediaWidget::isPaused() const
{
    return m_isPaused;
}

void MediaWidget::setMaxFrameRate(int fps)
{
    setMaxFrameRateOnWidget(fps);
}

void MediaWidget::setVideoQuality(int quality)
{
    setVideoQualityOnWidget(quality);
}

void MediaWidget::onPlayClicked()
{
    // Prevent multiple play commands
    if (m_isPlaying && !m_isPaused)
    {
        qDebug() << "Already playing, ignoring play request for:" << m_displayName;
        return;
    }

    qDebug() << "Play button clicked for:" << m_displayName;
    play();
}

void MediaWidget::onPauseClicked()
{
    pause();
}

void MediaWidget::onStopClicked()
{
    stop();
}

void MediaWidget::onSeekSliderChanged(int position)
{
    if (!m_currentFile.isEmpty())
    {
        double pos = position / 1000.0;
        seek(pos);
    }
}

void MediaWidget::onQualityChanged(int index)
{
    int quality = m_qualityCombo->itemData(index).toInt();
    setVideoQuality(quality);

    // Use compile-time constants for frame rates
    int fps = 0;
    switch (static_cast<AdvancedCpp::QualityLevel>(quality))
    {
    case AdvancedCpp::QualityLevel::Low:
        fps = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::Low>::fps;
        break;
    case AdvancedCpp::QualityLevel::Medium:
        fps = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::Medium>::fps;
        break;
    case AdvancedCpp::QualityLevel::High:
        fps = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::High>::fps;
        break;
    }
    setMaxFrameRate(fps);
}

void MediaWidget::updatePosition()
{
    if (m_isPlaying && !m_isPaused)
    {
        double currentPos = getCurrentPosition();
        double duration = getDuration();

        if (duration > 0)
        {
            int sliderPos = static_cast<int>((currentPos / duration) * 1000);
            m_positionSlider->blockSignals(true);
            m_positionSlider->setValue(sliderPos);
            m_positionSlider->blockSignals(false);

            // Update time label
            QTime currentTime(0, 0, 0);
            QTime totalTime(0, 0, 0);
            currentTime = currentTime.addMSecs(static_cast<int>(currentPos * 1000));
            totalTime = totalTime.addMSecs(static_cast<int>(duration * 1000));

            m_timeLabel->setText(QString("%1 / %2")
                                     .arg(currentTime.toString("mm:ss"))
                                     .arg(totalTime.toString("mm:ss")));
        }
    }
}

void MediaWidget::onVideoLoaded()
{
    m_progressBar->setVisible(false);
    updateControls();
    emit videoLoaded();
}

void MediaWidget::onPlaybackStarted()
{
    m_isPlaying = true;
    m_isPaused = false;
    m_positionTimer->start();
    updateControls();
    emit playbackStarted();
}

void MediaWidget::onPlaybackPaused()
{
    m_isPaused = !m_isPaused;
    if (m_isPaused)
    {
        m_positionTimer->stop();
    }
    else
    {
        m_positionTimer->start();
    }
    updateControls();
    emit playbackPaused();
}

void MediaWidget::onPositionChanged(double position)
{
    if (m_isPlaying && !m_isPaused)
    {
        updatePosition();
    }
    emit positionChanged(position);
}

void MediaWidget::onDurationChanged(double duration)
{
    emit durationChanged(duration);
}

void MediaWidget::onFpsChanged(double fps)
{
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

void MediaWidget::onResolutionChanged(const QSize &resolution)
{
    m_resolutionLabel->setText(QString("Resolution: %1x%2").arg(resolution.width()).arg(resolution.height()));
}

void MediaWidget::updateControls()
{
    bool hasVideo = !m_currentFile.isEmpty();
    bool canPlay = hasVideo && (!m_isPlaying || m_isPaused);
    bool canPause = hasVideo && m_isPlaying && !m_isPaused;
    bool canStop = hasVideo && (m_isPlaying || m_isPaused);

    m_playButton->setEnabled(canPlay);
    m_pauseButton->setEnabled(canPause);
    m_stopButton->setEnabled(canStop);
    m_positionSlider->setEnabled(hasVideo);

    qDebug() << m_displayName << "controls updated - hasVideo:" << hasVideo
             << "isPlaying:" << m_isPlaying << "isPaused:" << m_isPaused;
}

// Video widget management methods
void MediaWidget::createVideoWidget()
{
    qDebug() << "Creating video widget for:" << m_displayName << "Type: Software";

    // Always create Qt software widget
    m_videoWidgetQt = std::make_unique<VideoWidgetQt>(this);
    qDebug() << "Created software video widget for:" << m_displayName;

    qDebug() << "Widget creation completed. Qt widget:" << (m_videoWidgetQt ? "YES" : "NO");
}

void MediaWidget::destroyVideoWidget()
{
    qDebug() << "Destroying video widget for:" << m_displayName;

    // Force immediate destruction without waiting for destructors
    if (m_videoWidgetQt)
    {
        qDebug() << "Resetting Qt video widget for:" << m_displayName;
        m_videoWidgetQt.reset();
        qDebug() << "Qt video widget reset completed for:" << m_displayName;
    }

    // Note: We don't reset m_connectionsSetup here because we want to keep button connections
    // Only video widget connections will be re-established in setupConnections()
    qDebug() << "Video widget destruction completed for:" << m_displayName;
}

QWidget *MediaWidget::getCurrentVideoWidget() const
{
    if (m_videoWidgetQt)
    {
        return m_videoWidgetQt.get();
    }
    return nullptr;
}

VideoWidgetQt *MediaWidget::getVideoWidgetQt() const
{
    return m_videoWidgetQt.get();
}

void MediaWidget::setVideoWidgetType(VideoWidgetType type)
{
    // Only software type is supported now
    if (m_videoWidgetType != type)
    {
        qDebug() << "Changing video widget type for:" << m_displayName
                 << "from" << static_cast<int>(m_videoWidgetType)
                 << "to Software";

        m_videoWidgetType = VideoWidgetType::Software;

        // Recreate the video widget
        qDebug() << "About to destroy video widget for:" << m_displayName;
        destroyVideoWidget();
        qDebug() << "Video widget destroyed for:" << m_displayName;

        qDebug() << "About to create video widget for:" << m_displayName;
        createVideoWidget();
        qDebug() << "Video widget created for:" << m_displayName;

        // Re-setup connections and update video widget in UI
        qDebug() << "About to setup connections for:" << m_displayName;
        setupConnections();
        qDebug() << "Connections setup for:" << m_displayName;

        qDebug() << "About to update video widget in UI for:" << m_displayName;
        updateVideoWidgetInUI();
        qDebug() << "Video widget updated in UI for:" << m_displayName;

        // Restart the position timer if we were playing
        if (m_isPlaying && !m_isPaused)
        {
            m_positionTimer->start(100); // Update every 100ms
        }
    }
}

MediaWidget::VideoWidgetType MediaWidget::currentVideoWidgetType() const
{
    return m_videoWidgetType;
}

void MediaWidget::updateVideoWidgetInUI()
{
    // Find and remove any existing video widget from the layout
    for (int i = m_mainLayout->count() - 1; i >= 0; --i)
    {
        QLayoutItem *item = m_mainLayout->itemAt(i);
        if (item && item->widget())
        {
            QWidget *widget = item->widget();
            // Check if this is a video widget (Qt only now)
            if (widget == m_videoWidgetQt.get())
            {
                m_mainLayout->removeWidget(widget);
                widget->setParent(nullptr);
                break;
            }
        }
    }

    // Add the new video widget to the layout
    QWidget *newVideoWidget = getCurrentVideoWidget();
    if (newVideoWidget)
    {
        // Set size policy to expand and fill available space
        newVideoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        newVideoWidget->setMinimumSize(300, 200);
        newVideoWidget->setVisible(true);
        // Insert the video widget at position 1 (after the display label) with stretch factor
        m_mainLayout->insertWidget(1, newVideoWidget, 1); // Stretch factor of 1
        qDebug() << "Updated video widget in UI. Widget type: Qt";
        qDebug() << "Video widget size policy:" << newVideoWidget->sizePolicy().horizontalPolicy() << "x" << newVideoWidget->sizePolicy().verticalPolicy();
        qDebug() << "Video widget visible:" << newVideoWidget->isVisible();
    }
    else
    {
        qWarning() << "No video widget available for UI update!";
    }
}

// Helper methods for video widget operations
void MediaWidget::loadVideoOnCurrentWidget(const QString &fileName)
{
    qDebug() << "loadVideoOnCurrentWidget called for:" << m_displayName << "File:" << fileName;
    qDebug() << "Current widget type - Qt:" << (m_videoWidgetQt ? "YES" : "NO");

    if (m_videoWidgetQt)
    {
        qDebug() << "Loading video on Qt widget";
        m_videoWidgetQt->loadVideo(fileName);
    }
    else
    {
        qWarning() << "No video widget available for loading video!";
    }
}

void MediaWidget::playCurrentWidget()
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->play();
    }
}

void MediaWidget::pauseCurrentWidget()
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->pause();
    }
}

void MediaWidget::stopCurrentWidget()
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->stop();
    }
}

void MediaWidget::seekCurrentWidget(double position)
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->seek(position);
    }
}

double MediaWidget::getCurrentPositionFromWidget() const
{
    if (m_videoWidgetQt)
    {
        double pos = m_videoWidgetQt->getCurrentPosition();

        return pos;
    }

    return 0.0;
}

double MediaWidget::getDurationFromWidget() const
{
    if (m_videoWidgetQt)
    {
        double duration = m_videoWidgetQt->getDuration();

        return duration;
    }

    return 0.0;
}

void MediaWidget::setMaxFrameRateOnWidget(int fps)
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->setMaxFrameRate(fps);
    }
}

void MediaWidget::setVideoQualityOnWidget(int quality)
{
    if (m_videoWidgetQt)
    {
        m_videoWidgetQt->setVideoQuality(quality);
    }
}
