#include "GoalkeeperAI.h"
#include "PhysicsEngine.h"
#include <QPainter>
#include <cmath>
#include <algorithm>

GoalkeeperAI::GoalkeeperAI(Vec2D pos, Team team,
                             float goalLeft, float goalRight,
                             float fieldW, float fieldH)
    : GameEntity(pos, 20.f)
    , team_(team)
    , goalLeft_(goalLeft)
    , goalRight_(goalRight)
    , goalCenterX_((goalLeft + goalRight) * 0.5f)
    , homeY_(pos.y)
    , agent_(std::make_unique<AIAgent>(AIAgent::AgentType::GOALKEEPER, fieldW, fieldH))
{
    setZValue(6);
}

void GoalkeeperAI::update(float dt) {
    if (!active) return;

    // Animación
    animTimer_ += dt;
    if (animTimer_ >= 0.15f) {
        animTimer_ = 0.f;
        animFrame_ = (animFrame_ + 1) % 4;
    }

    if (saveTimer_ > 0.f) {
        saveTimer_ -= dt;
        if (saveTimer_ <= 0.f) isSaving_ = false;
    }

    setPos(position.x, position.y);
}

void GoalkeeperAI::updateAI(Vec2D ballPos, Vec2D ballVel,
                              Vec2D shooterPos, bool isShooting, float dt)
{
    if (!active) return;

    // PERCEPCIÓN
    agent_->perceive(ballPos, ballVel, shooterPos, isShooting);

    // RAZONAMIENTO
    Vec2D goalCenter = {goalCenterX_, homeY_};
    Vec2D desired = agent_->reason(position, goalCenter, dt);

    // ACCIÓN — solo moverse en X dentro del arco
    Vec2D effectiveVel = agent_->act(desired);
    effectiveVel.y = 0.f; // Movimiento solo lateral

    Vec2D newPos = position + effectiveVel * dt;

    // Limitar al área del arco (+- margen)
    float margin = collRadius;
    newPos.x = std::max(goalLeft_  - margin, std::min(goalRight_ + margin, newPos.x));
    newPos.y = homeY_; // Sin movimiento vertical

    velocity = effectiveVel;
    setPosition(newPos);
}

void GoalkeeperAI::notifyGoalScored(float ballImpactX) {
    agent_->learnFromGoal(ballImpactX, goalLeft_, goalRight_);

    // También sube dificultad directamente al recibir gol
    float currentDiff = agent_->getDifficultyLevel();
    agent_->updateDifficulty(currentDiff * 200.f + 30.f); // Simula más tiempo
}

void GoalkeeperAI::notifySave() {
    isSaving_  = true;
    saveTimer_ = 0.5f;
    agent_->learnFromSave(agent_->predictedZone());
}

void GoalkeeperAI::updateDifficulty(float gameTimeSeconds) {
    agent_->updateDifficulty(gameTimeSeconds);
}

void GoalkeeperAI::onCollision(Collidable* other, Vec2D normal) {
    (void)other; (void)normal;
    isSaving_  = true;
    saveTimer_ = 0.3f;
}

// ─────────────────────────────────────────────────────────────────────────────
// PINTURA
// ─────────────────────────────────────────────────────────────────────────────
void GoalkeeperAI::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    if (team_ == Team::HUMAN) {
        drawGoalkeeper(p);
    } else {
        drawEnemyGoalkeeper(p);
    }
}

void GoalkeeperAI::drawGoalkeeper(QPainter* p) {
    // Arquero humano: Mathias Gidsel — camiseta amarilla/verde portero
    float armSwing = isSaving_ ? 25.f : 5.f * std::sin(animFrame_ * 1.57f);

    // Sombra
    p->setBrush(QColor(0,0,0,50));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 20), 16, 5);

    // Piernas
    p->setBrush(QColor(30, 80, 30));
    p->drawRoundedRect(QRectF(-9, 10, 8, 18), 4, 4);
    p->drawRoundedRect(QRectF(1,  10, 8, 18), 4, 4);

    // Cuerpo camiseta portero (verde/amarillo)
    QLinearGradient bodyGrad(-12, -5, 12, 15);
    bodyGrad.setColorAt(0, QColor(200, 220, 40));
    bodyGrad.setColorAt(1, QColor(130, 170, 20));
    p->setBrush(bodyGrad);
    p->setPen(QPen(QColor(100, 130, 10), 1));
    p->drawRoundedRect(QRectF(-13, -5, 26, 20), 6, 6);

    // Guantes extendidos si está atajando
    p->setBrush(QColor(255, 200, 0));
    p->setPen(QPen(QColor(180, 140, 0), 1));
    p->drawEllipse(QPointF(-16, 0 - armSwing * 0.3f), 7, 7);  // Guante izq
    p->drawEllipse(QPointF(16,  0 + armSwing * 0.3f), 7, 7);  // Guante der

    // Cabeza
    p->setBrush(QColor(255, 210, 170));
    p->setPen(QPen(QColor(200, 160, 120), 1));
    p->drawEllipse(QPointF(0, -17), 12, 12);

    // Casco portero (amarillo)
    p->setBrush(QColor(220, 200, 0));
    p->setPen(Qt::NoPen);
    p->drawChord(QRectF(-12, -29, 24, 24), 0, 180 * 16);

    // Visera
    p->setBrush(QColor(180, 160, 0));
    p->drawRect(QRectF(-13, -18, 26, 4));

    // Ojos
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(-4, -18), 2, 2);
    p->drawEllipse(QPointF(4,  -18), 2, 2);

    // Indicador de dificultad (barra bajo el arquero)
    float diff = agent_->getDifficultyLevel();
    p->setBrush(QColor(255 * (1.f - diff), 255 * diff, 0, 160));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-20, 28, 40 * diff, 5), 2, 2);
    p->setPen(QColor(200, 200, 200, 120));
    p->setFont(QFont("Arial", 5));
    p->drawText(QRectF(-20, 28, 40, 5), Qt::AlignCenter,
                QString("IA %1%").arg(int(diff * 100)));
}

void GoalkeeperAI::drawEnemyGoalkeeper(QPainter* p) {
    // Arquero rival — Speedy González con camiseta LT
    float armSwing = isSaving_ ? 22.f : 4.f * std::sin(animFrame_ * 1.57f);

    p->setBrush(QColor(0,0,0,50));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(2, 20), 16, 5);

    // Piernas
    p->setBrush(QColor(200, 160, 50));
    p->drawRoundedRect(QRectF(-8, 10, 7, 16), 3, 3);
    p->drawRoundedRect(QRectF(1,  10, 7, 16), 3, 3);

    // Cuerpo portero rojo
    QLinearGradient bodyGrad(-12, -5, 12, 15);
    bodyGrad.setColorAt(0, QColor(220, 40, 40));
    bodyGrad.setColorAt(1, QColor(160, 20, 20));
    p->setBrush(bodyGrad);
    p->setPen(QPen(QColor(120, 10, 10), 1));
    p->drawRoundedRect(QRectF(-12, -5, 24, 19), 5, 5);

    // Guantes rojos
    p->setBrush(QColor(255, 80, 80));
    p->setPen(QPen(QColor(180, 30, 30), 1));
    p->drawEllipse(QPointF(-16, 0 - armSwing * 0.3f), 6, 6);
    p->drawEllipse(QPointF(16,  0 + armSwing * 0.3f), 6, 6);

    // Sombrero mexicano (Speedy) — icónico
    p->setBrush(QColor(240, 220, 60));
    p->setPen(QPen(QColor(180, 150, 20), 1.5f));
    // Ala del sombrero
    p->drawEllipse(QPointF(0, -20), 18, 4);
    // Copa del sombrero
    p->drawRoundedRect(QRectF(-8, -35, 16, 17), 4, 4);
    // Banda roja
    p->setBrush(QColor(220, 40, 40));
    p->setPen(Qt::NoPen);
    p->drawRect(QRectF(-8, -25, 16, 4));

    // Cara marrón clara (ratón)
    p->setBrush(QColor(220, 180, 120));
    p->setPen(QPen(QColor(160, 120, 70), 1));
    p->drawEllipse(QPointF(0, -14), 10, 10);

    // Bigote y nariz
    p->setBrush(QColor(100, 70, 30));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(0, -11), 4, 3);
    // Bigotes
    p->setPen(QPen(QColor(100, 70, 30), 1));
    p->drawLine(QPointF(-4, -11), QPointF(-14, -9));
    p->drawLine(QPointF(4,  -11), QPointF(14,  -9));

    // Ojos
    p->setBrush(Qt::black);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(-3.5f, -17), 2, 2);
    p->drawEllipse(QPointF(3.5f,  -17), 2, 2);

    // Indicador de dificultad
    float diff = agent_->getDifficultyLevel();
    p->setBrush(QColor(255 * (1.f - diff), 255 * diff, 0, 160));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(-20, 28, 40 * diff, 5), 2, 2);
    p->setPen(QColor(200, 200, 200, 120));
    p->setFont(QFont("Arial", 5));
    p->drawText(QRectF(-20, 28, 40, 5), Qt::AlignCenter,
                QString("IA %1%").arg(int(diff * 100)));
}
