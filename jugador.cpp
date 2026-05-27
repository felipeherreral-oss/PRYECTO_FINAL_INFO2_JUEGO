#include "jugador.h"
#include <QGraphicsScene>
#include <QGraphicsView>

Jugador::Jugador() {
    // Definimos el tamaño del jugador (x, y, ancho, alto)
    // El punto (0,0) es el origen local del jugador
    setRect(0, 0, 50, 50);

    // Para que el objeto escuche los eventos del teclado, debe tener el "foco"
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
}

void Jugador::keyPressEvent(QKeyEvent *event) {
    int velocidad = 15; // Píxeles que se moverá por cada pulsación

    // Controles de movimiento
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        if (pos().x() > 0) // Límite izquierdo
            setPos(x() - velocidad, y());
    }
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        if (pos().x() + rect().width() < 800) // Límite derecho (ancho de escena)
            setPos(x() + velocidad, y());
    }
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        if (pos().y() > 0) // Límite superior
            setPos(x(), y() - velocidad);
    }
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        if (pos().y() + rect().height() < 3000) // Límite inferior (alto de escena)
            setPos(x(), y() + velocidad);
    }

    // MAGIA DEL SCROLL: Centrar la cámara en el jugador después de moverse
    // Obtenemos la escena, luego la vista, y le decimos que siga al jugador
    if(scene()) {
        QGraphicsView * view = scene()->views().first();
        view->centerOn(this);
    }
}
