#include "nivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 3000);
    setScene(escena);
    escena->setBackgroundBrush(QBrush(QColor(245, 222, 179)));

    QPen lapizBlanco(Qt::white);
    lapizBlanco.setWidth(3);
    for (int i = 0; i <= 3000; i += 200) {
        escena->addLine(0, i, 800, i, lapizBlanco);
    }

    escena->addRect(0, 2800, 800, 200, QPen(Qt::transparent), QBrush(QColor(0, 0, 0, 20)));

    // Variables de control de la partida
    goles = 0;
    vidas = 3;

    // Portería
    porteria = new QGraphicsRectItem(300, 0, 200, 50);
    porteria->setBrush(QBrush(Qt::white));
    porteria->setPen(QPen(Qt::black, 3));
    escena->addItem(porteria);

    // Texto de Puntaje y Vidas (HUD)
    textoPuntaje = new QGraphicsTextItem("Goles: 0 / 3   |   Vidas: 3");
    QFont fuente("Arial", 16, QFont::Bold);
    textoPuntaje->setFont(fuente);
    textoPuntaje->setDefaultTextColor(Qt::darkGreen);
    textoPuntaje->setZValue(100); // Para que siempre esté por encima del jugador y los bordes
    escena->addItem(textoPuntaje);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);

    gidsel = new Jugador();
    gidsel->setPos(375, 2900);
    escena->addItem(gidsel);
    centerOn(gidsel);

    balon = nullptr;

    jeringa = new Jeringa(100, 1500);
    escena->addItem(jeringa);

    timerPowerUp = new QTimer(this);
    timerPowerUp->setSingleShot(true);
    connect(timerPowerUp, SIGNAL(timeout()), this, SLOT(terminarPowerUp()));

    // === ENEMIGOS REAJUSTADOS (Sin zonas seguras + Arquero) ===

    // 1. EL ARQUERO: Justo frente a la portería, movimiento rapidísimo
    listaEnemigos.push_back(new Enemigo(400, 60, 350, 4.5, Enemigo::HORIZONTAL_MAS));

    // 2. Aduana Central: Ahora cubren de pared a pared (Amplitud 380)
    listaEnemigos.push_back(new Enemigo(400, 2500, 380, 2.5, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(400, 2500, 150, 3.0, Enemigo::VERTICAL_MAS));

    listaEnemigos.push_back(new Enemigo(200, 2100, 180, 1.2, Enemigo::TRAYECTORIA_L));
    listaEnemigos.push_back(new Enemigo(400, 1600, 350, 2.0, Enemigo::CIRCULAR));
    listaEnemigos.push_back(new Enemigo(600, 1200, 180, 1.5, Enemigo::TRAYECTORIA_L));
    listaEnemigos.push_back(new Enemigo(400, 800,  380, 3.5, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(400, 450,  350, 2.2, Enemigo::CIRCULAR));

    for (Enemigo* ene : listaEnemigos) {
        escena->addItem(ene);
    }

    relojJuego = new QTimer(this);
    connect(relojJuego, SIGNAL(timeout()), this, SLOT(actualizarJuego()));
    relojJuego->start(20);
}

void Nivel1::actualizarJuego() {
    double dt = 0.02;

    // HUD FIJO: Lo anclamos siempre a la esquina superior izquierda de la vista actual
    textoPuntaje->setPos(mapToScene(15, 15));

    for (Enemigo* ene : listaEnemigos) {
        ene->actualizarFisica(dt);

        // COLISIÓN: Lógica de pérdida de vidas
        if (gidsel->collidesWithItem(ene)) {
            vidas--; // Resta una vida
            gidsel->resetearPosicion();
            centerOn(gidsel);

            if (vidas > 0) {
                textoPuntaje->setPlainText("Goles: " + QString::number(goles) + " / 3   |   Vidas: " + QString::number(vidas));
            } else {
                // GAME OVER
                textoPuntaje->setPlainText("¡GAME OVER! Te quedaste sin vidas.");
                textoPuntaje->setDefaultTextColor(Qt::red);
                relojJuego->stop(); // Congela el juego por derrota
                return; // Sale de la función para evitar procesar más colisiones
            }
        }
    }

    if (gidsel->consultarDisparo()) {
        if (balon != nullptr) {
            escena->removeItem(balon);
            delete balon;
        }
        balon = new Balon(gidsel->x() + 15, gidsel->y() - 15, 0, -650);
        escena->addItem(balon);
    }

    if (balon != nullptr) {
        balon->actualizarFisica(dt);

        if (balon->collidesWithItem(porteria)) {
            goles++;

            if (goles >= 3) {
                textoPuntaje->setPlainText("¡VICTORIA! Eres el campeon.");
                textoPuntaje->setDefaultTextColor(Qt::blue);
                relojJuego->stop();
            } else {
                textoPuntaje->setPlainText("Goles: " + QString::number(goles) + " / 3   |   Vidas: " + QString::number(vidas));
            }

            escena->removeItem(balon);
            delete balon;
            balon = nullptr;
        }
        else if (!balon->estaActivo()) {
            escena->removeItem(balon);
            delete balon;
            balon = nullptr;
        }
    }

    if (!gidsel->getTieneBalon() && balon == nullptr) {
        if (gidsel->pos().y() > 2800) {
            gidsel->setTieneBalon(true);
        }
    }

    if (jeringa != nullptr && gidsel->collidesWithItem(jeringa)) {
        gidsel->activarSuperVelocidad();
        escena->removeItem(jeringa);
        delete jeringa;
        jeringa = nullptr;
        timerPowerUp->start(30000);
    }
}

void Nivel1::terminarPowerUp() {
    gidsel->desactivarSuperVelocidad();
}

Nivel1::~Nivel1() {
    delete gidsel;
    for (Enemigo* ene : listaEnemigos) {
        delete ene;
    }
    listaEnemigos.clear();

    if (balon != nullptr) delete balon;
    if (jeringa != nullptr) delete jeringa;
}
