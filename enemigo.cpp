#include "enemigo.h"
#include <cmath>   // Para usar la función cos()
#include <QBrush>

Enemigo::Enemigo(double xCentro, double yPos, double amp, double velAngular) {
    x_centro = xCentro;
    y_pos = yPos;
    amplitud = amp;
    omega = velAngular;
    tiempo = 0.0; // El tiempo arranca en cero

    // Dibujamos al enemigo temporalmente como un cuadrado rojo de 40x40
    setRect(0, 0, 40, 40);
    setBrush(QBrush(Qt::red));

    // Lo colocamos en su posición inicial
    setPos(x_centro + amplitud, y_pos);
}

void Enemigo::actualizarFisica(double dt) {
    tiempo += dt; // Incrementamos el paso del tiempo

    // Implementación matemática estricta del MAS:
    // x(t) = x0 + A * cos(w * t)
    double nuevoX = x_centro + amplitud * std::cos(omega * tiempo);

    // Actualizamos la posición en la escena de Qt (Y se mantiene constante)
    setPos(nuevoX, y_pos);
}
