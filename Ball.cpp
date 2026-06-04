#include "Ball.h"
#include "SpriteManager.h"
#include <QPainter>
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

        // Nota: NO se rebota en los bordes. Si el balón cruza los límites del
        // terreno, la escena (checkOutOfBounds) lo detecta y lo reubica en el
        // centro para que ambos equipos luchen por él. Así la escena es la única
        // autoridad sobre el "fuera de juego".

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
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    int d = int(BALL_RADIUS * 2.4f);
    QPixmap ballPx = SpriteManager::instance().getBall(QSize(d, d));

    if (!ballPx.isNull()) {
        p->drawPixmap(-d/2, -d/2, ballPx);
    } else {
        // Fallback visual si el sprite no cargó
        QRadialGradient grad(-3, -3, BALL_RADIUS * 1.5f);
        grad.setColorAt(0.0, QColor(255, 130, 60));
        grad.setColorAt(1.0, QColor(160, 60, 0));
        p->setBrush(grad);
        p->setPen(QPen(Qt::black, 1));
        p->drawEllipse(QPointF(0, 0), BALL_RADIUS, BALL_RADIUS);
    }
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
