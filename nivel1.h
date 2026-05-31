#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <vector>
#include "jugador.h"
#include "enemigo.h"
#include "balon.h"
#include "jeringa.h"

class Nivel1 : public QGraphicsView {
    Q_OBJECT
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

public slots:
    void actualizarJuego();
    void terminarPowerUp();

private:
    QGraphicsScene *escena;
    Jugador *gidsel;
    QTimer *relojJuego;

    QTimer *timerPowerUp;
    Jeringa *jeringa;

    std::vector<Enemigo*> listaEnemigos;
    Balon *balon;

    QGraphicsRectItem *porteria;
    QGraphicsTextItem *textoPuntaje;
    int goles;
    int vidas; // Nueva variable para controlar las oportunidades
};

#endif // NIVEL1_H
