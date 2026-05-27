#include "Ball.h"
#include <QPainter>
#include <QBrush>
#include <cmath>

Ball::Ball(Vec2D startPos)
    : GameEntity(startPos, BALL_RADIUS)
{
    setZValue(10); // Dibujar encima de jugadores
}

void Ball::update(float dt) {
    if (!active) return;

    switch (state_) {
    case State::HELD:
        // Posición la controla el jugador que lo tiene
        break;

    case State::FREE: {
        // Integración de Verlet con fricción
        position = PhysicsEngine::verletPosition(position, velocity, accel_, dt);
        velocity = PhysicsEngine::verletVelocity(velocity, accel_, dt);
        velocity = PhysicsEngine::applyFriction(velocity, FRICTION, dt);

        // Rebotar en los bordes del campo
        if (position.x < minX_ + BALL_RADIUS) {
            position.x = minX_ + BALL_RADIUS;
            velocity = PhysicsEngine::reflect(velocity, {1.f, 0.f});
            velocity *= 0.6f; // Pérdida de energía en rebote
        }
        if (position.x > maxX_ - BALL_RADIUS) {
            position.x = maxX_ - BALL_RADIUS;
            velocity = PhysicsEngine::reflect(velocity, {-1.f, 0.f});
            velocity *= 0.6f;
        }
        if (position.y < minY_ + BALL_RADIUS) {
            position.y = minY_ + BALL_RADIUS;
            velocity = PhysicsEngine::reflect(velocity, {0.f, 1.f});
            velocity *= 0.6f;
        }
        if (position.y > maxY_ - BALL_RADIUS) {
            position.y = maxY_ - BALL_RADIUS;
            velocity = PhysicsEngine::reflect(velocity, {0.f, -1.f});
            velocity *= 0.6f;
        }

        // Si la velocidad es muy baja, detener
        if (velocity.lengthSq() < 1.f) {
            velocity = Vec2D::zero();
        }
        break;
    }

    case State::SHOT:
    case State::PASSED: {
        // Movimiento rectilíneo rápido hacia destino (sin fricción fuerte)
        position = PhysicsEngine::verletPosition(position, velocity, Vec2D::zero(), dt);

        // Verificar si llegó al destino
        if (position.distanceTo(target_) < 15.f) {
            state_ = State::FREE;
            velocity *= 0.3f; // Frena al llegar
        }

        // Rebotar en bordes
        if (position.x < minX_ || position.x > maxX_ ||
            position.y < minY_ || position.y > maxY_) {
            state_ = State::FREE;
        }
        break;
    }
    }

    setPos(position.x, position.y);
}

void Ball::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);

    // Sombra
    p->setBrush(QColor(0, 0, 0, 60));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 3), BALL_RADIUS - 1, BALL_RADIUS - 1);

    // Balón naranja con hexágonos negros (balonmano)
    QRadialGradient grad(-3, -3, BALL_RADIUS * 1.5f);
    grad.setColorAt(0.0, QColor(255, 160, 40));
    grad.setColorAt(0.6, QColor(230, 110, 10));
    grad.setColorAt(1.0, QColor(180, 70, 0));
    p->setBrush(grad);
    p->setPen(QPen(Qt::black, 1));
    p->drawEllipse(QPointF(0, 0), BALL_RADIUS, BALL_RADIUS);

    // Líneas decorativas del balón
    p->setPen(QPen(QColor(80, 40, 0), 1.2f));
    p->drawArc(QRectF(-BALL_RADIUS * 0.5f, -BALL_RADIUS, BALL_RADIUS, BALL_RADIUS * 2),
               30 * 16, 120 * 16);
    p->drawArc(QRectF(-BALL_RADIUS * 0.5f, -BALL_RADIUS, BALL_RADIUS, BALL_RADIUS * 2),
               210 * 16, 120 * 16);
}

void Ball::shoot(Vec2D targetPos, float speed) {
    target_  = targetPos;
    Vec2D dir = (targetPos - position).normalized();
    velocity = dir * speed;
    state_   = State::SHOT;
    accel_   = Vec2D::zero();
}

void Ball::pass(Vec2D targetPos, float speed) {
    target_  = targetPos;
    Vec2D dir = (targetPos - position).normalized();
    velocity = dir * speed;
    state_   = State::PASSED;
    accel_   = Vec2D::zero();
}

void Ball::release(Vec2D withVelocity) {
    velocity = withVelocity;
    state_   = State::FREE;
    accel_   = Vec2D::zero();
}

void Ball::pickup() {
    velocity = Vec2D::zero();
    accel_   = Vec2D::zero();
    state_   = State::HELD;
}

void Ball::onCollision(Collidable* other, Vec2D normal) {
    (void)other;
    if (state_ == State::SHOT || state_ == State::PASSED) {
        // Rebota con el bloqueador
        velocity = PhysicsEngine::reflect(velocity, normal) * 0.5f;
        state_   = State::FREE;
    }
}
