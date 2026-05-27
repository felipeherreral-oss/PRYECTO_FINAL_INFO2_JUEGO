#include "nivel1.h"

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    // 1. Configurar la escena (nuestra cancha gigante)
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 3000); // 3000 de alto para el scroll
    setScene(escena);

    // 1.5 Pintar la cancha de verde
    escena->setBackgroundBrush(QBrush(QColor(34, 139, 34))); // Verde pasto/cancha

    // Dibujar líneas blancas horizontales cada 200 píxeles para notar el scroll
    QPen lapizBlanco(Qt::white);
    lapizBlanco.setWidth(3); // Líneas gruesas

    for (int i = 0; i <= 3000; i += 200) {
        escena->addLine(0, i, 800, i, lapizBlanco);
    }

    // 2. Configurar la vista (la cámara/ventana)
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600); // Lo que el usuario ve

    // 3. Crear al Jugador usando memoria dinámica
    gidsel = new Jugador();
    gidsel->setPos(375, 2900); // Posición inicial abajo
    escena->addItem(gidsel);

    // 4. Enfocar la cámara en él
    centerOn(gidsel);
}

Nivel1::~Nivel1() {
    delete gidsel;
}
