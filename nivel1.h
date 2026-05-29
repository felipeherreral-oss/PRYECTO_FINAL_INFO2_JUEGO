#ifndef NIVEL1_H
#define NIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <vector> // Necesario para el contenedor dinámico
#include "jugador.h"
#include "enemigo.h"

class Nivel1 : public QGraphicsView {
    Q_OBJECT //  Esta macro permite usar funciones de Slots en Qt
public:
    Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

public slots:
    // Este método se ejecutará en cada tick del reloj
    void actualizarJuego();

private:
    QGraphicsScene *escena;
    Jugador *gidsel;
    QTimer *relojJuego;

    std::vector<Enemigo*> listaEnemigos; // Nuestro contenedor de punteros (Memoria dinámica)
};

#endif // NIVEL1_H
