#include <QApplication>
#include "nivel1.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Instanciamos y mostramos el nivel
    Nivel1 *nivel = new Nivel1();
    nivel->show();

    return a.exec();
}
