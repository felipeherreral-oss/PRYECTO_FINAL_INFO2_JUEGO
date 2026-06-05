#include "enemigo.h"
#include <cmath>
#include <QBrush>
#include <QPixmap>
#include <QPen>

Enemigo::Enemigo(double xCentro, double yPos, double amp, double velAngular, TipoMovimiento tipoMov, bool esArquero) {
    x_centro = xCentro;
    y_pos = yPos;
    amplitud = amp;
    omega = velAngular;
    tiempo = 0.0;
    tipo = tipoMov; // Asignamos el tipo

    // === INICIALIZAR VARIABLES DE ANIMACIÓN ===
    frameActual = 1;
    contadorFrames = 0;

    // Cargar las imágenes en la memoria (Ajusta los tamaños 60x60 o 65x65 a tu gusto)
    pixTaz1 = QPixmap(":/imagenes/giro_1.png").scaled(80, 80);
    pixTaz2 = QPixmap(":/imagenes/giro_2.png").scaled(80, 80);
    pixBossDer = QPixmap(":/imagenes/boss_H_der.png").scaled(80, 80);
    pixBossIzq = QPixmap(":/imagenes/boss_H_izq.png").scaled(80, 80);

    // ====================================================
    // === ASIGNACIÓN DE SPRITES SEGÚN TIPO DE ENEMIGO ===
    // ====================================================
    if (esArquero) {
        // 1. EL ARQUERO (Tazmania)
        setRect(0, 0, 80, 80);
        setBrush(QBrush(QPixmap(":/imagenes/tazmania_arquero.png").scaled(80, 80)));
        setPen(Qt::NoPen);

    } else if (tipo == CIRCULAR) {
        // 2. TORNADO (Animado)
        setRect(0, 0, 80, 80);
        setBrush(QBrush(pixTaz1));
        setPen(Qt::NoPen);

    } else if (tipo == TRAYECTORIA_L) {
        // 3. PATO LUCAS EN L
        setRect(0, 0, 80, 80);
        setBrush(QBrush(QPixmap(":/imagenes/lucas_izq.png").scaled(80, 80)));
        setPen(Qt::NoPen);

    } else if (tipo == VERTICAL_MAS) {
        // 4. LUCAS VERTICAL
        setRect(0, 0, 80, 80);
        setBrush(QBrush(QPixmap(":/imagenes/lucas_v.png").scaled(80, 80)));
        setPen(Qt::NoPen);

    } else if (tipo == HORIZONTAL_MAS) {
        // 5. BOSS HORIZONTAL (Inicia mirando a la derecha por defecto)
        setRect(0, 0, 80, 80);
        setBrush(QBrush(pixBossDer));
        setPen(Qt::NoPen);

    } else {
        // 6. POR DEFECTO
        setRect(0, 0, 40, 40);
        setBrush(QBrush(Qt::red));
    }
}

void Enemigo::actualizarFisica(double dt) {
    tiempo += dt;

    double nuevoX = x_centro;
    double nuevoY = y_pos;

    // === ANIMACIÓN DEL TORNADO ===
    if (tipo == CIRCULAR) {
        contadorFrames++;
        if (contadorFrames >= 5) {
            contadorFrames = 0;
            if (frameActual == 1) {
                setBrush(QBrush(pixTaz2));
                frameActual = 2;
            } else {
                setBrush(QBrush(pixTaz1));
                frameActual = 1;
            }
        }
    }

    // === CÁLCULO DE MOVIMIENTO ===
    switch (tipo) {
    case HORIZONTAL_MAS:
        nuevoX = x_centro + amplitud * std::cos(omega * tiempo);
        nuevoY = y_pos;
        break;

    case VERTICAL_MAS:
        nuevoX = x_centro;
        nuevoY = y_pos + amplitud * std::cos(omega * tiempo);
        break;

    case CIRCULAR:
        nuevoX = x_centro + amplitud * std::cos(omega * tiempo);
        nuevoY = y_pos + amplitud * std::sin(omega * tiempo);
        break;

    case TRAYECTORIA_L: {
        double distancia = omega * 100.0 * tiempo;
        if (distancia < amplitud) {
            nuevoX = x_centro + distancia;
            nuevoY = y_pos;
        } else if (distancia < 2 * amplitud) {
            nuevoX = x_centro + amplitud;
            nuevoY = y_pos + (distancia - amplitud);
        } else if (distancia < 3 * amplitud) {
            nuevoX = x_centro + amplitud;
            nuevoY = y_pos + amplitud - (distancia - 2 * amplitud);
        } else if (distancia < 4 * amplitud) {
            nuevoX = x_centro + amplitud - (distancia - 3 * amplitud);
            nuevoY = y_pos;
        } else {
            tiempo = 0.0;
            nuevoX = x_centro;
            nuevoY = y_pos;
        }
    }
    }

    // === NUEVO: CAMBIO DE SPRITE POR DIRECCIÓN PARA EL BOSS HORIZONTAL ===
    if (tipo == HORIZONTAL_MAS) {
        // Comparamos el nuevoX con la posición actual (pos().x())
        if (nuevoX > pos().x()) {
            // Se está moviendo hacia la derecha
            setBrush(QBrush(pixBossDer));
        } else if (nuevoX < pos().x()) {
            // Se está moviendo hacia la izquierda
            setBrush(QBrush(pixBossIzq));
        }
    }

    // Aplicamos la nueva posición en la interfaz de Qt
    setPos(nuevoX, nuevoY);
}
