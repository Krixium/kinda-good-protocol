#include "KindaGoodProtocol.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KindaGoodProtocol w;
    w.show();
    return a.exec();
}
