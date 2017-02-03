#include "MMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {

    // Open up a QT application
    QApplication a(argc, argv);
    MMainWindow w;
    w.show();

    return a.exec();
}
