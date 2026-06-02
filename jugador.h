#ifndef JUGADOR_H
#define JUGADOR_H

#include <QGraphicsPixmapItem>
#include <QMap>
#include <QPixmap>
#include <QKeyEvent>

class Jugador : public QGraphicsPixmapItem {
public:
    // Enumeración para las posibles direcciones y estados
    enum Direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA, PARADO_ATRAS, PARADO_ADELANTE };

    Jugador(QGraphicsItem *parent = nullptr);

    // Métodos para gestionar el estado del balón
    void setConBalon(bool balon);
    bool getConBalon() const;

    // Métodos para gestionar la dirección y el sprite
    void setDireccion(Direccion dir);
    Direccion getDireccion() const;
    void keyPressEvent(QKeyEvent *event) override;

protected:
    // Sobrecargamos el evento de teclado para el movimiento


private:
    void cargarSprites();
    void cambiarSpriteCorrecto();

    QMap<QString, QPixmap> sprites;
    bool conBalon;
    Direccion direccionActual;

    int velocidad;
};

#endif // JUGADOR_H
