
#include "MainWindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Load Styles
    QFile styleFile(":/aero.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(styleFile.readAll());
    }

    MainWindow window;
    window.resize(1024, 700);
    window.show();

    return app.exec();
}
