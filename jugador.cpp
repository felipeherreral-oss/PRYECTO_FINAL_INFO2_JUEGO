#include "jugador.h"
#include <QGraphicsScene>
#include <QStringList>
#include <QBitmap> // <-- ¡Librería necesaria para manejar las máscaras de transparencia!

Jugador::Jugador(QGraphicsItem *parent) : QGraphicsPixmapItem(parent) {
    // Forzamos a Gidsel a estar en la capa superior (Z = 10) para que no lo tape la cancha
    setZValue(10);

    // Permitimos que el jugador capture el foco del teclado
    setFlag(QGraphicsItem::ItemIsFocusable);

    cargarSprites();
    conBalon = false;
    direccionActual = PARADO_ATRAS;
    velocidad = 7;

    cambiarSpriteCorrecto();
}

void Jugador::cargarSprites() {
    QStringList prefijos = { ":/imagenes/", ":/" };

    auto buscarImagen = [&](QString nombreArchivo) {
        for (const QString& prefijo : prefijos) {
            QPixmap pix(prefijo + nombreArchivo);
            if (!pix.isNull()) {


                // Crea una máscara que vuelve invisible el color blanco puro (Qt::white)
                QBitmap mascara = pix.createMaskFromColor(Qt::white, Qt::MaskInColor);
                pix.setMask(mascara);

                //EL TAMAÑO DEL JUGADOR

                pix = pix.scaled(70, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                return pix;
            }
        }

        // Sistema de emergencia si no encuentra el archivo
        QPixmap pixFallback(40, 60);
        pixFallback.fill(Qt::red);
        return pixFallback;
    };

    // Carga y procesamiento automático de toda la colección
    sprites["arriba_sin"] = buscarImagen("gidsel_adelante.png");
    sprites["arriba_con"] = buscarImagen("gidsel_adelante_balon.png");
    sprites["parado_arriba"] = buscarImagen("gidsel_quieto_subiendo.png");

    sprites["abajo_sin"] = buscarImagen("gidsel_bajando.png");
    sprites["abajo_con"] = buscarImagen("gidsel_bajando_balon.png");

    sprites["der_sin"] = buscarImagen("gidsel_der.png");
    sprites["der_con"] = buscarImagen("gidsel_der_balon.png");

    sprites["izq_sin"] = buscarImagen("gidsel_izq.png");
    sprites["izq_con"] = buscarImagen("gidsel_izq_balon.png");
}

void Jugador::setConBalon(bool balon) {
    conBalon = balon;
    cambiarSpriteCorrecto();
}

bool Jugador::getConBalon() const {
    return conBalon;
}

void Jugador::setDireccion(Direccion dir) {
    if (direccionActual != dir) {
        direccionActual = dir;
        cambiarSpriteCorrecto();
    }
}

Jugador::Direccion Jugador::getDireccion() const {
    return direccionActual;
}

void Jugador::cambiarSpriteCorrecto() {
    if (conBalon) {
        switch (direccionActual) {
        case ARRIBA:          setPixmap(sprites["arriba_con"]); break;
        case ABAJO:           setPixmap(sprites["abajo_con"]); break;
        case IZQUIERDA:       setPixmap(sprites["izq_con"]); break;
        case DERECHA:         setPixmap(sprites["der_con"]); break;
        case PARADO_ATRAS:    setPixmap(sprites["arriba_con"]); break;
        case PARADO_ADELANTE: setPixmap(sprites["abajo_con"]); break;
        default:              setPixmap(sprites["abajo_con"]); break;
        }
    } else {
        switch (direccionActual) {
        case ARRIBA:          setPixmap(sprites["arriba_sin"]); break;
        case ABAJO:           setPixmap(sprites["abajo_sin"]); break;
        case IZQUIERDA:       setPixmap(sprites["izq_sin"]); break;
        case DERECHA:         setPixmap(sprites["der_sin"]); break;
        case PARADO_ATRAS:    setPixmap(sprites["parado_arriba"]); break;
        case PARADO_ADELANTE: setPixmap(sprites["abajo_sin"]); break;
        default:              setPixmap(sprites["parado_arriba"]); break;
        }
    }
}

void Jugador::keyPressEvent(QKeyEvent *event) {
    QPointF posActual = pos();
    int dx = 0;
    int dy = 0;

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        dy = -velocidad;
        setDireccion(ARRIBA);
    } else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        dy = velocidad;
        setDireccion(ABAJO);
    } else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        dx = -velocidad;
        setDireccion(IZQUIERDA);
    } else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        dx = velocidad;
        setDireccion(DERECHA);
    }

    if (dx == 0 && dy == 0) {
        if (direccionActual == ARRIBA) {
            setDireccion(PARADO_ATRAS);
        } else if (direccionActual == ABAJO || direccionActual == IZQUIERDA || direccionActual == DERECHA) {
            setDireccion(PARADO_ADELANTE);
        }
    } else {
        setPos(posActual + QPointF(dx, dy));
    }
}
