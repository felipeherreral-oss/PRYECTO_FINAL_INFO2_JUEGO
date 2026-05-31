#include "balon.h"
#include <QBrush>

Balon::Balon(double xIni, double yIni, double vx0, double vy0) {
    x_inicial = xIni;
    y_inicial = yIni;
    v_x0 = vx0;
    v_y0 = vy0;   // Debe ser un valor negativo alto (ej. -600) para disparar hacia arriba
    g = 450.0;    // Gravedad que tira el balón hacia abajo (+Y en Qt)
    tiempo = 0.0;
    activo = true;

    // Dibujamos el balón como un círculo naranja de 20x20 píxeles
    setRect(0, 0, 20, 20);
    setBrush(QBrush(QColor(255, 140, 0))); // Color Naranja Balonmano
    setPos(x_inicial, y_inicial);
}

void Balon::actualizarFisica(double dt) {
    if (!activo) return;

    tiempo += dt;

    // !!! MODELO FÍSICO 3: Ecuaciones de Movimiento Parabólico (2D) !!!
    double nuevoX = x_inicial + v_x0 * tiempo;
    double nuevoY = y_inicial + v_y0 * tiempo + 0.5 * g * tiempo * tiempo;

    setPos(nuevoX, nuevoY);

    // Si el balón supera los límites lógicos de la cancha, se desactiva
    if (nuevoY > 3000 || nuevoY < 0 || nuevoX < 0 || nuevoX > 800) {
        activo = false;
    }
}

bool Balon::estaActivo() const { return activo; }
