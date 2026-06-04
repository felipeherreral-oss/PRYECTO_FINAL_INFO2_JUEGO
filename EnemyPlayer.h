#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "AIAgent.h"
#include "PhysicsEngine.h"
#include "Ball.h"
#include "SpriteManager.h"
#include <QPainter>
#include <QString>
#include <memory>

/**
 * @brief Jugador enemigo (Looney Tunes).
 *
 *  - TAZMANIA  : MCU (órbita) + modo tornado (SPIN) + mareado (DIZZY)
 *  - BUGS_BUNNY: defensa posicional con AIAgent
 *  - DAFFY_DUCK: usado como arquero rival (GoalkeeperAI), no como EnemyPlayer
 *
 *  Herencia propia: EnemyPlayer → GameEntity (no hereda de Qt directamente).
 */
class EnemyPlayer : public GameEntity, public Collidable {
public:
    enum class EnemyType  { TAZMANIA, BUGS_BUNNY, DAFFY_DUCK };
    enum class EnemyState { ORBITING, CHASING, INTERCEPTING, RETURNING };

    EnemyPlayer(Vec2D pos, EnemyType type,
                Vec2D orbitCtr, float orbitR, float omega, float phi0,
                float fieldW, float fieldH);

    // ── GameEntity ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    // ── Collidable ────────────────────────────────────────────────────────────
    void  onCollision(Collidable* other, Vec2D normal) override;
    float getMass()      const override;
    Vec2D getVelocity()  const override { return velocity; }
    void  setVelocity(Vec2D v) override { velocity = v; }
    Vec2D getPosition()  const override { return position; }

    // ── IA ────────────────────────────────────────────────────────────────────
    void updateAI(Vec2D ballPos, Vec2D ballVel,
                  Vec2D humanPlayerPos, bool humanIsShooting,
                  Vec2D ownGoalCenter, float dt);

    void updateDifficulty(float gameTimeSeconds);
    AIAgent* getAgent() { return agent_.get(); }

    EnemyType getType() const { return type_; }

    // ── Movimiento ofensivo / persecución (controlado por la escena) ───────────
    /** Mueve al jugador hacia un objetivo con la velocidad dada (clamp al campo). */
    void steerTo(Vec2D target, float speed, float dt);

    /** Define los límites del terreno jugable (zona amarilla). */
    void setPlayBounds(float minX, float maxX, float minY, float maxY) {
        minX_ = minX; maxX_ = maxX; minY_ = minY; maxY_ = maxY;
    }

    // ── Balón ─────────────────────────────────────────────────────────────────
    bool  hasBall() const { return hasBall_; }
    void  takeBall(Ball* b);
    Ball* dropBall();

    // ── Estado tornado/mareo ──────────────────────────────────────────────────
    bool isSpinning() const { return isSpinning_; }
    bool isDizzy()    const { return isDizzy_; }

private:
    EnemyType  type_;
    EnemyState state_ = EnemyState::ORBITING;

    // Física circular (Tazmania MCU)
    Vec2D orbitCenter_;
    float orbitRadius_;
    float omega_, phi0_;
    float orbitTime_ = 0.f;

    // AI
    std::unique_ptr<AIAgent> agent_;

    // Balón
    bool  hasBall_  = false;
    Ball* heldBall_ = nullptr;

    // Tornado / mareo
    bool  isSpinning_   = false;
    bool  isDizzy_      = false;
    float spinTimer_    = 0.f;
    float dizzyTimer_   = 0.f;
    float spinCooldown_ = 5.f;

    static constexpr float SPIN_DURATION     = 2.5f;
    static constexpr float DIZZY_DURATION    = 2.0f;
    static constexpr float SPIN_COOLDOWN_MIN = 6.0f;

    // Animación
    float animTimer_ = 0.f;
    int   animFrame_ = 0;
    SpriteManager::AnimState currentAnim_ = SpriteManager::AnimState::IDLE;

    float chaseSpeed_ = 160.f;

    // Límites del terreno jugable (zona amarilla)
    float minX_ = 0.f, maxX_ = 1200.f, minY_ = 0.f, maxY_ = 750.f;

    void updateTazmania(Vec2D ballPos, Vec2D humanPos, float dt);
    void updateFieldPlayer(Vec2D ballPos, Vec2D ballVel,
                           Vec2D humanPos, bool isShooting,
                           Vec2D goalCenter, float dt);
    QString spriteKey() const;
};
