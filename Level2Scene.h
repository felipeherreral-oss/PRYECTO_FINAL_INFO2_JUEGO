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
 *  - Equipo rival (Looney Tunes): 2 jugadores de campo + 1 arquero (IA)
 *  - 1 balón
 *  - HUD y GameManager
 *
 *  STL containers:
 *  - std::vector<EnemyPlayer*>  para jugadores rivales
 *  - std::vector<HumanPlayer*>  para jugadores humanos
 *
 *  La geometría del campo (PLAY_*) está medida directamente sobre la imagen
 *  de cancha (zona amarilla jugable). Los arcos quedan exactamente sobre las
 *  líneas de gol visibles y los arqueros dentro del campo.
 */
class Level2Scene : public QGraphicsScene {
    Q_OBJECT

public:
    // ── Tamaño total de la escena (la imagen de cancha se estira a este tamaño) ──
    static constexpr float SCENE_W = 1200.f;
    static constexpr float SCENE_H = 750.f;

    // ── Zona amarilla jugable (coordenadas de escena, medidas sobre la imagen) ──
    static constexpr float PLAY_LEFT   = 245.f;   // línea de gol izquierda (arco humano)
    static constexpr float PLAY_RIGHT  = 958.f;   // línea de gol derecha   (arco rival)
    static constexpr float PLAY_TOP    = 152.f;
    static constexpr float PLAY_BOTTOM = 586.f;
    static constexpr float PLAY_W = PLAY_RIGHT - PLAY_LEFT;
    static constexpr float PLAY_H = PLAY_BOTTOM - PLAY_TOP;
    static constexpr float PLAY_CX = (PLAY_LEFT + PLAY_RIGHT) * 0.5f;
    static constexpr float PLAY_CY = (PLAY_TOP + PLAY_BOTTOM) * 0.5f;

    // ── Arcos (la boca del arco es vertical en esta vista cenital) ──────────────
    static constexpr float GOAL_HALF     = 92.f;            // media altura de la boca
    static constexpr float GOAL_Y_CENTER = PLAY_CY;
    static constexpr float GOAL_TOP      = GOAL_Y_CENTER - GOAL_HALF;
    static constexpr float GOAL_BOT      = GOAL_Y_CENTER + GOAL_HALF;

    static constexpr float HUMAN_GOAL_X  = PLAY_LEFT;       // arco humano (izquierda)
    static constexpr float ENEMY_GOAL_X  = PLAY_RIGHT;      // arco rival  (derecha)
    static constexpr float GK_INSET      = 26.f;            // qué tan adentro está el arquero

    // ── Área de arco (zona AZUL, semicírculo) ──────────────────────────────────
    // Ningún jugador de campo (humano o Looney) puede entrar; sólo los arqueros.
    // Radio medido sobre la imagen de cancha respecto al punto de gol.
    static constexpr float AREA_RADIUS = 134.f;

    explicit Level2Scene(QObject* parent = nullptr, int difficultyPreset = 1);
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
    std::vector<EnemyPlayer*>      enemyPlayers_;     // 2 jugadores rivales
    GoalkeeperAI*                  enemyGoalkeeper_;  // Arquero rival
    HUD*                           hud_;

    // ── Timers ────────────────────────────────────────────────────────────────
    QTimer* loopTimer_;
    qint64  lastTime_      = 0;
    float   gameTimeAccum_ = 0.f;

    // ── Estado ────────────────────────────────────────────────────────────────
    int   activePlayerIdx_  = 0;    // Índice en humanPlayers_ del jugador controlado
    int   difficultyPreset_ = 1;    // 0 = Fácil, 1 = Normal, 2 = Difícil
    float enemyAggro_       = 1.f;  // Multiplicador de velocidad ofensiva rival
    bool  ballShotByHuman_  = false;// Quién realizó el último tiro (para bloqueos)

    // Coordinación ofensiva rival
    float enemyDecisionTimer_ = 1.2f;

    // ── Inicialización ────────────────────────────────────────────────────────
    void setupField();
    void setupPlayers();
    void setupBall();
    void setupHUD();
    void connectSignals();
    void applyDifficultyPreset();

    // ── Lógica del juego ──────────────────────────────────────────────────────
    void updateGame(float dt);
    void checkCollisions();
    void checkGoals();
    void checkBallPickup();
    void checkOutOfBounds();
    void enforceGoalAreas();   // empuja a los jugadores de campo fuera de la zona azul
    void updateAI(float dt);
    void runEnemyOffense(EnemyPlayer* holder, float dt);
    void runEnemyLooseChase(float dt);
    void updateHumanSupport();
    void switchActivePlayer();
    void doKickoff(bool humanKickoff);
    void resetPositions();

    // ── Colisión elástica ─────────────────────────────────────────────────────
    void resolveElasticCollision(Collidable* a, Collidable* b);

    // ── Dibujo del campo ─────────────────────────────────────────────────────
    void drawBackground();

    // ── Utilidades ────────────────────────────────────────────────────────────
    HumanPlayer*  getActivePlayer();
    HumanPlayer*  getInactivePlayer();
    HumanPlayer*  humanHolder();
    EnemyPlayer*  enemyHolder();
    EnemyPlayer*  nearestEnemyTo(Vec2D p);
    bool          ballInsideGoalMouth() const;

    static constexpr int LOOP_INTERVAL_MS = 16;  // ~60 FPS
};
