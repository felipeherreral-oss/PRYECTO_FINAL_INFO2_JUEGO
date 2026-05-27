#ifndef JUGADOR_H
#define JUGADOR_H

#include <QGraphicsRectItem>
#include <QKeyEvent>

// Heredamos de QGraphicsRectItem para que sea visible en la escena
class Jugador : public QGraphicsRectItem {
public:
    Jugador();
    // Método nativo de Qt para detectar cuando se presiona una tecla
    void keyPressEvent(QKeyEvent *event);
};

#endif // JUGADOR_H
