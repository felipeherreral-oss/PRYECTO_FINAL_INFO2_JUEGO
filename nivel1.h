#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QKeyEvent>
#include <QList>

class Jugador;
class Enemigo;
class Balon;
class Jeringa;

class Nivel1 : public QGraphicsView {
    Q_OBJECT
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void actualizarJuego();

private:
    void disminuirVida();

    QGraphicsScene *escena;
    Jugador *gidsel;
    Balon *balon;
    Jeringa *jeringa;

    Enemigo *arquero;
    QList<Enemigo*> listaEnemigos;

    QGraphicsRectItem *porteria;
    QGraphicsTextItem *textoPuntaje;
    QTimer *relojJuego;

    int goles;
    int vidas;

    // NUEVAS VARIABLES PARA PANTALLA COMPLETA DINÁMICA
    int anchoPantalla;
    int altoPantalla;
    int inicioXCancha;
};

#endif // NIVEL1_H
