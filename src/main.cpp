#include "hlsplay.h"
#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    hlsplay w;
    w.show();

    return app.exec();
}

