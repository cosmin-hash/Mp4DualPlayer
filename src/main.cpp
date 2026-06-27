#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <iostream>
#include "mainwindow.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{
// Enable console window for Windows to see debug output
#ifdef _WIN32
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE **)stdin, "CONIN$", "r", stdin);
#endif

    // Configure Qt logging rules to suppress noisy painting/QPA messages
    // while preserving other debug output. We set rules in a single
    // environment variable (overwriting previous single-category calls),
    // and force logging to stderr so messages appear in the console.
    QByteArray loggingRules;
#ifdef DEBUG
    // In debug builds, keep useful multimedia debug info but disable
    // very noisy GUI painting and QPA internals.
    loggingRules = "qt.widgets.painting=false;qt.qpa.window=false;qt.qpa.events=false;qt.multimedia.debug=true";
#else
    // In non-debug builds, disable the noisy categories and other
    // subsystems we don't want printed in normal runs.
    loggingRules = "qt.widgets.painting=false;qt.qpa.window=false;qt.qpa.events=false;qt.multimedia.debug=false;qt.opengl.debug=false";
#endif
    qputenv("QT_LOGGING_RULES", loggingRules);
    qputenv("QT_FORCE_STDERR_LOGGING", "1");

    std::cout << "MPEG4 Player - Software Video Player" << std::endl;
    std::cout << "====================================" << std::endl;

    // Check for help argument
    for (int i = 1; i < argc; ++i)
    {
        QString arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: Mpeg4Player [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --help, -h      Show this help" << std::endl;
            return 0;
        }
    }

    std::cout << "Using software rendering mode..." << std::endl;

    std::cout << "Creating QApplication..." << std::endl;
    QApplication app(argc, argv);
    std::cout << "QApplication created" << std::endl;
    qDebug() << "QApplication created";

    std::cout << "Creating MainWindow..." << std::endl;
    MainWindow window;
    std::cout << "MainWindow created" << std::endl;
    qDebug() << "MainWindow created";

    std::cout << "Showing window..." << std::endl;
    window.show();
    std::cout << "Window shown" << std::endl;
    qDebug() << "Window shown";

    std::cout << "Starting event loop..." << std::endl;
    qDebug() << "Starting event loop...";

    int result = app.exec();

    // Ensure all resources are cleaned up before exit
    qDebug() << "Application event loop finished, cleaning up...";
    QApplication::processEvents();

    // Give a minimal delay to ensure all threads finish cleanup
    QThread::msleep(50);

    // Force process any remaining events
    QApplication::processEvents();

    std::cout << "Application exiting with code:" << result << std::endl;
    qDebug() << "Application exiting with code:" << result;

    return result;
}
