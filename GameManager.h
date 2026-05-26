#pragma once
#include <QObject>
#include <QString>
#include <memory>

/**
 * @brief Gestor de estado global del juego.
 *
 *  Controla: marcador, temporizador de partido, estado de juego,
 *  y notifica a Level2Scene de los eventos importantes.
 *
 *  Usa señales/slots de Qt para desacoplar la lógica de la vista.
 */
class GameManager : public QObject {
    Q_OBJECT

public:
    enum class GameState {
        KICKOFF,     // Inicio o tras un gol
        PLAYING,     // En curso
        GOAL_SCORED, // Animación de gol
        HALFTIME,    // Descanso (si aplica)
        GAME_OVER    // Fin del partido
    };

    explicit GameManager(int matchDurationSeconds = 300, QObject* parent = nullptr);

    // ── Marcador ──────────────────────────────────────────────────────────────
    void registerHumanGoal();
    void registerEnemyGoal();

    int getHumanGoals() const { return humanGoals_; }
    int getEnemyGoals() const { return enemyGoals_; }

    // ── Tiempo ────────────────────────────────────────────────────────────────
    /** Llama cada frame con el dt del juego. */
    void tick(float dt);

    int   getTimeLeft()    const { return int(timeLeft_); }
    float getGameTime()    const { return totalGameTime_; }
    GameState getState()   const { return state_; }
    bool  isPlaying()      const { return state_ == GameState::PLAYING; }

    void startMatch();
    void pauseMatch();
    void resumeMatch();

signals:
    void goalScoredByHuman();
    void goalScoredByEnemy();
    void gameOver(int humanGoals, int enemyGoals);
    void kickoffReady();
    void timeUpdated(int secondsLeft);
    void difficultyUpdated(float level);

private:
    int   humanGoals_    = 0;
    int   enemyGoals_    = 0;
    float timeLeft_;           // Segundos restantes
    float totalGameTime_ = 0.f;
    float goalDelay_     = 0.f; // Timer de pausa tras gol
    float matchDuration_;

    GameState state_ = GameState::KICKOFF;

    static constexpr float GOAL_PAUSE = 2.5f;    // Pausa tras gol (s)
    static constexpr float DIFFICULTY_UPDATE_INTERVAL = 5.f; // s
    float diffTimer_ = 0.f;
};
