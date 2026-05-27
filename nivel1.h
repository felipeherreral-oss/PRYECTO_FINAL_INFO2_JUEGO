#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "jugador.h"

// Le decimos manualmente que herede de QGraphicsView
class Nivel1 : public QGraphicsView {
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

private:
    QGraphicsScene *escena;
    Jugador *gidsel;
};

#endif // NIVEL1_H
