#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "PhysicsEngine.h"
#include <QPainter>

/**
 * @brief Balón de balonmano.
 *
 *  El balón puede estar:
 *  - HELD    : en posesión de un jugador (se mueve con él)
 *  - FREE    : rodando libremente con fricción (Verlet + fricción)
 *  - SHOT    : lanzado hacia el arco (Verlet + aceleración mínima)
 *  - PASSED  : pase entre jugadores
 *
 *  Hereda de Collidable para participar en colisiones.
 */
class Ball : public GameEntity, public Collidable {
public:
    enum class State { HELD, FREE, SHOT, PASSED };

    explicit Ball(Vec2D startPos);

    // ── GameEntity ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    // ── Collidable ────────────────────────────────────────────────────────────
    void onCollision(Collidable* other, Vec2D normal) override;
    float getMass()     const override { return 0.5f; }   // 500 g (reglamentario)
    Vec2D getVelocity() const override { return velocity; }
    void  setVelocity(Vec2D v) override { velocity = v; }
    Vec2D getPosition() const override { return position; }

    // ── Estado del balón ──────────────────────────────────────────────────────
    State getState()    const { return state_; }
    bool  isOwned()     const { return state_ == State::HELD; }

    /** Lanzar el balón hacia una posición objetivo con velocidad dada. */
    void shoot(Vec2D targetPos, float speed);

    /** Pasar el balón a una posición destino. */
    void pass(Vec2D targetPos, float speed);

    /** Soltar el balón (queda FREE). */
    void release(Vec2D withVelocity = Vec2D::zero());

    /** Tomar posesión del balón (HELD). */
    void pickup();

    /** Posición objetivo para pase (para la IA). */
    Vec2D getTarget() const { return target_; }

    void setBoundsCheck(float minX, float maxX, float minY, float maxY) {
        minX_ = minX; maxX_ = maxX; minY_ = minY; maxY_ = maxY;
    }

private:
    State state_  = State::FREE;
    Vec2D accel_  = Vec2D::zero();
    Vec2D target_;

    float minX_ = 0.f, maxX_ = 1200.f;
    float minY_ = 0.f, maxY_ = 700.f;

    static constexpr float FRICTION     = 1.8f;   // Coeficiente de fricción (campo)
    static constexpr float BALL_RADIUS  = 11.f;   // px visual
};
