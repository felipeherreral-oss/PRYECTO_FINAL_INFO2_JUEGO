#include "nivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 3000);
    setScene(escena);
    escena->setBackgroundBrush(QBrush(QPixmap(":/imagenes/madera.png")));


    // === TRIBUNAS LATERALES REPETIDAS AFUERA ===

    QPixmap pixIzquierdo = QPixmap(":/imagenes/tribuna_izq.png").scaled(250, 1000);
    QPixmap pixDerecho = QPixmap(":/imagenes/tribuna_der.png").scaled(250, 1000);

    // Ciclo para repetir las imágenes verticalmente cada 600 píxeles
    for (int yActual = 0; yActual < 3000; yActual += 600) {
        // Izquierda: Colocada afuera en la zona negativa (-150)
        QGraphicsPixmapItem *ti = new QGraphicsPixmapItem(pixIzquierdo);
        ti->setPos(-250, yActual);
        ti->setZValue(-10);
        escena->addItem(ti);

        // Derecha: Colocada afuera justo donde termina la cancha (800)
        QGraphicsPixmapItem *td = new QGraphicsPixmapItem(pixDerecho);
        td->setPos(800, yActual);
        td->setZValue(-10);
        escena->addItem(td);
    }
    // ===========================================
    // ==================================


    // === ESTÉTICA DE CANCHA REAL ===
    QPen lapizBlanco(Qt::white);
    lapizBlanco.setWidth(4); // Un poco más grueso para que resalte bien sobre la madera

    // 1. Líneas de banda externas (Delimitan el rectángulo de juego 800x3000)
    escena->addRect(0, 0, 800, 3000, lapizBlanco);

    // 2. Línea de Mitad de Cancha (Justo en la mitad del scroll: Y = 1500)
    escena->addLine(0, 1500, 800, 1500, lapizBlanco);

    // 3. Círculo Central (Centro en X=400, Y=1500)
    escena->addEllipse(400 - 120, 1500 - 120, 240, 240, lapizBlanco);

    // 4. Área Superior (Zona del arco de arriba)
    escena->addRect(200, 0, 400, 250, lapizBlanco);   // Área grande
    escena->addRect(300, 0, 200, 90, lapizBlanco);    // Área chica

    // 5. Área Inferior (Zona del arco de abajo)
    escena->addRect(200, 2750, 400, 250, lapizBlanco); // Área grande (3000 - 250)
    escena->addRect(300, 2910, 200, 90, lapizBlanco);  // Área chica (3000 - 90)

    escena->addRect(0, 2800, 800, 200, QPen(Qt::transparent), QBrush(QColor(0, 0, 0, 20)));

    // Variables de control de la partida
    goles = 0;
    vidas = 3;

    // Portería
    porteria = new QGraphicsRectItem(300, 0, 200, 50);
    porteria->setBrush(QBrush(Qt::white));
    porteria->setPen(QPen(Qt::black, 3));
    escena->addItem(porteria);

    // === NUEVO HUD ESTILIZADO DE FORMA PROFESIONAL ===
    textoPuntaje = new QGraphicsTextItem();
    textoPuntaje->setHtml(
        "<div style='background-color: rgba(0, 0, 0, 0.75); color: white; padding: 6px 15px; "
        "border: 2px solid #2ecc71; border-radius: 8px; font-family: \"Impact\", sans-serif; font-size: 18px;'>"
        "⚽ GOLES: <span style='color: #2ecc71;'>0</span> / 3 &nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;"
        "❤️ VIDAS: <span style='color: #e74c3c;'>3</span>"
        "</div>"
        );
    textoPuntaje->setZValue(100); // Mantiene el marcador al frente
    escena->addItem(textoPuntaje);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setFixedSize(800, 600);
    showFullScreen();

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
    listaEnemigos.push_back(new Enemigo(400, 60, 350, 4.5, Enemigo::HORIZONTAL_MAS, true));

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

    centerOn(gidsel);

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
                // Actualización estética al perder una vida
                textoPuntaje->setHtml(
                    "<div style='background-color: rgba(0, 0, 0, 0.75); color: white; padding: 6px 15px; "
                    "border: 2px solid #2ecc71; border-radius: 8px; font-family: \"Impact\", sans-serif; font-size: 18px;'>"
                    "⚽ GOLES: <span style='color: #2ecc71;'>" + QString::number(goles) + "</span> / 3 &nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;"
                                               "❤️ VIDAS: <span style='color: #e74c3c;'>" + QString::number(vidas) + "</span>"
                                               "</div>"
                    );
            } else {
                // Actualización estética en 0 vidas (Borde cambia a rojo)
                textoPuntaje->setHtml(
                    "<div style='background-color: rgba(0, 0, 0, 0.75); color: white; padding: 6px 15px; "
                    "border: 2px solid #e74c3c; border-radius: 8px; font-family: \"Impact\", sans-serif; font-size: 18px;'>"
                    "⚽ GOLES: <span style='color: #2ecc71;'>" + QString::number(goles) + "</span> / 3 &nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;"
                                               "❤️ VIDAS: <span style='color: #e74c3c;'>0</span>"
                                               "</div>"
                    );

                // === GAME OVER GIGANTE Y CENTRADO ===
                QGraphicsTextItem *textoGameOver = new QGraphicsTextItem("¡GAME OVER!");
                QFont fuenteGameOver("Impact", 60, QFont::Bold);
                textoGameOver->setFont(fuenteGameOver);
                textoGameOver->setDefaultTextColor(Qt::red);

                double xCentro = 400 - (textoGameOver->boundingRect().width() / 2);
                double yCentro = gidsel->y() - 150;

                textoGameOver->setPos(xCentro, yCentro);
                textoGameOver->setZValue(100);
                escena->addItem(textoGameOver);

                relojJuego->stop(); // Congela el juego por derrota
                return;
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
                // Actualización estética al ganar el juego (Borde azul de campeonato)
                textoPuntaje->setHtml(
                    "<div style='background-color: rgba(0, 0, 0, 0.85); color: white; padding: 6px 15px; "
                    "border: 2px solid #3498db; border-radius: 8px; font-family: \"Impact\", sans-serif; font-size: 18px;'>"
                    "🏆 ¡VICTORIA! &nbsp;&nbsp;|&nbsp;&nbsp; GOLES: <span style='color: #3498db;'>3 / 3</span>"
                    "</div>"
                    );
                relojJuego->stop();
            } else {
                // Actualización estética al meter un gol normal
                textoPuntaje->setHtml(
                    "<div style='background-color: rgba(0, 0, 0, 0.75); color: white; padding: 6px 15px; "
                    "border: 2px solid #2ecc71; border-radius: 8px; font-family: \"Impact\", sans-serif; font-size: 18px;'>"
                    "⚽ GOLES: <span style='color: #2ecc71;'>" + QString::number(goles) + "</span> / 3 &nbsp;&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;"
                                               "❤️ VIDAS: <span style='color: #e74c3c;'>" + QString::number(vidas) + "</span>"
                                               "</div>"
                    );
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
