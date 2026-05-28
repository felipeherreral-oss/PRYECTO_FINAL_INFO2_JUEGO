#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <QGraphicsRectItem>

class Enemigo : public QGraphicsRectItem {
public:
    // El constructor recibe los parámetros iniciales de la física
    Enemigo(double xCentro, double yPos, double amplitud, double velAngular);

    // Método que calcula la nueva posición usando la ecuación física
    void actualizarFisica(double dt);

private:
    double x_centro;       // Punto de equilibrio (x0)
    double y_pos;          // Altura fija en la cancha
    double amplitud;       // Amplitud del movimiento (A)
    double omega;          // Velocidad angular (w)
    double tiempo;         // Variable de tiempo acumulado (t)
};

#endif // ENEMIGO_H
