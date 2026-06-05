#include "EnemyPlayer.h"
#include "Ball.h"
#include "SpriteManager.h"
#include "AudioManager.h"
#include <QPainter>
#include <cmath>

EnemyPlayer::EnemyPlayer(Vec2D pos, EnemyType type,
                         Vec2D orbitCtr, float orbitR, float omega, float phi0,
                         float fieldW, float fieldH)
    : GameEntity(pos, type == EnemyType::TAZMANIA ? 22.f : 18.f)
    , type_(type)
    , orbitCenter_(orbitCtr)
    , orbitRadius_(orbitR)
    , omega_(omega)
    , phi0_(phi0)
    , agent_(std::make_unique<AIAgent>(AIAgent::AgentType::FIELD_DEFENDER, fieldW, fieldH))
    , spinTimer_(0.f)
    , dizzyTimer_(0.f)
{
    setZValue(4);
}

void EnemyPlayer::update(float dt) {
    if (!active) return;

    // Avanzar timer de tornado y mareo
    if (isSpinning_) {
        spinTimer_ -= dt;
        if (spinTimer_ <= 0.f) {
            isSpinning_ = false;
            isDizzy_    = true;
            dizzyTimer_ = DIZZY_DURATION;
            currentAnim_ = SpriteManager::AnimState::DIZZY;
            AudioManager::instance().playTazDizzy();   // terminó de girar → mareado
        } else {
            currentAnim_ = SpriteManager::AnimState::SPIN;
        }
    } else if (isDizzy_) {
        dizzyTimer_ -= dt;
        if (dizzyTimer_ <= 0.f) {
            isDizzy_     = false;
            currentAnim_ = SpriteManager::AnimState::IDLE;
        }
    }

    // Animación
    animTimer_ += dt;
    float animSpeed = (currentAnim_ == SpriteManager::AnimState::RUN)  ? 10.f :
                      (currentAnim_ == SpriteManager::AnimState::SPIN) ? 14.f : 5.f;
    if (animTimer_ >= 1.f / animSpeed) {
        animTimer_ = 0.f;
        animFrame_ = (animFrame_ + 1) %
            std::max(1, SpriteManager::instance().frameCount(spriteKey(), currentAnim_));
    }

    // Si lleva el balón, mantenerlo pegado al jugador (driblando)
    if (hasBall_ && heldBall_) {
        Vec2D offset = {-16.f, -10.f};   // ligeramente hacia el arco humano
        heldBall_->setPosition(position + offset);
    }

    setPos(position.x, position.y);
}

void EnemyPlayer::updateAI(Vec2D ballPos, Vec2D ballVel,
                            Vec2D humanPlayerPos, bool humanIsShooting,
                            Vec2D ownGoalCenter, float dt)
{
    if (!active || isDizzy_) return;

    agent_->perceive(ballPos, ballVel, humanPlayerPos, humanIsShooting);

    if (type_ == EnemyType::TAZMANIA) {
        updateTazmania(ballPos, humanPlayerPos, dt);
    } else {
        updateFieldPlayer(ballPos, ballVel, humanPlayerPos,
                          humanIsShooting, ownGoalCenter, dt);
    }
}

// ─── TAZMANIA ─────────────────────────────────────────────────────────────────
void EnemyPlayer::updateTazmania(Vec2D ballPos, Vec2D humanPos, float dt) {
    if (isSpinning_ || isDizzy_) return;

    orbitTime_ += dt;

    float distToBall  = position.distanceTo(ballPos);
    float distToHuman = position.distanceTo(humanPos);

    // Activar tornado aleatoriamente cuando no tiene el balón
    spinCooldown_ -= dt;
    if (!hasBall_ && spinCooldown_ <= 0.f &&
        distToHuman < 200.f && agent_->getDifficultyLevel() > 0.3f) {
        isSpinning_   = true;
        spinTimer_    = SPIN_DURATION;
        spinCooldown_ = SPIN_COOLDOWN_MIN +
                        std::sin(orbitTime_ * 7.3f) * 2.f; // pseudo-random
        currentAnim_ = SpriteManager::AnimState::SPIN;
        state_ = EnemyState::INTERCEPTING;
        AudioManager::instance().playTazSpin();   // empieza a girar (tornado)
        return;
    }

    if (distToBall < 80.f || distToHuman < 60.f) {
        state_ = EnemyState::INTERCEPTING;
        Vec2D target = (distToBall < distToHuman) ? ballPos : humanPos;
        Vec2D dir    = (target - position).normalized();
        float speed  = agent_->getMaxSpeed() * 1.2f;
        velocity     = agent_->act(dir * speed);
        Vec2D orbitVel = PhysicsEngine::circularVelocity(
            orbitRadius_, omega_, phi0_, orbitTime_);
        velocity = velocity + orbitVel * 0.3f;
        setPosition(PhysicsEngine::clampToField(
            position + velocity * dt, minX_, maxX_, minY_, maxY_));
        currentAnim_ = SpriteManager::AnimState::RUN;
    } else {
        state_ = EnemyState::ORBITING;
        Vec2D newPos = PhysicsEngine::circularPosition(
            orbitCenter_, orbitRadius_, omega_, phi0_, orbitTime_);
        velocity = PhysicsEngine::circularVelocity(
            orbitRadius_, omega_, phi0_, orbitTime_);
        setPosition(PhysicsEngine::clampToField(newPos, minX_, maxX_, minY_, maxY_));
        currentAnim_ = SpriteManager::AnimState::RUN;
    }
}

// ─── BUGS / DAFFY ─────────────────────────────────────────────────────────────
void EnemyPlayer::updateFieldPlayer(Vec2D ballPos, Vec2D ballVel,
                                     Vec2D humanPos, bool isShooting,
                                     Vec2D goalCenter, float dt)
{
    (void)ballVel; (void)humanPos; (void)isShooting;

    Vec2D desired    = agent_->reason(position, goalCenter, dt);
    velocity         = agent_->act(desired);

    Vec2D newPos = PhysicsEngine::clampToField(
        position + velocity * dt, minX_, maxX_, minY_, maxY_);
    setPosition(newPos);

    float distToBall = position.distanceTo(ballPos);
    currentAnim_ = (velocity.lengthSq() > 400.f)
                   ? SpriteManager::AnimState::RUN
                   : (distToBall < 150.f ? SpriteManager::AnimState::DEFEND
                                         : SpriteManager::AnimState::IDLE);
}

// ─── Movimiento ofensivo / persecución controlado por la escena ───────────────
void EnemyPlayer::steerTo(Vec2D target, float speed, float dt) {
    if (!active || isDizzy_) return;

    Vec2D dir = target - position;
    float dist = dir.length();
    if (dist > 2.f) {
        velocity = dir.normalized() * speed;
    } else {
        velocity = Vec2D::zero();
    }

    Vec2D newPos = PhysicsEngine::clampToField(
        position + velocity * dt, minX_, maxX_, minY_, maxY_);
    setPosition(newPos);

    if (!isSpinning_) {
        currentAnim_ = (velocity.lengthSq() > 200.f)
                       ? SpriteManager::AnimState::RUN
                       : SpriteManager::AnimState::IDLE;
    }
}

void EnemyPlayer::updateDifficulty(float gameTimeSeconds) {
    agent_->updateDifficulty(gameTimeSeconds);
    chaseSpeed_ = 120.f + 100.f * agent_->getDifficultyLevel();
}

float EnemyPlayer::getMass() const {
    switch (type_) {
    case EnemyType::TAZMANIA:   return 120.f;
    case EnemyType::BUGS_BUNNY: return 60.f;
    case EnemyType::DAFFY_DUCK: return 65.f;
    }
    return 70.f;
}

void EnemyPlayer::onCollision(Collidable* other, Vec2D normal) {
    (void)other; (void)normal;
}

void EnemyPlayer::takeBall(Ball* b) {
    hasBall_  = true;
    heldBall_ = b;
    currentAnim_ = SpriteManager::AnimState::SHOOT;
}

Ball* EnemyPlayer::dropBall() {
    hasBall_  = false;
    Ball* b   = heldBall_;
    heldBall_ = nullptr;
    currentAnim_ = SpriteManager::AnimState::IDLE;
    return b;
}

// ── PAINT ─────────────────────────────────────────────────────────────────────
void EnemyPlayer::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    SpriteManager& sm = SpriteManager::instance();
    QSize sprSize = (type_ == EnemyType::TAZMANIA) ? QSize(80, 90) : QSize(68, 85);

    // Tazmania siempre mira hacia la izquierda (hacia el arco humano)
    bool flipH = (velocity.x > 10.f);

    QPixmap frame = sm.getFrame(spriteKey(), currentAnim_,
                                animFrame_, sprSize, flipH);
    p->drawPixmap(-sprSize.width() / 2, -sprSize.height() / 2, frame);

    // Efecto de espiral sobre Tazmania cuando está mareado
    if (isDizzy_) {
        p->setPen(QPen(QColor(255, 220, 0, 180), 1.5f));
        p->setBrush(Qt::NoBrush);
        float r = 28.f + std::sin(dizzyTimer_ * 8.f) * 4.f;
        p->drawEllipse(QPointF(0, -sprSize.height()/2 - 10), r * 0.6f, r * 0.3f);
        p->drawEllipse(QPointF(0, -sprSize.height()/2 - 18), r * 0.4f, r * 0.2f);
    }
}

QString EnemyPlayer::spriteKey() const {
    switch (type_) {
    case EnemyType::TAZMANIA:   return "taz";
    case EnemyType::BUGS_BUNNY: return "bugs";
    case EnemyType::DAFFY_DUCK: return "gk_enemy";
    }
    return "taz";
}
