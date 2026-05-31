#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <vector>
#include "jugador.h"
#include "enemigo.h"
#include "balon.h"
#include "jeringa.h" // Cabecera del power-up

class Nivel1 : public QGraphicsView {
    Q_OBJECT
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

public slots:
    void actualizarJuego();
    void terminarPowerUp(); // Slot para apagar la supervelocidad tras 30s

private:
    QGraphicsScene *escena;
    Jugador *gidsel;
    QTimer *relojJuego;

    // === NUEVO FASE 4: Timers y Punteros del Power-up ===
    QTimer *timerPowerUp;
    Jeringa *jeringa;

    std::vector<Enemigo*> listaEnemigos;
    Balon *balon;
};

#endif // NIVEL1_H
