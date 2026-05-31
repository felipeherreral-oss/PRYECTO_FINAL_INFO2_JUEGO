#include "jugador.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBrush>

Jugador::Jugador() {
    setRect(0, 0, 50, 50);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    tieneBalon = true;
    quiereDisparar = false;

    // Inicializamos las velocidades físicas
    velocidadNormal = 15;
    velocidadRapida = 30; // ¡El doble de rápido!
    velocidadActual = velocidadNormal;

    setBrush(QBrush(Qt::blue));
}

bool Jugador::getTieneBalon() const { return tieneBalon; }

void Jugador::setTieneBalon(bool estado) {
    tieneBalon = estado;
    if (tieneBalon) {
        setBrush(QBrush(Qt::blue));
    } else {
        setBrush(QBrush(QColor(100, 149, 237)));
    }
}

bool Jugador::consultarDisparo() {
    if (quiereDisparar) {
        quiereDisparar = false;
        return true;
    }
    return false;
}

// === NUEVO FASE 4: Funciones del Power-Up ===
void Jugador::activarSuperVelocidad() {
    velocidadActual = velocidadRapida;
    // Efecto visual: Cambia a Cyan para notar el poder activo
    setBrush(QBrush(Qt::cyan));
}

void Jugador::desactivarSuperVelocidad() {
    velocidadActual = velocidadNormal;
    // Volvemos al color original según tenga o no el balón
    setTieneBalon(tieneBalon);
}

void Jugador::resetearPosicion() {
    setPos(375, 2900); // Regresa al inicio de la cancha
    desactivarSuperVelocidad(); // Pierde el poder si es golpeado
}

void Jugador::keyPressEvent(QKeyEvent *event) {
    // Reemplazamos la velocidad estática por la variable física adaptativa
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        if (pos().x() > 0) setPos(x() - velocidadActual, y());
    }
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        if (pos().x() + rect().width() < 800) setPos(x() + velocidadActual, y());
    }
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        if (pos().y() > 0) setPos(x(), y() - velocidadActual);
    }
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        if (pos().y() + rect().height() < 3000) setPos(x(), y() + velocidadActual);
    }
    else if (event->key() == Qt::Key_Space) {
        if (tieneBalon) {
            setTieneBalon(false);
            quiereDisparar = true;
        }
    }

    if(scene()) {
        QGraphicsView * view = scene()->views().first();
        view->centerOn(this);
    }
}
