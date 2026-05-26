#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "Ball.h"
#include <QPainter>
#include <QKeyEvent>
#include <QString>

/**
 * @brief Jugador controlado por el humano.
 *
 *  El jugador activo (que tiene el balón o está más cerca) recibe input.
 *  El jugador inactivo se mueve automáticamente a una posición de apoyo.
 *
 *  Hereda de GameEntity (posición, lógica) y Collidable (choque elástico).
 */
class HumanPlayer : public GameEntity, public Collidable {
public:
    enum class PlayerNumber { ONE, TWO };

    /**
     * @param pos      Posición inicial en el campo (px).
     * @param number   Jugador 1 o Jugador 2.
     */
    HumanPlayer(Vec2D pos, PlayerNumber number);

    // ── GameEntity ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    // ── Collidable ────────────────────────────────────────────────────────────
    void onCollision(Collidable* other, Vec2D normal) override;
    float getMass()     const override { return 80.f; }  // kg promedio
    Vec2D getVelocity() const override { return velocity; }
    void  setVelocity(Vec2D v) override { velocity = v; }
    Vec2D getPosition() const override { return position; }

    // ── Input del jugador ─────────────────────────────────────────────────────
    void handleKeyPress(int key);
    void handleKeyRelease(int key);

    // ── Balón ─────────────────────────────────────────────────────────────────
    bool hasBall()      const { return hasBall_; }
    void giveBall(Ball* ball);
    Ball* releaseBall();

    /** Lanzar al arco (objetivo = posición del arco rival). */
    void shoot(Vec2D goalCenter);

    /** Pasar al compañero. */
    void passToBuddy(HumanPlayer* buddy);

    // ── Estado ────────────────────────────────────────────────────────────────
    bool isActive()     const { return isActive_; }
    void setActiveControl(bool active) { isActive_ = active; }

    PlayerNumber getNumber() const { return number_; }

    void setBounds(float minX, float maxX, float minY, float maxY);
    void setGoalCenter(Vec2D gc) { goalCenter_ = gc; }

    // Para el auto-movimiento del jugador sin control
    void setIdleTarget(Vec2D t) { idleTarget_ = t; }

    bool isShooting() const { return isShooting_; }

private:
    PlayerNumber number_;
    bool isActive_    = false;
    bool hasBall_     = false;
    bool isShooting_  = false;

    Ball* heldBall_   = nullptr;

    // Input acumulado
    float inputDX_ = 0.f, inputDY_ = 0.f;

    // Parámetros de movimiento
    float maxSpeed_   = 200.f;   // px/s
    float accel_      = 800.f;   // px/s²
    float decel_      = 500.f;   // fricción al soltar tecla

    // Límites del campo (mitad del lado humano)
    float minX_ = 0.f, maxX_ = 1200.f;
    float minY_ = 0.f, maxY_ = 700.f;

    Vec2D goalCenter_   = {1100.f, 350.f};
    Vec2D idleTarget_   = {600.f, 350.f};

    // Animación
    float animTimer_    = 0.f;
    int   animFrame_    = 0;
    static constexpr float ANIM_SPEED = 8.f;  // frames/s

    // Teclas activas
    bool keyUp_ = false, keyDown_ = false;
    bool keyLeft_ = false, keyRight_ = false;
    bool keyShoot_ = false, keyPass_ = false;

    void applyMovement(float dt);
    void drawPlayer(QPainter* p, bool withBall);
};
