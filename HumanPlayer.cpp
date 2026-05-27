#include "HumanPlayer.h"
#include "PhysicsEngine.h"
#include "GameExceptions.h"
#include <QPainter>
#include <QKeyEvent>
#include <Qt>
#include <cmath>

// ── Teclas de control ─────────────────────────────────────────────────────────
// Jugador 1: WASD + F (tiro) + G (pase)
// Jugador 2: Flechas + K (tiro) + L (pase)  [cuando sea el activo]
// El jugador activo es el que tiene el balón o el más cerca de él.

HumanPlayer::HumanPlayer(Vec2D pos, PlayerNumber number)
    : GameEntity(pos, 18.f)
    , number_(number)
{
    setZValue(5);
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::update(float dt) {
    if (!active) return;

    animTimer_ += dt;
    if (animTimer_ >= 1.f / ANIM_SPEED) {
        animTimer_ = 0.f;
        animFrame_ = (animFrame_ + 1) % 4;
    }

    if (isActive_) {
        applyMovement(dt);
    } else {
        // Movimiento automático hacia posición de apoyo
        Vec2D dir = idleTarget_ - position;
        float dist = dir.length();
        if (dist > 5.f) {
            Vec2D desired = dir.normalized() * (maxSpeed_ * 0.6f);
            velocity = velocity + (desired - velocity) * std::min(1.f, dt * 4.f);
        } else {
            velocity = PhysicsEngine::applyFriction(velocity, 8.f, dt);
        }

        Vec2D newPos = position + velocity * dt;
        newPos = PhysicsEngine::clampToField(newPos, minX_, maxX_, minY_, maxY_);
        setPosition(newPos);
    }

    // Si tiene el balón, moverlo con él
    if (hasBall_ && heldBall_) {
        Vec2D ballOffset = {0.f, -20.f}; // Balón delante del jugador
        heldBall_->setPosition(position + ballOffset);
    }

    isShooting_ = false; // Reset cada frame
}

void HumanPlayer::applyMovement(float dt) {
    // Construir dirección deseada desde input
    float dx = 0.f, dy = 0.f;
    if (keyLeft_)  dx -= 1.f;
    if (keyRight_) dx += 1.f;
    if (keyUp_)    dy -= 1.f;
    if (keyDown_)  dy += 1.f;

    Vec2D inputDir = {dx, dy};
    if (inputDir.lengthSq() > 0.f) inputDir = inputDir.normalized();

    Vec2D desired = inputDir * maxSpeed_;

    // Aceleración/desaceleración suave
    Vec2D diff = desired - velocity;
    float diffLen = diff.length();

    float rate = (inputDir.lengthSq() > 0.f) ? accel_ : decel_;
    float step = rate * dt;

    if (step >= diffLen) {
        velocity = desired;
    } else {
        velocity = velocity + diff.normalized() * step;
    }

    Vec2D newPos = position + velocity * dt;
    newPos = PhysicsEngine::clampToField(newPos, minX_, maxX_, minY_, maxY_);
    setPosition(newPos);
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::handleKeyPress(int key) {
    if (number_ == PlayerNumber::ONE) {
        if (key == Qt::Key_W) keyUp_    = true;
        if (key == Qt::Key_S) keyDown_  = true;
        if (key == Qt::Key_A) keyLeft_  = true;
        if (key == Qt::Key_D) keyRight_ = true;
        if (key == Qt::Key_F && hasBall_ && isActive_) {
            shoot(goalCenter_);
        }
        // El pase se gestiona desde Level2Scene (necesita referencia al compañero)
    } else {
        if (key == Qt::Key_Up)    keyUp_    = true;
        if (key == Qt::Key_Down)  keyDown_  = true;
        if (key == Qt::Key_Left)  keyLeft_  = true;
        if (key == Qt::Key_Right) keyRight_ = true;
        if (key == Qt::Key_K && hasBall_ && isActive_) {
            shoot(goalCenter_);
        }
    }
}

void HumanPlayer::handleKeyRelease(int key) {
    if (number_ == PlayerNumber::ONE) {
        if (key == Qt::Key_W) keyUp_    = false;
        if (key == Qt::Key_S) keyDown_  = false;
        if (key == Qt::Key_A) keyLeft_  = false;
        if (key == Qt::Key_D) keyRight_ = false;
    } else {
        if (key == Qt::Key_Up)    keyUp_    = false;
        if (key == Qt::Key_Down)  keyDown_  = false;
        if (key == Qt::Key_Left)  keyLeft_  = false;
        if (key == Qt::Key_Right) keyRight_ = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::giveBall(Ball* ball) {
    if (!ball)
        throw InvalidGameStateException("giveBall: puntero nulo al balón.");
    heldBall_ = ball;
    hasBall_  = true;
    ball->pickup();
}

Ball* HumanPlayer::releaseBall() {
    hasBall_ = false;
    Ball* b  = heldBall_;
    heldBall_ = nullptr;
    return b;
}

void HumanPlayer::shoot(Vec2D goalCenter) {
    if (!hasBall_ || !heldBall_) return;
    isShooting_ = true;
    Ball* b = releaseBall();
    b->shoot(goalCenter, 550.f);
}

void HumanPlayer::passToBuddy(HumanPlayer* buddy) {
    if (!hasBall_ || !heldBall_ || !buddy) return;
    Ball* b = releaseBall();
    b->pass(buddy->getPosition(), 400.f);
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::setBounds(float minX, float maxX, float minY, float maxY) {
    minX_ = minX; maxX_ = maxX;
    minY_ = minY; maxY_ = maxY;
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::onCollision(Collidable* other, Vec2D normal) {
    // Si choca con Tazmania: colisión elástica (el motor lo resuelve)
    (void)other; (void)normal;
    // La velocidad se actualiza externamente desde Level2Scene
}

// ─────────────────────────────────────────────────────────────────────────────
void HumanPlayer::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    drawPlayer(p, hasBall_);
}

void HumanPlayer::drawPlayer(QPainter* p, bool withBall) {
    // Colores: J1 = azul cielo, J2 = verde lima (equipo humano)
    QColor bodyColor = (number_ == PlayerNumber::ONE)
                       ? QColor(40, 100, 220)
                       : QColor(40, 180, 60);
    QColor skinColor = QColor(255, 210, 170);
    QColor shirtColor = bodyColor;

    // Sombra
    p->setBrush(QColor(0,0,0,50));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 16), 12, 5);

    // Piernas animadas
    float legSwing = std::sin(animFrame_ * 1.57f) * 6.f;
    p->setBrush(QColor(20, 30, 100));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-8 + legSwing, 10, 7, 16), 3, 3);
    p->drawRoundedRect(QRectF(1  - legSwing, 10, 7, 16), 3, 3);

    // Cuerpo / camiseta
    p->setBrush(shirtColor);
    p->setPen(QPen(bodyColor.darker(130), 1));
    p->drawRoundedRect(QRectF(-11, -4, 22, 18), 5, 5);

    // Número en camiseta
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 7, QFont::Bold));
    p->drawText(QRectF(-6, 0, 12, 10), Qt::AlignCenter,
                number_ == PlayerNumber::ONE ? "1" : "2");

    // Cabeza
    p->setBrush(skinColor);
    p->setPen(QPen(QColor(180,130,90), 1));
    p->drawEllipse(QPointF(0, -14), 11, 11);

    // Casco (color equipo)
    p->setBrush(bodyColor.darker(120));
    p->setPen(Qt::NoPen);
    p->drawChord(QRectF(-11, -25, 22, 22), 0 * 16, 180 * 16);

    // Ojos
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(-4, -15), 2, 2);
    p->drawEllipse(QPointF(4,  -15), 2, 2);

    // Indicador de jugador activo
    if (isActive_) {
        p->setPen(QPen(Qt::yellow, 2));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(QPointF(0, 0), collRadius + 2, collRadius + 2);
    }

    // Si tiene el balón: dibuja mini-balón en la mano
    if (withBall) {
        p->setBrush(QColor(230, 110, 10));
        p->setPen(QPen(Qt::black, 1));
        p->drawEllipse(QPointF(14, -4), 6, 6);
    }
}
