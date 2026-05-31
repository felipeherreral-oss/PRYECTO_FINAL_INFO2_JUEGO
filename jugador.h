#ifndef JUGADOR_H
#define JUGADOR_H

#include <QGraphicsRectItem>
#include <QKeyEvent>

class Jugador : public QGraphicsRectItem {
public:
    Jugador();
    void keyPressEvent(QKeyEvent *event);

    bool getTieneBalon() const;
    void setTieneBalon(bool estado);
    bool consultarDisparo();

    // === NUEVO FASE 4: Métodos para el Power-Up ===
    void activarSuperVelocidad();
    void desactivarSuperVelocidad();
    void resetearPosicion(); // Para cuando lo golpeen

private:
    bool tieneBalon;
    bool quiereDisparar;

    // Variables de velocidad física
    int velocidadActual;
    int velocidadNormal;
    int velocidadRapida;
};

#endif // JUGADOR_H
