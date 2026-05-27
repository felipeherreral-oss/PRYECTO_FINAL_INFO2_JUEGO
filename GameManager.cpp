#include "GameManager.h"
#include "GameExceptions.h"
#include <cmath>
#include <algorithm>

GameManager::GameManager(int matchDurationSeconds, QObject* parent)
    : QObject(parent)
    , timeLeft_(float(matchDurationSeconds))
    , matchDuration_(float(matchDurationSeconds))
{
    if (matchDurationSeconds <= 0)
        throw InvalidGameStateException("La duración del partido debe ser positiva.");
}

void GameManager::startMatch() {
    state_     = GameState::PLAYING;
    goalDelay_ = 0.f;
    emit kickoffReady();
}

void GameManager::pauseMatch() {
    if (state_ == GameState::PLAYING)
        state_ = GameState::KICKOFF; // Reusar como pausa simple
}

void GameManager::resumeMatch() {
    if (state_ == GameState::KICKOFF)
        state_ = GameState::PLAYING;
}

void GameManager::registerHumanGoal() {
    humanGoals_++;
    state_     = GameState::GOAL_SCORED;
    goalDelay_ = GOAL_PAUSE;
    emit goalScoredByHuman();
}

void GameManager::registerEnemyGoal() {
    enemyGoals_++;
    state_     = GameState::GOAL_SCORED;
    goalDelay_ = GOAL_PAUSE;
    emit goalScoredByEnemy();
}

void GameManager::tick(float dt) {
    if (state_ == GameState::GOAL_SCORED) {
        goalDelay_ -= dt;
        if (goalDelay_ <= 0.f) {
            state_ = GameState::PLAYING;
            emit kickoffReady();
        }
        return;
    }

    if (state_ != GameState::PLAYING) return;

    totalGameTime_ += dt;
    timeLeft_ -= dt;

    // Actualizar dificultad periódicamente
    diffTimer_ += dt;
    if (diffTimer_ >= DIFFICULTY_UPDATE_INTERVAL) {
        diffTimer_ = 0.f;
        // Nivel de dificultad basado en tiempo de juego
        float diff = std::min(1.f, totalGameTime_ / 180.f) * 0.85f + 0.15f;
        emit difficultyUpdated(diff);
    }

    emit timeUpdated(int(timeLeft_));

    if (timeLeft_ <= 0.f) {
        timeLeft_ = 0.f;
        state_    = GameState::GAME_OVER;
        emit gameOver(humanGoals_, enemyGoals_);
    }
}
