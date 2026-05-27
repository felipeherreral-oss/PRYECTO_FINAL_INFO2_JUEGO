#include "EnemyPlayer.h"
#include "Ball.h"
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
{
    setZValue(4);
}

// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::update(float dt) {
    if (!active) return;

    animTimer_ += dt;
    if (animTimer_ >= 0.12f) {
        animTimer_ = 0.f;
        animFrame_ = (animFrame_ + 1) % 4;
    }

    setPos(position.x, position.y);
}

// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::updateAI(Vec2D ballPos, Vec2D ballVel,
                            Vec2D humanPlayerPos, bool humanIsShooting,
                            Vec2D ownGoalCenter, float dt)
{
    if (!active) return;

    // PERCEPCIÓN
    agent_->perceive(ballPos, ballVel, humanPlayerPos, humanIsShooting);

    if (type_ == EnemyType::TAZMANIA) {
        updateTazmania(ballPos, humanPlayerPos, dt);
    } else {
        updateFieldPlayer(ballPos, ballVel, humanPlayerPos,
                          humanIsShooting, ownGoalCenter, dt);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TAZMANIA — Movimiento Circular Uniforme + detección de balón cercano
// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::updateTazmania(Vec2D ballPos, Vec2D humanPos, float dt) {
    orbitTime_ += dt;

    float distToBall = position.distanceTo(ballPos);
    float distToHuman = position.distanceTo(humanPos);

    if (distToBall < 80.f || distToHuman < 60.f) {
        // Intercepción: sale de la órbita y se lanza hacia el objetivo
        state_ = EnemyState::INTERCEPTING;
        Vec2D target = (distToBall < distToHuman) ? ballPos : humanPos;
        Vec2D dir = (target - position).normalized();
        float speed = agent_->getMaxSpeed() * 1.2f;
        Vec2D desired = dir * speed;
        velocity = agent_->act(desired);

        Vec2D newPos = position + velocity * dt;
        setPosition(newPos);

        // Velocidad angular en órbita (para colisión elástica)
        // La velocidad de la órbita en el punto actual
        Vec2D orbitVel = PhysicsEngine::circularVelocity(orbitRadius_, omega_, phi0_, orbitTime_);
        velocity = velocity + orbitVel * 0.3f; // Mezcla de órbita e intercepción
    } else {
        // MCU puro
        state_ = EnemyState::ORBITING;
        Vec2D newPos = PhysicsEngine::circularPosition(
            orbitCenter_, orbitRadius_, omega_, phi0_, orbitTime_);
        velocity = PhysicsEngine::circularVelocity(orbitRadius_, omega_, phi0_, orbitTime_);
        setPosition(newPos);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BUGS / DAFFY — Defensa posicional con AIAgent
// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::updateFieldPlayer(Vec2D ballPos, Vec2D ballVel,
                                     Vec2D humanPos, bool isShooting,
                                     Vec2D goalCenter, float dt)
{
    // RAZONAMIENTO
    Vec2D desired = agent_->reason(position, goalCenter, dt);
    // ACCIÓN
    velocity = agent_->act(desired);

    Vec2D newPos = position + velocity * dt;

    // Mantener en su mitad del campo
    newPos = PhysicsEngine::clampToField(newPos,
        goalCenter.x - 500.f, goalCenter.x + 50.f,
        50.f, 620.f);

    setPosition(newPos);
}

// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::updateDifficulty(float gameTimeSeconds) {
    agent_->updateDifficulty(gameTimeSeconds);
    // Actualizar velocidad de persecución también
    chaseSpeed_ = 120.f + 100.f * agent_->getDifficultyLevel();
}

// ─────────────────────────────────────────────────────────────────────────────
float EnemyPlayer::getMass() const {
    switch (type_) {
    case EnemyType::TAZMANIA:    return 120.f; // Tazmania es más pesado
    case EnemyType::BUGS_BUNNY:  return 60.f;
    case EnemyType::DAFFY_DUCK:  return 65.f;
    }
    return 70.f;
}

void EnemyPlayer::onCollision(Collidable* other, Vec2D normal) {
    (void)other; (void)normal;
    // La resolución de colisión elástica la hace Level2Scene
}

void EnemyPlayer::takeBall(Ball* b) {
    hasBall_ = true;
    heldBall_ = b;
}

Ball* EnemyPlayer::dropBall() {
    hasBall_ = false;
    Ball* b = heldBall_;
    heldBall_ = nullptr;
    return b;
}

// ─────────────────────────────────────────────────────────────────────────────
// PINTURA
// ─────────────────────────────────────────────────────────────────────────────
void EnemyPlayer::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    switch (type_) {
    case EnemyType::TAZMANIA:   drawTazmania(p);  break;
    case EnemyType::BUGS_BUNNY: drawBugsBunny(p); break;
    case EnemyType::DAFFY_DUCK: drawDaffyDuck(p); break;
    }
}

void EnemyPlayer::drawTazmania(QPainter* p) {
    // Cuerpo marrón grande y redondo del Tazmania
    float legSwing = std::sin(animFrame_ * 1.57f) * 8.f;

    // Sombra
    p->setBrush(QColor(0,0,0,60));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 20), 18, 6);

    // Piernas
    p->setBrush(QColor(100, 60, 20));
    p->drawRoundedRect(QRectF(-12 + legSwing, 12, 10, 16), 4, 4);
    p->drawRoundedRect(QRectF(2   - legSwing, 12, 10, 16), 4, 4);

    // Cuerpo principal (marrón grisáceo)
    QRadialGradient bodyGrad(-5, -5, 28);
    bodyGrad.setColorAt(0, QColor(140, 100, 60));
    bodyGrad.setColorAt(1, QColor(80, 50, 20));
    p->setBrush(bodyGrad);
    p->setPen(QPen(QColor(60, 30, 10), 1.5f));
    p->drawEllipse(QPointF(0, 2), 22, 20);

    // Panza más clara
    p->setBrush(QColor(200, 170, 120));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(0, 5), 12, 12);

    // Cabeza
    p->setBrush(QColor(130, 90, 50));
    p->setPen(QPen(QColor(60, 30, 10), 1.5f));
    p->drawEllipse(QPointF(0, -18), 16, 15);

    // Mandíbula grande (icónica de Tazmania)
    p->setBrush(QColor(200, 170, 120));
    p->drawChord(QRectF(-14, -12, 28, 18), 200 * 16, 140 * 16);

    // Ojos salvajes
    p->setBrush(Qt::white);
    p->setPen(QPen(Qt::black, 1));
    p->drawEllipse(QPointF(-5, -21), 5, 5);
    p->drawEllipse(QPointF(5,  -21), 5, 5);
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(-4, -21), 3, 3);
    p->drawEllipse(QPointF(6,  -21), 3, 3);

    // Espiral de tormenta cuando está en intercepción
    if (state_ == EnemyState::INTERCEPTING) {
        p->setPen(QPen(QColor(255, 140, 0, 180), 2));
        p->setBrush(Qt::NoBrush);
        for (int i = 0; i < 3; ++i) {
            float r = 28.f + i * 8.f;
            p->drawEllipse(QPointF(0, 0), r, r * 0.4f);
        }
    }

    // Camiseta de los Looney Tunes (rojo con "LT")
    p->setBrush(QColor(200, 30, 30));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-10, -4, 20, 12), 4, 4);
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 5, QFont::Bold));
    p->drawText(QRectF(-8, -2, 16, 10), Qt::AlignCenter, "LT");
}

void EnemyPlayer::drawBugsBunny(QPainter* p) {
    float legSwing = std::sin(animFrame_ * 1.57f) * 6.f;

    p->setBrush(QColor(0,0,0,50));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 18), 12, 4);

    // Piernas
    p->setBrush(QColor(180, 180, 180));
    p->drawRoundedRect(QRectF(-8 + legSwing, 10, 7, 16), 3, 3);
    p->drawRoundedRect(QRectF(1  - legSwing, 10, 7, 16), 3, 3);

    // Cuerpo gris
    p->setBrush(QColor(200, 200, 200));
    p->setPen(QPen(QColor(150, 150, 150), 1));
    p->drawRoundedRect(QRectF(-11, -4, 22, 18), 5, 5);

    // Camiseta roja LT
    p->setBrush(QColor(200, 30, 30));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-9, -2, 18, 12), 3, 3);
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 5, QFont::Bold));
    p->drawText(QRectF(-7, 0, 14, 8), Qt::AlignCenter, "LT");

    // Cabeza
    p->setBrush(QColor(210, 210, 210));
    p->setPen(QPen(QColor(150,150,150), 1));
    p->drawEllipse(QPointF(0, -14), 11, 11);

    // Orejas largas de conejo
    p->setBrush(QColor(210, 210, 210));
    p->setPen(QPen(QColor(150,150,150), 1));
    p->drawRoundedRect(QRectF(-7, -36, 6, 22), 3, 3);
    p->drawRoundedRect(QRectF(1,  -36, 6, 22), 3, 3);
    // Interior rosado
    p->setBrush(QColor(255, 180, 180));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-5.5f, -35, 3, 19), 2, 2);
    p->drawRoundedRect(QRectF(2.5f,  -35, 3, 19), 2, 2);

    // Ojos
    p->setBrush(Qt::white);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(-4, -15), 3, 3);
    p->drawEllipse(QPointF(4,  -15), 3, 3);
    p->setBrush(QColor(100, 180, 255));
    p->drawEllipse(QPointF(-4, -15), 2, 2);
    p->drawEllipse(QPointF(4,  -15), 2, 2);
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(-4, -15), 1, 1);
    p->drawEllipse(QPointF(4,  -15), 1, 1);

    // Nariz rosada
    p->setBrush(QColor(255, 100, 100));
    p->drawEllipse(QPointF(0, -11), 2, 1.5f);

    // Dientes blancos
    p->setBrush(Qt::white);
    p->drawRoundedRect(QRectF(-3, -9, 2.5f, 4), 1, 1);
    p->drawRoundedRect(QRectF(0.5f, -9, 2.5f, 4), 1, 1);
}

void EnemyPlayer::drawDaffyDuck(QPainter* p) {
    float legSwing = std::sin(animFrame_ * 1.57f) * 6.f;

    p->setBrush(QColor(0,0,0,50));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 18), 12, 4);

    // Piernas
    p->setBrush(QColor(30, 30, 30));
    p->drawRoundedRect(QRectF(-8 + legSwing, 10, 7, 16), 3, 3);
    p->drawRoundedRect(QRectF(1  - legSwing, 10, 7, 16), 3, 3);
    // Pies naranja
    p->setBrush(QColor(255, 140, 0));
    p->drawEllipse(QPointF(-5 + legSwing, 26), 6, 3);
    p->drawEllipse(QPointF(4  - legSwing, 26), 6, 3);

    // Cuerpo negro
    p->setBrush(QColor(30, 30, 30));
    p->setPen(QPen(QColor(10, 10, 10), 1));
    p->drawRoundedRect(QRectF(-11, -4, 22, 18), 5, 5);

    // Camiseta roja LT
    p->setBrush(QColor(200, 30, 30));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-9, -2, 18, 12), 3, 3);
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 5, QFont::Bold));
    p->drawText(QRectF(-7, 0, 14, 8), Qt::AlignCenter, "LT");

    // Cuello blanco (collar)
    p->setBrush(Qt::white);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(0, -6), 8, 4);

    // Cabeza negra
    p->setBrush(QColor(30, 30, 30));
    p->setPen(QPen(QColor(10, 10, 10), 1));
    p->drawEllipse(QPointF(0, -16), 12, 11);

    // Pico naranja (característica de Daffy)
    p->setBrush(QColor(255, 140, 0));
    p->setPen(QPen(QColor(200, 100, 0), 1));
    QPainterPath beak;
    beak.moveTo(-5, -12);
    beak.lineTo(-12, -8);
    beak.lineTo(-5, -5);
    beak.closeSubpath();
    p->drawPath(beak);

    // Ojos amarillos
    p->setBrush(Qt::yellow);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(-3, -19), 4, 4);
    p->drawEllipse(QPointF(5,  -19), 4, 4);
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(-3, -19), 2, 2);
    p->drawEllipse(QPointF(5,  -19), 2, 2);
}
