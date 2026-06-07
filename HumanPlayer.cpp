#include "HumanPlayer.h"
#include "PhysicsEngine.h"
#include "GameExceptions.h"
#include "SpriteManager.h"
#include <QPainter>
#include <Qt>
#include <cmath>

HumanPlayer::HumanPlayer(Vec2D pos, PlayerNumber number)
    : GameEntity(pos, 18.f)
    , number_(number)
{
    setZValue(5);
}

void HumanPlayer::update(float dt) {
    if (!active) return;

    // Animación
    animTimer_ += dt;
    float animSpeed = (velocity.lengthSq() > 100.f) ? 8.f : 3.f;
    if (animTimer_ >= 1.f / animSpeed) {
        animTimer_ = 0.f;
        animFrame_ = (animFrame_ + 1) %
            SpriteManager::instance().frameCount(spriteKey(), currentAnimState_);
    }

    if (isActive_) {
        applyMovement(dt);
    } else {
        // Movimiento automático a posición de apoyo
        Vec2D dir  = idleTarget_ - position;
        float dist = dir.length();
        if (dist > 5.f) {
            Vec2D desired = dir.normalized() * (maxSpeed_ * 0.6f);
            velocity = velocity + (desired - velocity) * std::min(1.f, dt * 4.f);
        } else {
            velocity = PhysicsEngine::applyFriction(velocity, 8.f, dt);
        }
        Vec2D newPos = PhysicsEngine::clampToField(
            position + velocity * dt, minX_, maxX_, minY_, maxY_);
        setPosition(newPos);
    }

    // Determinar estado de animación
    if (isShooting_) {
        currentAnimState_ = SpriteManager::AnimState::SHOOT;
    } else if (velocity.lengthSq() > 400.f) {
        currentAnimState_ = SpriteManager::AnimState::RUN;
    } else {
        currentAnimState_ = SpriteManager::AnimState::IDLE;
    }

    // Mover el balón con el jugador
    if (hasBall_ && heldBall_) {
        Vec2D ballOffset = {0.f, -20.f};
        heldBall_->setPosition(position + ballOffset);
    }

    isShooting_ = false;
}

void HumanPlayer::applyMovement(float dt) {
    float dx = 0.f, dy = 0.f;
    if (keyLeft_)  dx -= 1.f;
    if (keyRight_) dx += 1.f;
    if (keyUp_)    dy -= 1.f;
    if (keyDown_)  dy += 1.f;

    Vec2D inputDir = {dx, dy};
    if (inputDir.lengthSq() > 0.f) inputDir = inputDir.normalized();

    Vec2D desired = inputDir * maxSpeed_;
    Vec2D diff    = desired - velocity;
    float rate    = (inputDir.lengthSq() > 0.f) ? accel_ : decel_;
    float step    = rate * dt;
    float diffLen = diff.length();

    velocity = (step >= diffLen) ? desired : velocity + diff.normalized() * step;

    Vec2D newPos = PhysicsEngine::clampToField(
        position + velocity * dt, minX_, maxX_, minY_, maxY_);
    setPosition(newPos);
}

void HumanPlayer::handleKeyPress(int key) {
    if (number_ == PlayerNumber::ONE) {
        if (key == Qt::Key_W) keyUp_    = true;
        if (key == Qt::Key_S) keyDown_  = true;
        if (key == Qt::Key_A) keyLeft_  = true;
        if (key == Qt::Key_D) keyRight_ = true;
        if (key == Qt::Key_F && hasBall_ && isActive_) shoot(goalCenter_);
    } else {
        if (key == Qt::Key_Up)    keyUp_    = true;
        if (key == Qt::Key_Down)  keyDown_  = true;
        if (key == Qt::Key_Left)  keyLeft_  = true;
        if (key == Qt::Key_Right) keyRight_ = true;
        if (key == Qt::Key_K && hasBall_ && isActive_) shoot(goalCenter_);
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

void HumanPlayer::giveBall(Ball* ball) {
    if (!ball)
        throw InvalidGameStateException("giveBall: puntero nulo al balón.");
    heldBall_ = ball;
    hasBall_  = true;
    ball->pickup();
}

Ball* HumanPlayer::releaseBall() {
    hasBall_  = false;
    Ball* b   = heldBall_;
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

void HumanPlayer::setBounds(float minX, float maxX, float minY, float maxY) {
    minX_ = minX; maxX_ = maxX;
    minY_ = minY; maxY_ = maxY;
}

void HumanPlayer::onCollision(Collidable* other, Vec2D normal) {
    (void)other; (void)normal;
}

//  PAINT
void HumanPlayer::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    SpriteManager& sm = SpriteManager::instance();

    // Tamaño del sprite en pantalla
    QSize sprSize(72, 88);

    // Espejado: J2 (espejo) mira hacia la izquierda por defecto;
    // si se mueve hacia la derecha, no espejamos
    bool flipH = (velocity.x < -10.f);

    QPixmap frame = sm.getFrame(spriteKey(), currentAnimState_,
                                animFrame_, sprSize, flipH);

    // Centrar el sprite sobre la posición del jugador
    p->drawPixmap(-sprSize.width() / 2, -sprSize.height() / 2, frame);

    // Indicador de jugador activo (círculo amarillo)
    if (isActive_) {
        p->setPen(QPen(Qt::yellow, 2));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(QPointF(0, 0), collRadius + 3, collRadius + 3);
    }

    // Número del jugador
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 7, QFont::Bold));
    QRectF numRect(-8, sprSize.height()/2 - 4, 16, 10);
    p->setBrush(QColor(0, 0, 0, 140));
    p->drawRoundedRect(numRect, 3, 3);
    p->drawText(numRect, Qt::AlignCenter,
                number_ == PlayerNumber::ONE ? "J1" : "J2");
}

QString HumanPlayer::spriteKey() const {
    return (number_ == PlayerNumber::ONE) ? "j1" : "j2";
}
