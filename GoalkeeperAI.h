#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "AIAgent.h"
#include "Ball.h"
#include "SpriteManager.h"
#include <QPainter>
#include <memory>

/**
 * @brief Arquero controlado por IA con aprendizaje adaptativo.
 *
 *  Herencia propia: GoalkeeperAI hereda de GameEntity (no de QGraphicsItem directo).
 *
 *  En esta vista cenital los arcos están a izquierda/derecha, así que la boca
 *  del arco es VERTICAL. El arquero mantiene una X fija (justo delante de su
 *  línea de gol) y se mueve VERTICALMENTE siguiendo la posición/predicción del
 *  balón para atajar los tiros.
 *
 *  Usa AIAgent para percepción + dificultad (velocidad) + aprendizaje por zona.
 */
class GoalkeeperAI : public GameEntity, public Collidable {
public:
    enum class Team { HUMAN, ENEMY };

    /**
     * @param pos          Posición inicial (X fija del arquero, Y centro del arco).
     * @param team         Qué arco defiende.
     * @param goalTopY     Y del borde superior de la boca del arco.
     * @param goalBotY     Y del borde inferior de la boca del arco.
     * @param fieldW/H     Dimensiones del campo jugable.
     */
    GoalkeeperAI(Vec2D pos, Team team,
                 float goalTopY, float goalBotY,
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
    void notifyGoalScored(float ballImpactY);

    // Aprendizaje cuando para un tiro
    void notifySave();

    void updateDifficulty(float gameTimeSeconds);
    AIAgent* getAgent() { return agent_.get(); }

    float getDifficulty() const { return agent_->getDifficultyLevel(); }

    Team getTeam() const { return team_; }

    float getGoalTop()  const { return goalTop_; }
    float getGoalBot()  const { return goalBot_; }
    float getFixedX()   const { return fixedX_; }

private:
    Team   team_;
    float  fixedX_;            // X fija del arquero (no se mueve horizontalmente)
    float  goalTop_, goalBot_; // límites verticales de la boca del arco
    float  goalCenterY_;

    std::unique_ptr<AIAgent> agent_;

    // Animación
    float animTimer_ = 0.f;
    int   animFrame_ = 0;
    bool  isSaving_  = false;
    float saveTimer_ = 0.f;
    SpriteManager::AnimState currentAnim_ = SpriteManager::AnimState::IDLE;
};
