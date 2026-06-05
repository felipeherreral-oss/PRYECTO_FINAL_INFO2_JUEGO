#include "menuinicio.h"
#include "nivel1.h"
#include <QGraphicsTextItem>
#include <QBrush>
#include <QColor>

MenuInicio::MenuInicio(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 600);
    setScene(escena);

    // Fondo oscuro estilo espacio profundo
    escena->setBackgroundBrush(QBrush(QColor(10, 10, 25)));

    QGraphicsTextItem *textoMenu = new QGraphicsTextItem();

    // Diseño estructurado con HTML y CSS integrado
    QString htmlContent =
        "<div style='text-align: center; width: 760px; font-family: \"Arial\", sans-serif;'>"
        "  "
        "  <h1 style='color: #4ad2ff; font-size: 42px; margin-bottom: 5px; font-family: \"Impact\"; letter-spacing: 3px;'>ESQUIVAR LOONEY TUNES</h1>"
        "  <h3 style='color: #ffffff; font-size: 18px; margin-top: 0; margin-bottom: 35px;'>Balonmano: Humanos vs Animados</h3>"

        "  "
        "  <div style='border: 2px solid #1a365d; background-color: rgba(15, 32, 67, 0.85); padding: 25px; border-radius: 10px; display: inline-block; width: 580px; text-align: center;'>"
        "    "
        "    <p style='color: #ffcc00; font-weight: bold; font-size: 18px; margin: 0 0 10px 0; letter-spacing: 1px;'>-- CONTROLES --</p>"
        "    <p style='color: #ffffff; font-size: 16px; margin: 5px 0;'><b>Mover a Gidsel:</b> [W / A / S / D] o [Flechas del Teclado]</p>"
        "    <p style='color: #ffffff; font-size: 16px; margin: 5px 0;'><b>Disparar Balón:</b> [Espacio] o [F]</p>"
        "    "
        "    <br>"
        "    "
        "    <p style='color: #ffcc00; font-weight: bold; font-size: 18px; margin: 15px 0 10px 0; letter-spacing: 1px;'>-- OBJETIVO DEL NIVEL --</p>"
        "    <p style='color: #ffffff; font-size: 15px; margin: 5px 0; line-height: 1.5;'>"
        "       Avanza de forma vertical por la cancha de madera esquivando los movimientos locos de los defensores rivales.<br>"
        "       Llega hasta el extremo superior y <b>anota 3 goles</b> para consagrarte campeón.<br>"
        "       <span style='color: #ff4d4d; font-weight: bold;'>⚠️ ¡Cuidado con el arquero Tazmania! Se mueve en la portería a máxima velocidad.</span>"
        "    </p>"
        "  </div>"

        "  <br><br><br><br>"

        "  "
        "  <div style='border: 2px solid #2ecc71; background-color: rgba(0, 0, 0, 0.5); padding: 12px 25px; display: inline-block; border-radius: 6px;'>"
        "    <p style='color: #2ecc71; font-weight: bold; font-size: 20px; margin: 0; font-family: \"Impact\"; letter-spacing: 1px;'>PRESIONA ENTER O ESPACIO PARA INICIAR</p>"
        "  </div>"
        "</div>";

    textoMenu->setHtml(htmlContent);

    // Centramos perfectamente el bloque de texto dentro de la escena de 800x600
    double xPos = 400 - (textoMenu->boundingRect().width() / 2);
    double yPos = 300 - (textoMenu->boundingRect().height() / 2);
    textoMenu->setPos(xPos, yPos);
    escena->addItem(textoMenu);

    // Configuraciones de visualización iguales a tu Nivel1
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    showFullScreen();
}

void MenuInicio::keyPressEvent(QKeyEvent *event) {
    // Si presiona Enter, Return o Espacio, arranca el juego real
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
        Nivel1 *juego = new Nivel1();
        juego->show(); // Muestra el nivel del juego
        this->close(); // Cierra esta pantalla de menú para liberar memoria
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

MenuInicio::~MenuInicio() {
    delete escena;
}
