#include "jeringa.h"
#include <QBrush>

Jeringa::Jeringa(double x, double y) {
    // Dibujamos la jeringa temporalmente como un rectángulo vertical magenta de 20x40
    setRect(0, 0, 20, 40);
    setBrush(QBrush(Qt::magenta));
    setPos(x, y);
}
