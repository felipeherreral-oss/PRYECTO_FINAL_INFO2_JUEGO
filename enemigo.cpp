#include "enemigo.h"
#include <cmath>
#include <QBrush>

Enemigo::Enemigo(double xCentro, double yPos, double amp, double velAngular, TipoMovimiento tipoMov) {
    x_centro = xCentro;
    y_pos = yPos;
    amplitud = amp;
    omega = velAngular;
    tiempo = 0.0;
    tipo = tipoMov; // Asignamos el tipo

    setRect(0, 0, 40, 40);
    setBrush(QBrush(Qt::red));

    setPos(x_centro, y_pos);
}

void Enemigo::actualizarFisica(double dt) {
    tiempo += dt;

    double nuevoX = x_centro;
    double nuevoY = y_pos;

    switch (tipo) {
    case HORIZONTAL_MAS:
        // Ecuación: x(t) = x0 + A * cos(w * t)
        nuevoX = x_centro + amplitud * std::cos(omega * tiempo);
        nuevoY = y_pos; // Altura fija
        break;

    case VERTICAL_MAS:
        // Ecuación: y(t) = y0 + A * cos(w * t)
        nuevoX = x_centro; // X fijo
        nuevoY = y_pos + amplitud * std::cos(omega * tiempo);
        break;

    case CIRCULAR:
        // Ecuaciones del Movimiento Circular Uniforme (MCU)
        // x(t) = x0 + R * cos(w * t)
        // y(t) = y0 + R * sin(w * t)
        nuevoX = x_centro + amplitud * std::cos(omega * tiempo);
        nuevoY = y_pos + amplitud * std::sin(omega * tiempo);
        break;
    }

    // Aplicamos los cambios calculados a la interfaz de Qt
    setPos(nuevoX, nuevoY);
}
