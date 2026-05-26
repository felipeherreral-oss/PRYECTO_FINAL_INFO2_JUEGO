#pragma once
#include "GameEntity.h"
#include "Collidable.h"
#include "AIAgent.h"
#include "PhysicsEngine.h"
#include "Ball.h"
#include <QPainter>
#include <QString>
#include <memory>

/**
 * @brief Jugador enemigo (personaje Looney Tunes).
 *
 *  Dos tipos de defensa:
 *  - TAZMANIA:  usa Movimiento Circular Uniforme (MCU) como física base.
 *               Puede cambiar a modo "intercepción" cuando el balón se acerca.
 *  - BUGS/DAFFY: defensa posicional; usa AIAgent para anticiparse al jugador.
 *
 *  Hereda de GameEntity y Collidable.
 *  Uso de herencia propia: EnemyPlayer hereda de GameEntity (no de Qt directamente).
 */
class EnemyPlayer : public GameEntity, public Collidable {
public:
    enum class EnemyType { TAZMANIA, BUGS_BUNNY, DAFFY_DUCK };
    enum class EnemyState { ORBITING, CHASING, INTERCEPTING, RETURNING };

    /**
     * @param pos       Posición inicial.
     * @param type      Tipo de enemigo (define física y apariencia).
     * @param orbitCtr  Centro de la órbita (solo Tazmania).
     * @param orbitR    Radio de la órbita (px).
     * @param omega     Velocidad angular (rad/s).
     * @param phi0      Fase inicial (rad).
     * @param fieldW/H  Dimensiones del campo para AIAgent.
     */
    EnemyPlayer(Vec2D pos, EnemyType type,
                Vec2D orbitCtr, float orbitR, float omega, float phi0,
                float fieldW, float fieldH);

    // ── GameEntity ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    // ── Collidable ────────────────────────────────────────────────────────────
    void onCollision(Collidable* other, Vec2D normal) override;
    float getMass()      const override;
    Vec2D getVelocity()  const override { return velocity; }
    void  setVelocity(Vec2D v) override { velocity = v; }
    Vec2D getPosition()  const override { return position; }

    // ── IA ────────────────────────────────────────────────────────────────────
    /**
     * Actualiza la percepción del agente y define el movimiento.
     * Llamado desde Level2Scene con el estado actual del juego.
     */
    void updateAI(Vec2D ballPos, Vec2D ballVel,
                  Vec2D humanPlayerPos, bool humanIsShooting,
                  Vec2D ownGoalCenter, float dt);

    void updateDifficulty(float gameTimeSeconds);

    AIAgent* getAgent() { return agent_.get(); }
    EnemyType getType() const { return type_; }

    bool hasBall() const { return hasBall_; }
    void takeBall(Ball* b);
    Ball* dropBall();

private:
    EnemyType  type_;
    EnemyState state_ = EnemyState::ORBITING;

    // Física circular (Tazmania)
    Vec2D orbitCenter_;
    float orbitRadius_;
    float omega_;
    float phi0_;
    float orbitTime_ = 0.f;    // Tiempo acumulado en la órbita

    // AI
    std::unique_ptr<AIAgent> agent_;

    // Balón (si lo capturó)
    bool  hasBall_ = false;
    Ball* heldBall_ = nullptr;

    // Animación
    float animTimer_ = 0.f;
    int   animFrame_ = 0;

    // Parámetros de movimiento (sin MCU)
    float chaseSpeed_ = 160.f;

    void updateTazmania(Vec2D ballPos, Vec2D humanPos, float dt);
    void updateFieldPlayer(Vec2D ballPos, Vec2D ballVel,
                           Vec2D humanPos, bool isShooting,
                           Vec2D goalCenter, float dt);

    void drawTazmania(QPainter* p);
    void drawBugsBunny(QPainter* p);
    void drawDaffyDuck(QPainter* p);
};
