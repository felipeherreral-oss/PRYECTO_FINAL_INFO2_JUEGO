#pragma once
#include "Vec2D.h"

/**
 * @brief Interfaz para entidades que participan en colisiones físicas.
 *        Herencia múltiple: GameEntity + Collidable.
 */
class Collidable {
public:
    virtual ~Collidable() = default;

    /** Llamado cuando esta entidad colisiona con otra. */
    virtual void onCollision(Collidable* other, Vec2D collisionNormal) = 0;

    /** Masa para cálculo de colisión elástica (kg). */
    virtual float getMass() const = 0;

    /** Velocidad actual (para resolver colisión elástica). */
    virtual Vec2D getVelocity() const = 0;
    virtual void  setVelocity(Vec2D v)  = 0;
    virtual Vec2D getPosition() const   = 0;
};
