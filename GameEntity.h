#pragma once
#include "Vec2D.h"
#include <QGraphicsItem>
#include <QString>

/**
 * @brief Entidad base abstracta del juego.
 *        Toda entidad tiene posición, velocidad, radio de colisión
 *        y puede actualizarse cada frame.
 *
 *        Hereda de QGraphicsItem para poder ser parte de la escena Qt.
 */
class GameEntity : public QGraphicsItem {
public:
    explicit GameEntity(Vec2D pos, float collisionRadius, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent)
        , position(pos)
        , velocity(Vec2D::zero())
        , collRadius(collisionRadius)
        , active(true)
    {
        setPos(position.x, position.y);
    }

    virtual ~GameEntity() = default;

    // ── Ciclo de juego ──────────────────────────────────────────────────────
    /** Actualiza lógica + físicas. dt en segundos. */
    virtual void update(float dt) = 0;

    // ── Posición / velocidad ─────────────────────────────────────────────────
    Vec2D getPosition()  const { return position; }
    Vec2D getVelocity()  const { return velocity; }
    float getRadius()    const { return collRadius; }
    bool  isActive()     const { return active; }

    void setPosition(Vec2D p) {
        position = p;
        setPos(p.x, p.y);
    }
    void setVelocity(Vec2D v) { velocity = v; }
    void setActive(bool a)    { active = a; setVisible(a); }

    /** Colisión simple por círculos */
    bool overlaps(const GameEntity& other) const {
        float dist = position.distanceTo(other.position);
        return dist < (collRadius + other.collRadius);
    }

    // ── QGraphicsItem ────────────────────────────────────────────────────────
    QRectF boundingRect() const override {
        float d = collRadius * 2.f;
        return QRectF(-collRadius, -collRadius, d, d);
    }

protected:
    Vec2D position;
    Vec2D velocity;
    float collRadius;
    bool  active;
};
