#include "nivel1.h"
#include <QBrush>
#include <QPen>

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    // 1. Configurar la escena
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 3000);
    setScene(escena);

    // Pintar cancha y líneas de referencia
    escena->setBackgroundBrush(QBrush(QColor(222, 184, 135)));
    QPen lapizBlanco(Qt::white);
    lapizBlanco.setWidth(3);
    for (int i = 0; i <= 3000; i += 200) {
        escena->addLine(0, i, 800, i, lapizBlanco);
    }

    // 2. Configurar vista
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);

    // 3. Crear Jugador
    gidsel = new Jugador();
    gidsel->setPos(375, 2900);
    escena->addItem(gidsel);
    centerOn(gidsel);

    // ==================== NUEVO DE LA FASE 2 ====================

    // 4. Creación dinámica de enemigos a diferentes alturas (Y)
    // Parámetros: Enemigo(xCentro, yPos, amplitud/radio, velocidadAngular, TIPO)
    listaEnemigos.push_back(new Enemigo(400, 2500, 200, 3.0, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(200, 2100, 100, 2.5, Enemigo::VERTICAL_MAS));   // Sube y baja de forma armónica
    listaEnemigos.push_back(new Enemigo(400, 1600, 120, 2.0, Enemigo::CIRCULAR));       // Gira en círculos perfectos (¡Física 2!)
    listaEnemigos.push_back(new Enemigo(500, 1100, 150, 3.5, Enemigo::HORIZONTAL_MAS));
    listaEnemigos.push_back(new Enemigo(400, 600,  100, 1.8, Enemigo::CIRCULAR));       // Otro torbellino circular antes de la meta

    // Añadir todos los enemigos a la escena
    for (Enemigo* ene : listaEnemigos) {
        escena->addItem(ene);
    }

    // 5. Configurar el Game Loop (QTimer)
    relojJuego = new QTimer(this);
    // Conectamos el temporizador con nuestra función de actualización
    connect(relojJuego, SIGNAL(timeout()), this, SLOT(actualizarJuego()));
    // Arranca el reloj para que dispare cada 20 milisegundos (~50 FPS)
    relojJuego->start(20);
}

void Nivel1::actualizarJuego() {
    double dt = 0.02; // El diferencial de tiempo equivale a los 20ms del timer (0.02 segundos)

    // Actualizamos la física de cada uno de los enemigos en el vector
    for (Enemigo* ene : listaEnemigos) {
        ene->actualizarFisica(dt);
    }
}

Nivel1::~Nivel1() {
    delete gidsel;

    // Limpieza estricta de memoria dinámica para evitar leaks
    for (Enemigo* ene : listaEnemigos) {
        delete ene;
    }
    listaEnemigos.clear();
}
