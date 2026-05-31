#ifndef BALON_H
#define BALON_H

#include <QGraphicsEllipseItem>

class Balon : public QGraphicsEllipseItem {
public:
    // El constructor recibe la posición inicial y las velocidades de lanzamiento
    Balon(double xIni, double yIni, double vx0, double vy0);

    void actualizarFisica(double dt);
    bool estaActivo() const;

private:
    double x_inicial;
    double y_inicial;
    double v_x0;
    double v_y0;
    double g;       // Gravedad simulada en la cancha
    double tiempo;  // Tiempo interno del proyectil
    bool activo;    // Controla si el balón sigue en juego o cayó
};

#endif // BALON_H
