#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QPainter>
#include <memory>
#include "Level2Scene.h"

/**
 * @brief Ventana principal del juego.
 *
 *  Contiene el QGraphicsView que renderiza la Level2Scene.
 *  Maneja el redimensionamiento y la presentación de la pantalla de inicio/fin.
 */
class GameWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

private slots:
    void onLevelCompleted(bool humanWon);
    void startGame();

private:
    QGraphicsView* view_;
    Level2Scene*   scene_ = nullptr;

    void setupView();
    void showStartScreen();
    void showEndScreen(bool humanWon, int hGoals, int eGoals);
};
