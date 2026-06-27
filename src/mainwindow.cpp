#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QCloseEvent>
#include <QThread>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_centralWidget(std::make_unique<QWidget>(this)), m_mainLayout(std::make_unique<QVBoxLayout>(m_centralWidget.get())), m_mediaSplitter(std::make_unique<QSplitter>(Qt::Horizontal, this)), m_mediaWidget1(std::make_unique<MediaWidget>(QString("Display 1"), this)), m_mediaWidget2(std::make_unique<MediaWidget>(QString("Display 2"), this))
{
    // Both widgets use software rendering
    m_mediaWidget1->setVideoWidgetType(MediaWidget::VideoWidgetType::Software);
    m_mediaWidget2->setVideoWidgetType(MediaWidget::VideoWidgetType::Software);
    std::cout << "Software rendering enabled for both displays" << std::endl;

    setupUI();
    setupConnections();
}

MainWindow::~MainWindow()
{
    // Stop and cleanup media widgets with timeout protection
    if (m_mediaWidget1)
    {
        m_mediaWidget1->stop();
        QApplication::processEvents(); // Process any pending events
        QThread::msleep(100);          // Give time for cleanup
    }

    if (m_mediaWidget2)
    {
        m_mediaWidget2->stop();
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
}

void MainWindow::setupUI()
{
    qDebug() << "Setting up UI...";

    setCentralWidget(m_centralWidget.get());

    // Configure main layout
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);

    // Add media widgets to splitter
    m_mediaSplitter->addWidget(m_mediaWidget1.get());
    m_mediaSplitter->addWidget(m_mediaWidget2.get());

    // Set equal sizes initially
    m_mediaSplitter->setSizes({1, 1});

    // Set splitter properties
    m_mediaSplitter->setChildrenCollapsible(false);
    m_mediaSplitter->setHandleWidth(8);
    m_mediaSplitter->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #cccccc;"
        "    border: 1px solid #999999;"
        "    margin: 2px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #aaaaaa;"
        "}");

    // Add splitter to main layout
    m_mainLayout->addWidget(m_mediaSplitter.get());

    // Set window properties
    setWindowTitle("MP4 Player - Dual Display");
    resize(1200, 600);

    qDebug() << "UI setup completed";
}

void MainWindow::setupConnections()
{
    qDebug() << "Setting up connections...";

    // Connect media widget signals if needed for global actions
    // For now, each MediaWidget is self-contained, so no additional connections needed

    qDebug() << "Connections setup completed";
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Keep the splitter centered when the main window is resized
    if (m_mediaSplitter)
    {
        // Set equal sizes to keep the splitter in the middle
        m_mediaSplitter->setSizes({1, 1});
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow closeEvent triggered";

    // Stop all media widgets before closing
    if (m_mediaWidget1)
    {
        qDebug() << "Stopping media widget 1 before close...";
        m_mediaWidget1->stop();
    }

    if (m_mediaWidget2)
    {
        qDebug() << "Stopping media widget 2 before close...";
        m_mediaWidget2->stop();
    }

    // Give more time for cleanup and force process events
    for (int i = 0; i < 5; ++i)
    {
        QApplication::processEvents();
        QThread::msleep(50); // Small delay to allow cleanup
    }

    qDebug() << "MainWindow closeEvent completed";

    // Accept the close event
    event->accept();
}
