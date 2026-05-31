#ifndef JUGADOR_H
#define JUGADOR_H

#include <QGraphicsRectItem>
#include <QKeyEvent>

class Jugador : public QGraphicsRectItem {
public:
    Jugador();
    void keyPressEvent(QKeyEvent *event);

    // Métodos lógicos para la posesión (Fase 3)
    bool getTieneBalon() const;
    void setTieneBalon(bool estado);
    bool consultarDisparo(); // Avisa al nivel si el usuario presionó espacio

private:
    bool tieneBalon;
    bool quiereDisparar; // Bandera de comunicación con el bucle de juego
};

#endif // JUGADOR_H
