#include "nivel1.h"
#include <QBrush>
#include <QPen>

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

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);

    gidsel = new Jugador();
    gidsel->setPos(375, 2900);
    escena->addItem(gidsel);
    centerOn(gidsel);

    balon = nullptr;

    // === NUEVO FASE 4: Instanciar la Jeringa en la mitad de la cancha ===
    jeringa = new Jeringa(100, 1500);
    escena->addItem(jeringa);

    // Configurar Timer del Power-up (Un solo disparo / SingleShot)
    timerPowerUp = new QTimer(this);
    timerPowerUp->setSingleShot(true); // Se dispara una única vez al cumplir el tiempo
    connect(timerPowerUp, SIGNAL(timeout()), this, SLOT(terminarPowerUp()));

    // Enemigos de Alta Dificultad
    listaEnemigos.push_back(new Enemigo(400, 2500, 220, 3.0, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(400, 2500, 150, 3.0, Enemigo::VERTICAL_MAS));
    listaEnemigos.push_back(new Enemigo(150, 2100, 130, 1.2, Enemigo::TRAYECTORIA_L));
    listaEnemigos.push_back(new Enemigo(400, 1600, 120, 2.0, Enemigo::CIRCULAR));
    listaEnemigos.push_back(new Enemigo(450, 1200, 110, 1.5, Enemigo::TRAYECTORIA_L));
    listaEnemigos.push_back(new Enemigo(400, 800,  250, 3.5, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(400, 450,  100, 2.2, Enemigo::CIRCULAR));

    for (Enemigo* ene : listaEnemigos) {
        escena->addItem(ene);
    }

    relojJuego = new QTimer(this);
    connect(relojJuego, SIGNAL(timeout()), this, SLOT(actualizarJuego()));
    relojJuego->start(20);
}

void Nivel1::actualizarJuego() {
    double dt = 0.02;

    // 1. Actualizar física de enemigos
    for (Enemigo* ene : listaEnemigos) {
        ene->actualizarFisica(dt);

        // !!! NUEVO FASE 4: COLISIÓN JUGADOR - ENEMIGO !!!
        if (gidsel->collidesWithItem(ene)) {
            gidsel->resetearPosicion(); // Lo regresa al inicio y pierde poderes
            centerOn(gidsel);
        }
    }

    // 2. Verificar Disparo
    if (gidsel->consultarDisparo()) {
        if (balon != nullptr) {
            escena->removeItem(balon);
            delete balon;
        }
        balon = new Balon(gidsel->x() + 15, gidsel->y() - 15, 0, -650);
        escena->addItem(balon);
    }

    // 3. Física Balón
    if (balon != nullptr) {
        balon->actualizarFisica(dt);
        if (!balon->estaActivo()) {
            escena->removeItem(balon);
            delete balon;
            balon = nullptr;
        }
    }

    // 4. Lógica de Zona de Recarga
    if (!gidsel->getTieneBalon() && balon == nullptr) {
        if (gidsel->pos().y() > 2800) {
            gidsel->setTieneBalon(true);
        }
    }

    // 5. !!! NUEVO FASE 4: COLISIÓN JUGADOR - JERINGA !!!
    if (jeringa != nullptr && gidsel->collidesWithItem(jeringa)) {
        gidsel->activarSuperVelocidad(); // Duplica su velocidad

        escena->removeItem(jeringa); // Lo saca del render
        delete jeringa;              // Libera memoria dinámica
        jeringa = nullptr;           // Evita punteros colgados

        // Iniciamos la cuenta regresiva estricta de 30 segundos (30000 milisegundos)
        timerPowerUp->start(30000);
    }
}

// === NUEVO FASE 4: Slot de Apagado automático ===
void Nivel1::terminarPowerUp() {
    gidsel->desactivarSuperVelocidad(); // El jugador vuelve a su estado normal
}

Nivel1::~Nivel1() {
    delete gidsel;
    for (Enemigo* ene : listaEnemigos) {
        delete ene;
    }
    listaEnemigos.clear();

    if (balon != nullptr) delete balon;

    // Limpieza de seguridad por si se cierra el juego antes de recoger la jeringa
    if (jeringa != nullptr) delete jeringa;
}
