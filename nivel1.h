#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <vector>
#include "jugador.h"
#include "enemigo.h"
#include "balon.h" // Incluimos la nueva cabecera

class Nivel1 : public QGraphicsView {
    Q_OBJECT
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

public slots:
    void actualizarJuego();

private:
    QGraphicsScene *escena;
    Jugador *gidsel;
    QTimer *relojJuego;
    std::vector<Enemigo*> listaEnemigos;

    Balon *balon; // Puntero dinámico para controlar el balón en el aire
};

#endif // NIVEL1_H
