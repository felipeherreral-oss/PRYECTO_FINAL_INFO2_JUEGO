#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "AIAgent.h"
#include "Ball.h"
#include <QPainter>

/**
 * @brief Arquero controlado por IA con aprendizaje adaptativo.
 *
 *  Herencia propia: GoalkeeperAI hereda de GameEntity (no de QGraphicsItem directo).
 *  El arquero solo se mueve lateralmente dentro del área de portería.
 *  Usa AIAgent para percepción + razonamiento + aprendizaje.
 */
class GoalkeeperAI : public GameEntity, public Collidable {
public:
    enum class Team { HUMAN, ENEMY };

    /**
     * @param pos          Posición inicial (centro del arco).
     * @param team         Qué arco defiende.
     * @param goalLeft     X del poste izquierdo.
     * @param goalRight    X del poste derecho.
     * @param fieldW/H     Dimensiones del campo.
     */
    GoalkeeperAI(Vec2D pos, Team team,
                 float goalLeft, float goalRight,
                 float fieldW, float fieldH);

    // ── GameEntity ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    // ── Collidable ────────────────────────────────────────────────────────────
    void onCollision(Collidable* other, Vec2D normal) override;
    float getMass()      const override { return 85.f; }
    Vec2D getVelocity()  const override { return velocity; }
    void  setVelocity(Vec2D v) override { velocity = v; }
    Vec2D getPosition()  const override { return position; }

    // ── IA del arquero ────────────────────────────────────────────────────────
    void updateAI(Vec2D ballPos, Vec2D ballVel,
                  Vec2D shooterPos, bool isShooting, float dt);

    // Aprendizaje cuando entra un gol
    void notifyGoalScored(float ballImpactX);

    // Aprendizaje cuando para un tiro
    void notifySave();

    void updateDifficulty(float gameTimeSeconds);

    float getDifficulty() const { return agent_->getDifficultyLevel(); }

    Team getTeam() const { return team_; }

    // Posición del poste izquierdo/derecho del arco
    float getGoalLeft()  const { return goalLeft_; }
    float getGoalRight() const { return goalRight_; }
    float getGoalY()     const { return position.y; }

private:
    Team   team_;
    float  goalLeft_, goalRight_;
    float  goalCenterX_;
    float  homeY_;             // Y fija del arquero (no se mueve verticalmente)

    std::unique_ptr<AIAgent> agent_;

    // Animación
    float animTimer_ = 0.f;
    int   animFrame_ = 0;
    bool  isSaving_  = false;
    float saveTimer_ = 0.f;

    void drawGoalkeeper(QPainter* p);
    void drawEnemyGoalkeeper(QPainter* p);
};
