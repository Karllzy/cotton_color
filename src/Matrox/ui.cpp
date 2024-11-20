//
// Created by zjc on 24-11-12.
//

#include "ui.h"
#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QPushButton button("Hello, Qt!");
    button.show();
    return app.exec();
}