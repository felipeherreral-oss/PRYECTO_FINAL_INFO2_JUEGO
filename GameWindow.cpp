#include "GameWindow.h"
#include "GameExceptions.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QFont>
#include <QTimer>
#include <QRandomGenerator>

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Estadio Intergalactico - Balonmano Nivel 2");
    resize(1200, 750);

    view_ = new QGraphicsView(this);
    view_->setRenderHints(QPainter::Antialiasing |
                          QPainter::SmoothPixmapTransform |
                          QPainter::TextAntialiasing);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setFrameStyle(QFrame::NoFrame);
    view_->setBackgroundBrush(QBrush(QColor(5, 5, 20)));
    view_->setAlignment(Qt::AlignCenter);
    setCentralWidget(view_);

    // Centrar en pantalla
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        move((geo.width() - 1200) / 2, (geo.height() - 750) / 2);
    }

    // Mostrar pantalla de inicio DESPUES de que la ventana este visible
    // usando QTimer::singleShot para que el event loop ya haya procesado
    // el resize y fitInView tenga dimensiones reales
    QTimer::singleShot(50, this, &GameWindow::showStartScreen);
}

// ─────────────────────────────────────────────────────────────────────────────
// RESIZE — reajustar la vista cuando cambia el tamaño de la ventana
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (view_ && view_->scene()) {
        view_->fitInView(view_->scene()->sceneRect(), Qt::KeepAspectRatio);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PANTALLA DE INICIO
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::showStartScreen() {
    auto* startScene = new QGraphicsScene(0, 0, 1200, 750, this);

    // Fondo oscuro
    startScene->addRect(0, 0, 1200, 750, Qt::NoPen, QBrush(QColor(5, 5, 30)));

    // Estrellas
    for (int i = 0; i < 80; ++i) {
        float x = float(QRandomGenerator::global()->bounded(1200));
        float y = float(QRandomGenerator::global()->bounded(750));
        float r = 0.5f + float(QRandomGenerator::global()->bounded(3)) * 0.5f;
        startScene->addEllipse(x, y, r * 2, r * 2, Qt::NoPen,
            QBrush(QColor(255, 255, 255,
                          100 + QRandomGenerator::global()->bounded(155))));
    }

    // Titulo
    auto addText = [&](const QString& txt, float x, float y,
                       int sz, QColor col, bool bold = true) {
        auto* t = startScene->addText(txt,
            QFont("Arial", sz, bold ? QFont::Bold : QFont::Normal));
        t->setDefaultTextColor(col);
        t->setPos(x - t->boundingRect().width() / 2.f, y);
        return t;
    };

    addText("ESTADIO INTERGALACTICO", 600, 60,  28, QColor(80, 200, 255));
    addText("Balonmano: Humanos vs Looney Tunes", 600, 115, 16,
            QColor(200, 255, 200), false);

    // Panel controles
    startScene->addRect(250, 200, 700, 310,
        QPen(QColor(100, 150, 255, 120), 2),
        QBrush(QColor(10, 20, 60, 210)));

    addText("-- CONTROLES --",          600, 215, 13, QColor(255, 220, 80));
    addText("Jugador 1: [W/A/S/D]  Tiro: [F]  Pase: [G]",
                                        600, 248, 11, QColor(220, 240, 255), false);
    addText("Jugador 2: [Flechas]   Tiro: [K]  Pase: [L]",
                                        600, 272, 11, QColor(220, 240, 255), false);
    addText("[TAB] Cambiar jugador activo (circulo amarillo)",
                                        600, 296, 11, QColor(220, 240, 255), false);
    addText("-- OBJETIVO --",           600, 322, 13, QColor(255, 220, 80));
    addText("Anota mas goles que los Looney Tunes en 5 minutos",
                                        600, 350, 11, QColor(220, 240, 255), false);
    addText("La IA rival aprende de tus jugadas y se vuelve mas dificil!",
                                        600, 372, 11, QColor(255, 150, 150), false);
    addText("3 vs 3  |  Arqueros controlados por IA  |  Vista cenital",
                                        600, 394, 10, QColor(180, 255, 180), false);
    addText("Equipos: lado IZQUIERDO = Humanos  |  lado DERECHO = Looney Tunes",
                                        600, 416, 10, QColor(180, 255, 180), false);

    // -- Selector de dificultad (teclas 1 / 2 / 3) --
    const char* names[3] = { "1) FACIL", "2) NORMAL", "3) DIFICIL" };
    float bx[3] = { 320.f, 520.f, 720.f };
    for (int i = 0; i < 3; ++i) {
        bool sel = (selectedDifficulty_ == i);
        startScene->addRect(bx[i], 452, 160, 34,
            QPen(sel ? QColor(255, 220, 80, 230) : QColor(100, 150, 255, 110),
                 sel ? 3 : 1),
            QBrush(sel ? QColor(60, 60, 15, 230) : QColor(10, 20, 60, 180)));
        addText(names[i], bx[i] + 80.f, 458, 11,
                sel ? QColor(255, 230, 120) : QColor(200, 220, 255), sel);
    }
    addText("Elige dificultad con [1] [2] [3]", 600, 498, 10,
            QColor(255, 220, 80), false);

    // Boton iniciar
    startScene->addRect(400, 540, 400, 60,
        QPen(QColor(100, 255, 100, 200), 2),
        QBrush(QColor(15, 70, 15, 230)));
    addText("PRESIONA ENTER O ESPACIO PARA INICIAR",
            600, 558, 11, QColor(100, 255, 100));

    view_->setScene(startScene);
    view_->fitInView(startScene->sceneRect(), Qt::KeepAspectRatio);
    view_->setFocus();

    inStartScreen_ = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// INICIAR JUEGO
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::startGame() {
    if (!inStartScreen_) return;
    inStartScreen_ = false;

    // Borrar escena anterior
    QGraphicsScene* old = view_->scene();
    view_->setScene(nullptr);
    if (old && old != scene_) delete old;

    try {
        scene_ = new Level2Scene(this, selectedDifficulty_);
        view_->setScene(scene_);
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        view_->setFocus();

        connect(scene_, &Level2Scene::levelCompleted,
                this,   &GameWindow::onLevelCompleted);

    } catch (const GameException& e) {
        QMessageBox::critical(this, "Error al iniciar",
            QString("Error al cargar el nivel:\n%1\n\n"
                    "Verifica que los archivos de imagen esten en:\n"
                    "resources/images/").arg(e.what()));
        inStartScreen_ = true;
        showStartScreen();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PANTALLA DE FIN
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::showEndScreen(bool humanWon, int hGoals, int eGoals) {
    auto* endScene = new QGraphicsScene(0, 0, 1200, 750, this);
    endScene->addRect(0, 0, 1200, 750, Qt::NoPen,
        QBrush(humanWon ? QColor(5, 30, 5) : QColor(30, 5, 5)));

    QString resultText = humanWon ? "¡VICTORIA!" :
                         (hGoals == eGoals ? "¡EMPATE!" : "¡DERROTA!");
    QColor  resultCol  = humanWon ? QColor(255, 220, 0) :
                         (hGoals == eGoals ? Qt::cyan : QColor(255, 80, 80));

    auto addText = [&](const QString& txt, float x, float y,
                       int sz, QColor col, bool bold = true) {
        auto* t = endScene->addText(txt,
            QFont("Arial", sz, bold ? QFont::Bold : QFont::Normal));
        t->setDefaultTextColor(col);
        t->setPos(x - t->boundingRect().width() / 2.f, y);
        return t;
    };

    addText("FIN DEL PARTIDO", 600, 70, 18, QColor(200, 220, 255));
    addText(resultText, 600, 130, 42, resultCol);
    addText(QString("HUMANOS  %1  -  %2  LOONEY TUNES")
            .arg(hGoals).arg(eGoals), 600, 235, 22, Qt::white);

    QString msg = humanWon
        ? "Gidsel y compania demostraron que los humanos\npueden superar a los Looney Tunes!"
        : (hGoals == eGoals
            ? "Un duelo parejo en el Estadio Intergalactico.\n¿Quieres la revancha?"
            : "Los Looney Tunes aprendieron de tus jugadas\ny te derrotaron con fisica caricaturesca!");
    addText(msg, 600, 315, 13, QColor(200, 230, 255), false);

    addText("¿Que quieres hacer?", 600, 430, 14, QColor(255, 220, 80));

    // Botones de decision: seguir jugando o salir
    endScene->addRect(300, 480, 270, 60,
        QPen(QColor(100, 255, 100, 220), 2), QBrush(QColor(15, 70, 15, 230)));
    addText("SEGUIR JUGANDO", 435, 492, 14, QColor(120, 255, 120));
    addText("[ENTER] o [R]", 435, 516, 10, QColor(180, 255, 180), false);

    endScene->addRect(630, 480, 270, 60,
        QPen(QColor(255, 100, 100, 220), 2), QBrush(QColor(70, 15, 15, 230)));
    addText("SALIR DEL JUEGO", 765, 492, 14, QColor(255, 120, 120));
    addText("[ESC] o [Q]", 765, 516, 10, QColor(255, 180, 180), false);

    QGraphicsScene* old = view_->scene();
    view_->setScene(endScene);
    if (old && old != scene_) delete old;
    if (scene_) { delete scene_; scene_ = nullptr; }

    view_->fitInView(endScene->sceneRect(), Qt::KeepAspectRatio);
    view_->setFocus();
    inStartScreen_ = false;
    inEndScreen_   = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// SLOTS Y EVENTOS
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::onLevelCompleted(bool humanWon) {
    int h = 0, e = 0;
    if (scene_) {
        h = scene_->getGameManager()->getHumanGoals();
        e = scene_->getGameManager()->getEnemyGoals();
    }
    // Diferir: no destruir la escena mientras su propio slot sigue en la pila
    QTimer::singleShot(0, this, [this, humanWon, h, e]() {
        showEndScreen(humanWon, h, e);
    });
}

void GameWindow::keyPressEvent(QKeyEvent* ev) {
    int key = ev->key();

    // Pantalla de inicio
    if (inStartScreen_) {
        if (key == Qt::Key_1) { selectedDifficulty_ = 0; showStartScreen(); return; }
        if (key == Qt::Key_2) { selectedDifficulty_ = 1; showStartScreen(); return; }
        if (key == Qt::Key_3) { selectedDifficulty_ = 2; showStartScreen(); return; }
        if (key == Qt::Key_Return || key == Qt::Key_Enter ||
            key == Qt::Key_Space) {
            startGame();
        }
        return;
    }

    // Pantalla de fin: decidir si seguir jugando o salir
    if (inEndScreen_) {
        if (key == Qt::Key_R || key == Qt::Key_Return ||
            key == Qt::Key_Enter || key == Qt::Key_Space) {
            inEndScreen_   = false;
            inStartScreen_ = true;
            showStartScreen();          // volver al menú para re-elegir dificultad y jugar
        } else if (key == Qt::Key_Escape || key == Qt::Key_Q) {
            close();                     // salir del juego
        }
        return;
    }

    // En juego
    if (scene_) {
        if (key == Qt::Key_Escape) {
            // Pausa rapida: volver al inicio
            inEndScreen_   = false;
            inStartScreen_ = true;
            if (scene_) { delete scene_; scene_ = nullptr; }
            showStartScreen();
            return;
        }
        scene_->keyPressEvent(ev);
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent* ev) {
    if (scene_ && !inStartScreen_ && !inEndScreen_)
        scene_->keyReleaseEvent(ev);
}

void GameWindow::closeEvent(QCloseEvent* ev) {
    ev->accept();
}
