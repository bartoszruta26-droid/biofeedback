/**
 * @file qt5_mandatory.cpp
 * @brief Minimal mandatory Qt5 code example.
 */

#include <QApplication>
#include <QLabel>
#include <QString>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QLabel label(QStringLiteral("Biofeedback: Qt5 example is active."));
    label.setMinimumSize(420, 80);
    label.show();

    return app.exec();
}
