#include "nivel1.h"
#include <QBrush>
#include <QPen>

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 3000);
    setScene(escena);
    escena->setBackgroundBrush(QBrush(QColor(245, 222, 179))); // Madera pino claro

    // Dibujar líneas blancas horizontales
    QPen lapizBlanco(Qt::white);
    lapizBlanco.setWidth(3);
    for (int i = 0; i <= 3000; i += 200) {
        escena->addLine(0, i, 800, i, lapizBlanco);
    }

    // VISUAL DE LA ZONA DE RECARGA: Pintamos el área de inicio (abajo) con un suave tinte gris
    escena->addRect(0, 2800, 800, 200, QPen(Qt::transparent), QBrush(QColor(0, 0, 0, 20)));

    // Configurar Vista
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);

    // Crear Jugador
    gidsel = new Jugador();
    gidsel->setPos(375, 2900);
    escena->addItem(gidsel);
    centerOn(gidsel);

    // El balón inicia destruido/nulo ya que el jugador lo tiene guardado lógicamente
    balon = nullptr;

    // --- Enemigos de Alta Dificultad (Fase 2) ---
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

    // Configurar Game Loop
    relojJuego = new QTimer(this);
    connect(relojJuego, SIGNAL(timeout()), this, SLOT(actualizarJuego()));

    relojJuego->start(20);
}

void Nivel1::actualizarJuego() {
    double dt = 0.02;

    // 1. Actualizar física de enemigos
    for (Enemigo* ene : listaEnemigos) {
        ene->actualizarFisica(dt);
    }

    // 2. Verificar si el jugador presionó Espacio para disparar
    if (gidsel->consultarDisparo()) {
        if (balon != nullptr) {
            escena->removeItem(balon);
            delete balon; // Si había un balón perdido anterior, limpiamos memoria
        }
        // Instanciamos el balón dinámicamente en el centro del jugador
        // Lanzamiento: v_x0 = 0 (recto), v_y0 = -650 (velocidad inicial hacia arriba)
        balon = new Balon(gidsel->x() + 15, gidsel->y() - 15, 0, -650);
        escena->addItem(balon);
    }

    // 3. Actualizar la trayectoria parabólica del balón activo
    if (balon != nullptr) {
        balon->actualizarFisica(dt);

        // Si el tiro completó su parábola y cayó o salió del mapa, se destruye
        if (!balon->estaActivo()) {
            escena->removeItem(balon);
            delete balon; // Liberación estricta de memoria dinámica
            balon = nullptr;
        }
    }

    // 4. LÓGICA DE LA ZONA DE RECARGA
    // Si el jugador no tiene el balón y no hay ningún tiro en el aire actualmente
    if (!gidsel->getTieneBalon() && balon == nullptr) {
        // Si entra al área de su propia portería (Y > 2800)
        if (gidsel->pos().y() > 2800) {
            gidsel->setTieneBalon(true); // ¡Recarga exitosa!
        }
    }
}

Nivel1::~Nivel1() {
    delete gidsel;
    for (Enemigo* ene : listaEnemigos) {
        delete ene;
    }
    listaEnemigos.clear();

    // Limpieza final de seguridad del balón si se cierra el juego en pleno tiro
    if (balon != nullptr) {
        delete balon;
    }
}
