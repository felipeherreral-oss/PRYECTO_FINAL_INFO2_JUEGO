#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <QGraphicsRectItem>

class Enemigo : public QGraphicsRectItem {
public:
    // Definimos los tipos de movimientos disponibles
    enum TipoMovimiento { HORIZONTAL_MAS, VERTICAL_MAS, CIRCULAR, TRAYECTORIA_L };

    // Añadimos el tipo de movimiento al constructor
    Enemigo(double xCentro, double yPos, double amplitud, double velAngular, TipoMovimiento tipoMov, bool esArquero = false);

    void actualizarFisica(double dt);

private:
    double x_centro;
    double y_pos;
    double amplitud;       // Funciona como Amplitud en MAS y como Radio en Circular
    double omega;
    double tiempo;

    TipoMovimiento tipo;   // Guarda qué movimiento hace este enemigo en específico

    QPixmap pixTaz1;
    QPixmap pixTaz2;
    QPixmap pixBossDer; // <--- NUEVA: Para Boss mirando a la derecha
    QPixmap pixBossIzq;
    int frameActual;
    int contadorFrames;
};

#endif // ENEMIGO_H
