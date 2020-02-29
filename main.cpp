#include "symphonium.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Symphonium w;
    w.show();
    return a.exec();
}
