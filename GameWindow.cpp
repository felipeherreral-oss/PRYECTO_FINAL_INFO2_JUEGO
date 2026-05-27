#include "GameWindow.h"
#include "GameExceptions.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QRandomGenerator>

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("🏟 Estadio Intergaláctico — Balonmano Nivel 2");
    setMinimumSize(1200, 750);

    // Vista de gráficos
    view_ = new QGraphicsView(this);
    view_->setRenderHints(QPainter::Antialiasing |
                          QPainter::SmoothPixmapTransform |
                          QPainter::TextAntialiasing);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setFrameStyle(QFrame::NoFrame);
    view_->setBackgroundBrush(QBrush(QColor(5, 5, 20)));
    setCentralWidget(view_);

    // Centrar ventana en pantalla
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        int x = (geo.width()  - width())  / 2;
        int y = (geo.height() - height()) / 2;
        move(x, y);
    }

    showStartScreen();
}

// ─────────────────────────────────────────────────────────────────────────────
// PANTALLA DE INICIO
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::showStartScreen() {
    auto* startScene = new QGraphicsScene(0, 0, 1200, 750, this);

    // Fondo
    startScene->addRect(0, 0, 1200, 750, Qt::NoPen,
                        QBrush(QColor(5, 5, 30)));

    // Estrellas decorativas
    for (int i = 0; i < 80; ++i) {
        float x = float(QRandomGenerator::global()->bounded(1200));
        float y = float(QRandomGenerator::global()->bounded(750));
        float r = 0.5f + float(QRandomGenerator::global()->bounded(3)) * 0.5f;
        startScene->addEllipse(x, y, r*2, r*2, Qt::NoPen,
                               QBrush(QColor(255,255,255, 100 + QRandomGenerator::global()->bounded(155))));
    }

    // Título principal
    auto* title = startScene->addText(
        "⚡ ESTADIO INTERGALÁCTICO ⚡",
        QFont("Arial", 30, QFont::Bold));
    title->setDefaultTextColor(QColor(80, 200, 255));
    title->setPos(600 - title->boundingRect().width() / 2, 80);

    auto* subtitle = startScene->addText(
        "Balonmano: Humanos vs Looney Tunes",
        QFont("Arial", 18, QFont::Normal));
    subtitle->setDefaultTextColor(QColor(200, 255, 200));
    subtitle->setPos(600 - subtitle->boundingRect().width() / 2, 150);

    // Panel de controles
    auto* ctrlBg = startScene->addRect(300, 220, 600, 280,
        QPen(QColor(100, 150, 255, 120), 2),
        QBrush(QColor(10, 20, 60, 200)));
    ctrlBg->setZValue(1);

    auto addInfoText = [&](const QString& txt, float y, QColor col = QColor(220,240,255),
                           int fontSize = 12) {
        auto* t = startScene->addText(txt, QFont("Arial", fontSize));
        t->setDefaultTextColor(col);
        t->setPos(600 - t->boundingRect().width() / 2, y);
        t->setZValue(2);
    };

    addInfoText("── CONTROLES ──", 235, QColor(255, 220, 80), 14);
    addInfoText("Jugador 1: [W/A/S/D] Mover   [F] Tiro   [G] Pase", 270);
    addInfoText("Jugador 2: [↑/←/↓/→] Mover   [K] Tiro   [L] Pase", 298);
    addInfoText("[TAB] Cambiar jugador activo (⬤ círculo amarillo)", 326);
    addInfoText("── OBJETIVO ──", 365, QColor(255, 220, 80), 14);
    addInfoText("Anota más goles que los Looney Tunes en 5 minutos", 395);
    addInfoText("¡La IA rival aprende de tus jugadas y se vuelve más difícil!", 423,
                QColor(255, 150, 150));
    addInfoText("3 vs 3 · Arqueros controlados por IA · Vista cenital", 451,
                QColor(180, 255, 180));

    // Botón de inicio (simulado con texto interactivo)
    auto* startBtn = startScene->addRect(450, 530, 300, 60,
        QPen(QColor(100, 255, 100, 200), 2),
        QBrush(QColor(20, 80, 20, 220)));
    startBtn->setZValue(2);

    auto* startTxt = startScene->addText("▶  INICIAR PARTIDO",
                                          QFont("Arial", 16, QFont::Bold));
    startTxt->setDefaultTextColor(QColor(100, 255, 100));
    startTxt->setPos(600 - startTxt->boundingRect().width() / 2, 545);
    startTxt->setZValue(3);

    auto* pressEnter = startScene->addText("[ENTER] o click para comenzar",
                                            QFont("Arial", 10));
    pressEnter->setDefaultTextColor(QColor(150, 200, 150, 180));
    pressEnter->setPos(600 - pressEnter->boundingRect().width() / 2, 620);
    pressEnter->setZValue(2);

    view_->setScene(startScene);
    view_->fitInView(startScene->sceneRect(), Qt::KeepAspectRatio);

    // Conectar click en el botón de inicio
    view_->setInteractive(true);
    connect(view_, &QGraphicsView::rubberBandChanged,
            this, [](QRect, QPointF, QPointF){});

    // Usar un QTimer para escuchar Enter o click
    // El evento se maneja en keyPressEvent
}

// ─────────────────────────────────────────────────────────────────────────────
// INICIAR JUEGO
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::startGame() {
    try {
        scene_ = new Level2Scene(this);
        view_->setScene(scene_);
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        view_->setFocus();

        connect(scene_, &Level2Scene::levelCompleted,
                this,   &GameWindow::onLevelCompleted);

    } catch (const GameException& e) {
        QMessageBox::critical(this, "Error al iniciar",
                              QString("Error: %1").arg(e.what()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PANTALLA DE FIN
// ─────────────────────────────────────────────────────────────────────────────
void GameWindow::showEndScreen(bool humanWon, int hGoals, int eGoals) {
    auto* endScene = new QGraphicsScene(0, 0, 1200, 750, this);
    endScene->addRect(0, 0, 1200, 750, Qt::NoPen,
                      QBrush(humanWon ? QColor(5, 30, 5) : QColor(30, 5, 5)));

    QString resultText = humanWon ? "🏆 ¡VICTORIA!" : (hGoals == eGoals ? "🤝 ¡EMPATE!" : "💀 DERROTA");
    QColor  resultCol  = humanWon ? QColor(255, 220, 0) : (hGoals == eGoals ? Qt::cyan : QColor(255,80,80));

    auto* result = endScene->addText(resultText, QFont("Arial", 40, QFont::Bold));
    result->setDefaultTextColor(resultCol);
    result->setPos(600 - result->boundingRect().width() / 2, 150);

    auto* score = endScene->addText(
        QString("HUMANOS  %1  —  %2  LOONEY TUNES").arg(hGoals).arg(eGoals),
        QFont("Arial", 22, QFont::Bold));
    score->setDefaultTextColor(Qt::white);
    score->setPos(600 - score->boundingRect().width() / 2, 250);

    auto* msg = endScene->addText(
        humanWon ? "¡Gidsel y compañía demostraron que los humanos\npueden superar las leyes de la física caricaturesca!"
                 : "¡Los Looney Tunes aprendieron de tus jugadas\ny te derrotaron con su física caricaturesca!",
        QFont("Arial", 14));
    msg->setDefaultTextColor(QColor(200, 230, 255));
    msg->setPos(600 - msg->boundingRect().width() / 2, 330);

    // Botón reiniciar
    auto* restartBtn = endScene->addRect(400, 500, 200, 55,
        QPen(QColor(100, 255, 100, 200), 2), QBrush(QColor(20, 80, 20, 200)));
    auto* restartTxt = endScene->addText("↺ REINICIAR", QFont("Arial", 14, QFont::Bold));
    restartTxt->setDefaultTextColor(QColor(100, 255, 100));
    restartTxt->setPos(500 - restartTxt->boundingRect().width() / 2, 515);
    (void)restartBtn;

    // Botón salir
    auto* exitBtn = endScene->addRect(620, 500, 180, 55,
        QPen(QColor(255, 100, 100, 200), 2), QBrush(QColor(80, 20, 20, 200)));
    auto* exitTxt = endScene->addText("✖ SALIR", QFont("Arial", 14, QFont::Bold));
    exitTxt->setDefaultTextColor(QColor(255, 100, 100));
    exitTxt->setPos(710 - exitTxt->boundingRect().width() / 2, 515);
    (void)exitBtn;

    auto* hint = endScene->addText("[R] Reiniciar   [ESC] Salir", QFont("Arial", 10));
    hint->setDefaultTextColor(QColor(150, 150, 150));
    hint->setPos(600 - hint->boundingRect().width() / 2, 600);

    view_->setScene(endScene);
    view_->fitInView(endScene->sceneRect(), Qt::KeepAspectRatio);
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
    showEndScreen(humanWon, h, e);
}

void GameWindow::keyPressEvent(QKeyEvent* ev) {
    // En pantalla de inicio o fin: Enter = acción
    if (!scene_ || !view_->scene()) {
        if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter ||
            ev->key() == Qt::Key_Space) {
            startGame();
            return;
        }
    }

    // En pantalla de fin: R = reiniciar, ESC = salir
    if (scene_ && !scene_->getGameManager()->isPlaying()) {
        if (ev->key() == Qt::Key_R) {
            delete scene_;
            scene_ = nullptr;
            startGame();
            return;
        }
        if (ev->key() == Qt::Key_Escape) {
            close();
            return;
        }
    }

    // Durante el juego: pasar eventos a la escena
    if (scene_) {
        scene_->keyPressEvent(ev);
    }

    QMainWindow::keyPressEvent(ev);
}

void GameWindow::keyReleaseEvent(QKeyEvent* ev) {
    if (scene_) scene_->keyReleaseEvent(ev);
    QMainWindow::keyReleaseEvent(ev);
}

void GameWindow::closeEvent(QCloseEvent* ev) {
    ev->accept();
}

void GameWindow::setupView() {
    view_->setFocus();
}
