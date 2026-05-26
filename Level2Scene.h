#pragma once
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QTimer>
#include <memory>
#include <vector>

#include "GameManager.h"
#include "Ball.h"
#include "HumanPlayer.h"
#include "GoalkeeperAI.h"
#include "EnemyPlayer.h"
#include "HUD.h"
#include "PhysicsEngine.h"
#include "GameExceptions.h"

/**
 * @brief Escena principal del Nivel 2: Partido 3 vs 3 en el Estadio Intergaláctico.
 *
 *  Contiene:
 *  - Equipo humano: 2 jugadores de campo + 1 arquero (IA)
 *  - Equipo rival (Looney Tunes): 2 defensas de campo + 1 arquero (IA)
 *  - 1 balón
 *  - HUD y GameManager
 *
 *  STL containers:
 *  - std::vector<EnemyPlayer*>  para defensas rivales
 *  - std::vector<HumanPlayer*>  para jugadores humanos
 *
 *  Detecta colisiones, coordina la IA, y administra el flujo del partido.
 */
class Level2Scene : public QGraphicsScene {
    Q_OBJECT

public:
    // Dimensiones del campo (vista cenital)
    static constexpr float FIELD_W = 1100.f;
    static constexpr float FIELD_H = 650.f;
    static constexpr float FIELD_MARGIN = 50.f;

    // Arcos
    static constexpr float GOAL_WIDTH  = 100.f;
    static constexpr float GOAL_DEPTH  = 30.f;
    static constexpr float HUMAN_GOAL_X  = FIELD_MARGIN;              // Arco humano (izquierda)
    static constexpr float ENEMY_GOAL_X  = FIELD_MARGIN + FIELD_W;   // Arco rival (derecha)
    static constexpr float GOAL_Y_CENTER = FIELD_MARGIN + FIELD_H * 0.5f;

    explicit Level2Scene(QObject* parent = nullptr);
    ~Level2Scene() override = default;

    // ── Input de teclado ──────────────────────────────────────────────────────
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    // ── Acceso externo ────────────────────────────────────────────────────────
    GameManager* getGameManager() { return manager_.get(); }

signals:
    void levelCompleted(bool humanWon);

private slots:
    void gameLoop();
    void onGoalByHuman();
    void onGoalByEnemy();
    void onGameOver(int h, int e);
    void onKickoff();

private:
    // ── Objetos del juego ─────────────────────────────────────────────────────
    std::unique_ptr<GameManager>   manager_;
    std::unique_ptr<Ball>          ball_;
    std::vector<HumanPlayer*>      humanPlayers_;     // 2 jugadores humanos (raw ptr, owned by scene)
    GoalkeeperAI*                  humanGoalkeeper_;  // Arquero humano (raw ptr, owned by scene)
    std::vector<EnemyPlayer*>      enemyPlayers_;     // 2 defensas rivales
    GoalkeeperAI*                  enemyGoalkeeper_;  // Arquero rival
    HUD*                           hud_;

    // ── Timers ────────────────────────────────────────────────────────────────
    QTimer* loopTimer_;
    qint64  lastTime_      = 0;
    float   gameTimeAccum_ = 0.f;

    // ── Estado ────────────────────────────────────────────────────────────────
    int   activePlayerIdx_ = 0;    // Índice en humanPlayers_ del jugador controlado
    bool  enemyHasBall_    = false;
    float enemyBallTimer_  = 0.f;  // Timer para que la IA decida cuándo lanzar

    // ── Inicialización ────────────────────────────────────────────────────────
    void setupField();
    void setupPlayers();
    void setupBall();
    void setupHUD();
    void connectSignals();

    // ── Lógica del juego ──────────────────────────────────────────────────────
    void updateGame(float dt);
    void updatePhysics(float dt);
    void checkCollisions();
    void checkGoals();
    void checkBallPickup();
    void updateAI(float dt);
    void handleEnemyBallPossession(float dt);
    void switchActivePlayer();
    void doKickoff(bool humanKickoff);
    void resetPositions();

    // ── Colisión elástica ─────────────────────────────────────────────────────
    void resolveElasticCollision(Collidable* a, Collidable* b);

    // ── Dibujo del campo ─────────────────────────────────────────────────────
    void drawBackground();

    // ── Utilidades ────────────────────────────────────────────────────────────
    HumanPlayer* getActivePlayer();
    HumanPlayer* getInactivePlayer();
    bool ballInHumanGoal() const;
    bool ballInEnemyGoal() const;

    static constexpr int LOOP_INTERVAL_MS = 16;  // ~60 FPS
};
