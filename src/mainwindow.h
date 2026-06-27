#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include <memory>
#include "mediawidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Move semantics for performance
    MainWindow(MainWindow &&other) noexcept = default;
    MainWindow &operator=(MainWindow &&other) noexcept = default;

    // Disable copy semantics (Qt objects should not be copied)
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupConnections();

    std::unique_ptr<QWidget> m_centralWidget;
    std::unique_ptr<QVBoxLayout> m_mainLayout;
    std::unique_ptr<QSplitter> m_mediaSplitter;

    // Media widgets
    std::unique_ptr<MediaWidget> m_mediaWidget1;
    std::unique_ptr<MediaWidget> m_mediaWidget2;
};
