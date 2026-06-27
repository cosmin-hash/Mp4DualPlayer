#include "videowidget_qt.h"
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include <QApplication>

// VideoProcessingThread implementation
VideoProcessingThread::VideoProcessingThread(QObject *parent)
    : QThread(parent), m_stopRequested(false), m_hasPendingFrame(false)
{
}

VideoProcessingThread::~VideoProcessingThread()
{
    // Cooperative shutdown: request the worker to stop (sets m_stopRequested and
    // wakes the wait condition), then wait() to join the thread cleanly. No
    // terminate() - the worker is never killed mid-operation.
    qDebug() << "VideoProcessingThread destructor: requesting cooperative stop";
    stop();
    wait();
    qDebug() << "VideoProcessingThread destructor: worker joined";
}

void VideoProcessingThread::processFrame(QVideoFrame &&frame, const QSize &targetSize, int quality)
{
    QMutexLocker locker(&m_mutex);

    // Skip if we already have a pending frame (drop frames for performance)
    if (m_hasPendingFrame)
    {
        return;
    }

    // Move the frame to avoid copying large video data
    m_pendingFrame.frame = std::move(frame);
    m_pendingFrame.targetSize = targetSize;
    m_pendingFrame.quality = quality;
    m_hasPendingFrame = true;

    m_condition.wakeOne();
}

void VideoProcessingThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopRequested = true;
    m_condition.wakeAll(); // Wake all waiting threads, not just one
}

void VideoProcessingThread::run()
{
    qDebug() << "VideoProcessingThread started";

    while (!m_stopRequested)
    {
        QMutexLocker locker(&m_mutex);

        // Block until a new frame is enqueued (processFrame wakes us) or a stop
        // is requested (stop() wakes us). No timeout, so the idle worker uses no
        // CPU - there is no busy-wait.
        while (!m_hasPendingFrame && !m_stopRequested)
        {
            m_condition.wait(&m_mutex);
        }

        if (m_stopRequested)
        {
            qDebug() << "VideoProcessingThread stopping due to stop request";
            break;
        }

        // Get the frame data using move semantics
        QVideoFrame frame = std::move(m_pendingFrame.frame);
        QSize targetSize = m_pendingFrame.targetSize;
        int quality = m_pendingFrame.quality;
        m_hasPendingFrame = false;

        locker.unlock();

        // Check for stop request before processing
        if (m_stopRequested)
        {
            qDebug() << "VideoProcessingThread stopping during frame processing";
            break;
        }

        // Process the frame with type safety
        if (frame.isValid())
        {
            QImage image = frame.toImage();
            if (!image.isNull() && !targetSize.isEmpty())
            {
                // Use compile-time quality settings for transformation mode
                Qt::TransformationMode transformMode = Qt::FastTransformation;
                switch (static_cast<AdvancedCpp::QualityLevel>(quality))
                {
                case AdvancedCpp::QualityLevel::Low:
                    transformMode = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::Low>::transformMode;
                    break;
                case AdvancedCpp::QualityLevel::Medium:
                    transformMode = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::Medium>::transformMode;
                    break;
                case AdvancedCpp::QualityLevel::High:
                    transformMode = AdvancedCpp::QualitySettings<AdvancedCpp::QualityLevel::High>::transformMode;
                    break;
                }

                if (image.size() != targetSize)
                {
                    image = image.scaled(targetSize, Qt::KeepAspectRatio, transformMode);
                }

                // Convert to pixmap using type-safe image processing
                QPixmap pixmap = QPixmap::fromImage(std::move(image));

                // Type safety check using the ImageType concept
                static_assert(AdvancedCpp::ImageType<QPixmap>, "QPixmap must be a valid image type");

                emit frameProcessed(std::move(pixmap));
            }
        }

        // Check for stop request after processing
        if (m_stopRequested)
        {
            qDebug() << "VideoProcessingThread stopping after frame processing";
            break;
        }
    }

    qDebug() << "VideoProcessingThread finished";
}

VideoWidgetQt::VideoWidgetQt(QWidget *parent)
    : QWidget(parent), m_mediaPlayer(std::make_unique<QMediaPlayer>(this)), m_videoSink(std::make_unique<QVideoSink>(this)), m_videoLabel(std::make_unique<QLabel>(this)), m_layout(std::make_unique<QVBoxLayout>(this)), m_processingThread(std::make_unique<VideoProcessingThread>(this)), m_duration(0.0), m_isLoaded(false), m_lastFrameTime(0), m_maxFrameRate(30), m_videoQuality(1) // Medium quality by default
      ,
      m_lastVideoSize(0, 0), m_frameCount(0), m_currentFps(0.0), m_videoResolution(0, 0)
{
    // Configure layout
    m_layout->setContentsMargins(0, 0, 0, 0);

    // Configure video label for display
    m_videoLabel->setMinimumSize(400, 300);
    m_videoLabel->setStyleSheet("border: 2px solid black; background-color: black; color: white;");
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setText("No video loaded");
    m_videoLabel->setScaledContents(false); // Don't scale contents automatically
    m_videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(m_videoLabel.get());

    // Configure media player
    m_mediaPlayer->setProperty("debug", false);

    // Configure video sink
    m_mediaPlayer->setVideoOutput(m_videoSink.get());

    // Start processing thread
    m_processingThread->start();

    // Connect signals using perfect forwarding
    AdvancedCpp::safeConnect(m_mediaPlayer.get(), &QMediaPlayer::mediaStatusChanged,
                             this, &VideoWidgetQt::onMediaStatusChanged);
    AdvancedCpp::safeConnect(m_mediaPlayer.get(), &QMediaPlayer::playbackStateChanged,
                             this, &VideoWidgetQt::onPlaybackStateChanged);
    AdvancedCpp::safeConnect(m_mediaPlayer.get(), &QMediaPlayer::positionChanged,
                             this, &VideoWidgetQt::onPositionChanged);
    AdvancedCpp::safeConnect(m_mediaPlayer.get(), &QMediaPlayer::durationChanged,
                             this, &VideoWidgetQt::onDurationChanged);
    AdvancedCpp::safeConnect(m_mediaPlayer.get(), &QMediaPlayer::errorOccurred,
                             this, &VideoWidgetQt::onErrorOccurred);

    // Connect video frame handler with threading using perfect forwarding
    AdvancedCpp::safeConnect(m_videoSink.get(), &QVideoSink::videoFrameChanged,
                             this, [this](QVideoFrame frame)
                             {
                if (frame.isValid()) {
                    // Frame rate limiting using compile-time constants
                    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                    int frameSkipMs = 0;

                    // Use compile-time frame rate limits based on current FPS
                    if (m_maxFrameRate <= 15) {
                        frameSkipMs = AdvancedCpp::FrameRateLimits<15>::frameSkipMs;
                    } else if (m_maxFrameRate <= 30) {
                        frameSkipMs = AdvancedCpp::FrameRateLimits<30>::frameSkipMs;
                    } else {
                        frameSkipMs = AdvancedCpp::FrameRateLimits<60>::frameSkipMs;
                    }

                    if (currentTime - m_lastFrameTime < frameSkipMs) {
                        return; // Skip this frame to maintain performance
                    }
                    m_lastFrameTime = currentTime;

                    // Update resolution if it changed
                    QSize currentResolution = frame.size();
                    if (m_videoResolution != currentResolution) {
                        m_videoResolution = currentResolution;
                        emit resolutionChanged(m_videoResolution);
                    }

                    // FPS tracking
                    if (!m_fpsTimer.isValid()) {
                        m_fpsTimer.start();
                        m_frameCount = 0;
                    }
                    m_frameCount++;

                    // Update FPS every second
                    if (m_fpsTimer.elapsed() >= 1000) {
                        m_currentFps = (m_frameCount * 1000.0) / m_fpsTimer.elapsed();
                        emit fpsChanged(m_currentFps);
                        m_fpsTimer.restart();
                        m_frameCount = 0;
                    }

                    // Calculate target size for aspect ratio
                    QSize labelSize = m_videoLabel->size();
                    QSize imageSize = frame.size();

                    if (!labelSize.isEmpty() && !imageSize.isEmpty()) {
                        QSize targetSize;

                        // Calculate size to fill the available frame while maintaining aspect ratio
                        double labelAspect = static_cast<double>(labelSize.width()) / labelSize.height();
                        double imageAspect = static_cast<double>(imageSize.width()) / imageSize.height();

                        if (imageAspect > labelAspect) {
                            // Image is wider - fit to width
                            targetSize.setWidth(labelSize.width());
                            targetSize.setHeight(static_cast<int>(labelSize.width() / imageAspect));
                        } else {
                            // Image is taller - fit to height
                            targetSize.setHeight(labelSize.height());
                            targetSize.setWidth(static_cast<int>(labelSize.height() * imageAspect));
                        }

                        // Send frame to processing thread using perfect forwarding
                        AdvancedCpp::processFrame(std::move(frame),
                            [this](auto&& f, auto&& size, auto&& quality) {
                                m_processingThread->processFrameGeneric(
                                    std::forward<decltype(f)>(f),
                                    std::forward<decltype(size)>(size),
                                    std::forward<decltype(quality)>(quality)
                                );
                            }, targetSize, m_videoQuality);
                        m_lastVideoSize = labelSize;
                    }
                } });

    // Connect processing thread result using perfect forwarding
    AdvancedCpp::safeConnect(m_processingThread.get(), &VideoProcessingThread::frameProcessed,
                             this, [this](const QPixmap &pixmap)
                             { m_videoLabel->setPixmap(pixmap); });
}

VideoWidgetQt::~VideoWidgetQt()
{
    // Need to stop resources gracefully first

    // Stop media player first with timeout protection
    if (m_mediaPlayer)
    {
        qDebug() << "Stopping media player...";
        qDebug() << "Media player state:" << m_mediaPlayer->playbackState();
        qDebug() << "Media player status:" << m_mediaPlayer->mediaStatus();

        // Disconnect signals first to prevent any callbacks during destruction
        m_mediaPlayer->disconnect();

        // Only call stop() if the media player is actually playing
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
        {
            qDebug() << "Media player is playing, calling stop()...";
            m_mediaPlayer->stop();
        }
        else
        {
            qDebug() << "Media player is already stopped, skipping stop() call";
        }

        // Clear source immediately - but skip if it might hang
        qDebug() << "About to clear media player source...";
        // Skip setSource() during destruction as it can hang
        // The media player will be destroyed anyway by the smart pointer
        qDebug() << "Skipping setSource() to avoid hang during destruction";

        qDebug() << "Media player cleanup completed";
    }

    // Stop the processing thread cooperatively, then destroy it. Resetting the
    // unique_ptr runs ~VideoProcessingThread, which performs stop() + wait() to
    // join the worker - no terminate(), no fixed timeout, no busy-wait.
    if (m_processingThread)
    {
        qDebug() << "Stopping processing thread...";
        m_processingThread->stop();
        m_processingThread.reset(); // joins the worker via its destructor
        qDebug() << "Processing thread cleanup completed";
    }

    // Smart pointers will automatically clean up in reverse order of declaration
    qDebug() << "VideoWidgetQt destructor completed";
}

void VideoWidgetQt::loadVideo(QString fileName)
{
    qDebug() << "loadVideo called with:" << fileName;

    if (!QFileInfo::exists(fileName))
    {
        qDebug() << "File does not exist:" << fileName;
        return;
    }

    m_isLoaded = false;
    m_duration = 0.0;
    // Move the filename to avoid copying
    QUrl url = QUrl::fromLocalFile(std::move(fileName));
    m_mediaPlayer->setSource(url);
    qDebug() << "Media source set to:" << url.toString();

    // QVideoWidget will handle the display automatically
    // Don't auto-play - let user control playback
}

void VideoWidgetQt::play()
{
    qDebug() << "play() called";
    if (m_mediaPlayer)
    {
        m_mediaPlayer->play();
        emit playbackStarted();
    }
}

void VideoWidgetQt::pause()
{
    qDebug() << "pause() called";
    if (m_mediaPlayer)
    {
        m_mediaPlayer->pause();
        emit playbackPaused();
    }
}

void VideoWidgetQt::stop()
{
    qDebug() << "stop() called";
    if (m_mediaPlayer)
    {
        m_mediaPlayer->stop();
        m_fpsTimer.invalidate(); // Reset FPS timer
        m_frameCount = 0;
        m_currentFps = 0.0;
        m_videoResolution = QSize(0, 0); // Reset resolution
    }
}

void VideoWidgetQt::seek(double position)
{
    qDebug() << "seek() called to position:" << position;
    if (m_mediaPlayer && m_duration > 0)
    {
        qint64 pos = static_cast<qint64>(position * 1000); // Convert to milliseconds
        m_mediaPlayer->setPosition(pos);
    }
}

double VideoWidgetQt::getCurrentPosition() const
{
    if (m_mediaPlayer)
    {
        return m_mediaPlayer->position() / 1000.0; // Convert from milliseconds
    }
    return 0.0;
}

bool VideoWidgetQt::isPlaying() const
{
    if (m_mediaPlayer)
    {
        return m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
    }
    return false;
}

double VideoWidgetQt::getDuration() const
{
    return m_duration;
}

void VideoWidgetQt::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "Media status changed to:" << status;

    switch (status)
    {
    case QMediaPlayer::LoadedMedia:
        qDebug() << "Media loaded successfully";
        m_isLoaded = true;
        emit videoLoaded();
        break;
    case QMediaPlayer::EndOfMedia:
        qDebug() << "End of media reached";
        emit playbackFinished();
        break;
    case QMediaPlayer::InvalidMedia:
        qDebug() << "Invalid media";
        break;
    default:
        break;
    }
}

void VideoWidgetQt::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    qDebug() << "Playback state changed to:" << state;
}

void VideoWidgetQt::onPositionChanged(qint64 position)
{
    double pos = position / 1000.0; // Convert from milliseconds
    emit positionChanged(pos);
}

void VideoWidgetQt::onDurationChanged(qint64 duration)
{
    m_duration = duration / 1000.0; // Convert from milliseconds
    qDebug() << "Duration changed to:" << m_duration << "seconds";
    emit durationChanged(m_duration);
}

void VideoWidgetQt::onErrorOccurred(QMediaPlayer::Error error, QString errorString)
{
    qDebug() << "Media player error:" << error << std::move(errorString);
}

void VideoWidgetQt::setMaxFrameRate(int fps)
{
    m_maxFrameRate = qBound(5, fps, 60); // Limit between 5-60 FPS
    qDebug() << "Max frame rate set to:" << m_maxFrameRate << "FPS";
}

void VideoWidgetQt::setVideoQuality(int quality)
{
    m_videoQuality = qBound(0, quality, 2); // 0=low, 1=medium, 2=high
    qDebug() << "Video quality set to:" << m_videoQuality;
}

void VideoWidgetQt::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Force a video frame update when resizing to maintain aspect ratio
    if (m_isLoaded && m_videoSink)
    {
        // Trigger a refresh of the current video frame
        m_lastVideoSize = QSize(0, 0); // Force recalculation
    }
}
