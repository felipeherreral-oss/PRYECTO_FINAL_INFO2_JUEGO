#include "jugador.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBrush>

Jugador::Jugador() {
    setRect(0, 0, 50, 50);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();

    // Al iniciar el juego, Gidsel empieza con el balón
    tieneBalon = true;
    quiereDisparar = false;
    setBrush(QBrush(Qt::blue)); // Color azul: Tiene posesión
}

bool Jugador::getTieneBalon() const { return tieneBalon; }

void Jugador::setTieneBalon(bool estado) {
    tieneBalon = estado;
    if (tieneBalon) {
        setBrush(QBrush(Qt::blue)); // Azul: Listo para disparar
    } else {
        setBrush(QBrush(QColor(100, 149, 237))); // Azul claro: Sin balón
    }
}

bool Jugador::consultarDisparo() {
    if (quiereDisparar) {
        quiereDisparar = false; // Reseteamos la bandera
        return true;
    }
    return false;
}

void Jugador::keyPressEvent(QKeyEvent *event) {
    int velocidad = 15;

    // Controles de movimiento estándar
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        if (pos().x() > 0) setPos(x() - velocidad, y());
    }
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        if (pos().x() + rect().width() < 800) setPos(x() + velocidad, y());
    }
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        if (pos().y() > 0) setPos(x(), y() - velocidad);
    }
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        if (pos().y() + rect().height() < 3000) setPos(x(), y() + velocidad);
    }
    // LÓGICA DE DISPARO: Al presionar Espacio lanza el balón si lo tiene
    else if (event->key() == Qt::Key_Space) {
        if (tieneBalon) {
            setTieneBalon(false); // Pierde el balón inmediatamente
            quiereDisparar = true; // Levanta la mano para que el nivel dibuje el balón
        }
    }

    if(scene()) {
        QGraphicsView * view = scene()->views().first();
        view->centerOn(this);
    }
}
