#include "nivel1.h"
#include "jugador.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QScreen>          // Necesario para detectar el monitor
#include <QGuiApplication>   // Necesario para detectar el monitor

Nivel1::Nivel1(QWidget *parent) : QGraphicsView(parent) {
    // ==========================================================
    // DETECCIÓN AUTOMÁTICA DE TU MONITOR (PANTALLA COMPLETA)
    // ==========================================================
    QScreen *pantalla = QGuiApplication::primaryScreen();
    QRect medidaPantalla = pantalla->geometry();
    anchoPantalla = medidaPantalla.width();   // Ej: 1920 o 1366
    altoPantalla = medidaPantalla.height();   // Ej: 1080 o 768

    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, anchoPantalla, 3000);
    setScene(escena);


    // Agregamos la textura real de la madera
    QPixmap texturaMadera(":/imagenes/madera.png");
    escena->setBackgroundBrush(QBrush(texturaMadera));
    // Capa 1: Piso de madera
    //escena->setBackgroundBrush(QBrush(QColor(222, 184, 135)));

    // ==========================================================
    // CAPA 2: TRIBUNAS ALINEADAS A LOS EXTREMOS REALES
    // ==========================================================
    QPixmap pixIzquierdo(":/imagenes/tribuna_izq.png");
    QPixmap pixDerecho(":/imagenes/tribuna_der.png");

    int anchoTribuna = 150;
    int altoImagen = 600;

    for (int yActual = 0; yActual < 3000; yActual += altoImagen) {
        // Izquierda pegada a X = 0
        QGraphicsPixmapItem *ti = new QGraphicsPixmapItem(pixIzquierdo);
        ti->setPos(0, yActual);
        ti->setZValue(-10);
        escena->addItem(ti);

        // Derecha pegada al borde total de tu monitor
        QGraphicsPixmapItem *td = new QGraphicsPixmapItem(pixDerecho);
        td->setPos(anchoPantalla - anchoTribuna, yActual);
        td->setZValue(-10);
        escena->addItem(td);
    }

    // ==========================================================
    // CAPA 3: LÍNEAS DE CANCHA ANCHA DEDUCIDAS MATEMÁTICAMENTE
    // ==========================================================
    QPen lapizLineas(Qt::white);
    lapizLineas.setWidth(5);

    inicioXCancha = anchoTribuna; // X = 150
    int areaJuegoAncho = anchoPantalla - (anchoTribuna * 2); // Todo el resto del centro
    int centroX = anchoPantalla / 2; // El centro exacto de tu monitor

    // Líneas de banda
    escena->addLine(inicioXCancha, 0, inicioXCancha, 3000, lapizLineas);
    escena->addLine(inicioXCancha + areaJuegoAncho, 0, inicioXCancha + areaJuegoAncho, 3000, lapizLineas);

    // Línea de medio campo y círculo central adaptados al nuevo centro
    escena->addLine(inicioXCancha, 1500, inicioXCancha + areaJuegoAncho, 1500, lapizLineas);
    escena->addEllipse(centroX - 60, 1500 - 60, 120, 120, lapizLineas);

    // Áreas de portería re-centradas
    QGraphicsEllipseItem *areaArriba = escena->addEllipse(centroX - 150, 0 - 150, 300, 300, lapizLineas);
    areaArriba->setStartAngle(180 * 16);
    areaArriba->setSpanAngle(180 * 16);

    QGraphicsEllipseItem *areaAbajo = escena->addEllipse(centroX - 150, 3000 - 150, 300, 300, lapizLineas);
    areaAbajo->setStartAngle(0 * 16);
    areaAbajo->setSpanAngle(180 * 16);

    // ==========================================================
    // CAPA 4: ELEMENTOS LÓGICOS RE-CENTRADOS
    // ==========================================================
    goles = 0;
    vidas = 3;

    // Portería Rival centrada
    porteria = new QGraphicsRectItem(centroX - 100, 0, 200, 40);
    porteria->setBrush(QBrush(Qt::transparent));
    porteria->setPen(QPen(Qt::white, 4));
    escena->addItem(porteria);

    // Marcador HUD
    textoPuntaje = new QGraphicsTextItem("Goles: 0 / 3   |   Vidas: 3");
    QFont fuente("Impact", 18);
    textoPuntaje->setFont(fuente);
    textoPuntaje->setDefaultTextColor(Qt::yellow);
    textoPuntaje->setZValue(100);
    escena->addItem(textoPuntaje);

    // CONFIGURACIÓN DE VENTANA EN PANTALLA COMPLETA
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(anchoPantalla, altoPantalla); // Se adapta al monitor
    showFullScreen();                          // FUERZA MODO PANTALLA COMPLETA

    // Gidsel aparece en el centro exacto de la nueva cancha ancha
    gidsel = new Jugador();
    gidsel->setPos(centroX - 25, 2850);
    escena->addItem(gidsel);
    centerOn(gidsel);

    balon = nullptr;
    jeringa = nullptr;
    arquero = nullptr;

    relojJuego = new QTimer(this);
    connect(relojJuego, SIGNAL(timeout()), this, SLOT(actualizarJuego()));
    relojJuego->start(20);
}

Nivel1::~Nivel1() {
    delete escena;
}

void Nivel1::actualizarJuego() {
    centerOn(gidsel);

    // El HUD se ancla dinámicamente un poco a la izquierda del centro de la pantalla visible
    textoPuntaje->setPos(mapToScene(anchoPantalla / 2 - 170, 20));
    textoPuntaje->setPlainText(QString("Goles: %1 / 3   |   Vidas: %2").arg(goles).arg(vidas));
}

void Nivel1::disminuirVida() {
    vidas--;
    if (vidas <= 0) {
        relojJuego->stop();
        QMessageBox::critical(this, "Game Over", "Te has quedado sin vidas.");
    } else {
        gidsel->setPos((anchoPantalla / 2) - 25, 2850);
    }
}

void Nivel1::keyPressEvent(QKeyEvent *event) {
    gidsel->keyPressEvent(event);
}
