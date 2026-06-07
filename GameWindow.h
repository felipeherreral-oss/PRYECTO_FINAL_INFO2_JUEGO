#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QPainter>
#include <memory>
#include "Level2Scene.h"

class GameWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow() override = default;

signals:
    void pedirMenuPrincipal();   // "seguir jugando" tras el partido → menú principal
    void pedirSalir();           // "salir del juego"

protected:
    void keyPressEvent(QKeyEvent* e)   override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void closeEvent(QCloseEvent* e)    override;
    void resizeEvent(QResizeEvent* e)  override;   // ← reajusta fitInView

private slots:
    void onLevelCompleted(bool humanWon);
    void startGame();
    void showStartScreen();

private:
    QGraphicsView* view_    = nullptr;
    Level2Scene*   scene_   = nullptr;
    bool inStartScreen_     = false;
    bool inEndScreen_       = false;
    int  selectedDifficulty_ = 1;   // 0 = Fácil, 1 = Normal, 2 = Difícil

    void showEndScreen(bool humanWon, int hGoals, int eGoals);
};
