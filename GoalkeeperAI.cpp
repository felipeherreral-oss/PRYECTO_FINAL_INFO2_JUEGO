#include "GoalkeeperAI.h"
#include "PhysicsEngine.h"
#include "SpriteManager.h"
#include <QPainter>
#include <cmath>
#include <algorithm>

GoalkeeperAI::GoalkeeperAI(Vec2D pos, Team team,
                             float goalTopY, float goalBotY,
                             float fieldW, float fieldH)
    : GameEntity(pos, 20.f)
    , team_(team)
    , fixedX_(pos.x)
    , goalTop_(goalTopY)
    , goalBot_(goalBotY)
    , goalCenterY_((goalTopY + goalBotY) * 0.5f)
    , agent_(std::make_unique<AIAgent>(AIAgent::AgentType::GOALKEEPER, fieldW, fieldH))
{
    setZValue(6);
}

void GoalkeeperAI::update(float dt) {
    if (!active) return;

    animTimer_ += dt;
    float speed = (isSaving_) ? 12.f : 5.f;
    if (animTimer_ >= 1.f / speed) {
        animTimer_ = 0.f;
        QString key = (team_ == Team::HUMAN) ? "gk_human" : "gk_enemy";
        animFrame_ = (animFrame_ + 1) %
            std::max(1, SpriteManager::instance().frameCount(key, currentAnim_));
    }

    if (saveTimer_ > 0.f) {
        saveTimer_ -= dt;
        if (saveTimer_ <= 0.f) {
            isSaving_    = false;
            currentAnim_ = SpriteManager::AnimState::IDLE;
        }
    }

    setPos(position.x, position.y);
}

// ─────────────────────────────────────────────────────────────────────────────
// IA: el arquero se mueve VERTICALMENTE. Sigue la Y del balón; si viene un tiro
// hacia su arco, predice el punto de impacto. Añade un pequeño sesgo aprendido.
// ─────────────────────────────────────────────────────────────────────────────
void GoalkeeperAI::updateAI(Vec2D ballPos, Vec2D ballVel,
                              Vec2D shooterPos, bool isShooting, float dt) {
    if (!active) return;

    // PERCEPCIÓN
    agent_->perceive(ballPos, ballVel, shooterPos, isShooting);

    // RAZONAMIENTO ───────────────────────────────────────────────────────────
    // ¿El balón viene hacia mi arco? (humano = izquierda, rival = derecha)
    bool comingToMe = (team_ == Team::HUMAN) ? (ballVel.x < -5.f)
                                             : (ballVel.x >  5.f);

    float targetY = ballPos.y;  // por defecto, seguir la Y del balón

    if (comingToMe && std::abs(ballVel.x) > 1.f) {
        // Predecir Y de impacto en el plano del arquero
        float t = (fixedX_ - ballPos.x) / ballVel.x;
        if (t > 0.f && t < 3.f) {
            targetY = ballPos.y + ballVel.y * t;
        }
    }

    // Sesgo por aprendizaje: cubrir antes la zona donde más le anotan
    float bias    = agent_->learnedZoneBias();   // [-1, 1]
    float diff    = agent_->getDifficultyLevel();
    float w       = std::min(0.4f, diff * 0.5f);
    float biasedY = goalCenterY_ + bias * (goalBot_ - goalCenterY_) * 0.6f;
    targetY       = targetY * (1.f - w) + biasedY * w;

    // Limitar a la boca del arco (con un pequeño margen)
    float margin = collRadius * 0.5f;
    targetY = std::max(goalTop_ + margin, std::min(goalBot_ - margin, targetY));

    // ACCIÓN: moverse hacia targetY con la velocidad permitida por la dificultad
    float maxV = agent_->getMaxSpeed();
    float dy   = targetY - position.y;
    float vy   = 0.f;
    if (std::abs(dy) > 1.f) {
        vy = std::max(-maxV, std::min(maxV, dy / std::max(dt, 0.001f)));
        if (vy >  maxV) vy =  maxV;
        if (vy < -maxV) vy = -maxV;
    }

    Vec2D newPos = {fixedX_, position.y + vy * dt};
    newPos.y = std::max(goalTop_ + margin, std::min(goalBot_ - margin, newPos.y));
    velocity = {0.f, vy};
    setPosition(newPos);

    if (!isSaving_) {
        currentAnim_ = (std::abs(vy) > 25.f)
                       ? SpriteManager::AnimState::RUN
                       : SpriteManager::AnimState::IDLE;
    }
}

void GoalkeeperAI::notifyGoalScored(float ballImpactY) {
    // APRENDIZAJE: clasifica la zona vertical donde le anotaron
    agent_->learnFromGoal(ballImpactY, goalTop_, goalBot_);
}

void GoalkeeperAI::notifySave() {
    isSaving_    = true;
    saveTimer_   = 0.6f;
    currentAnim_ = SpriteManager::AnimState::SAVE;
    agent_->learnFromSave(agent_->predictedZone());
}

void GoalkeeperAI::updateDifficulty(float gameTimeSeconds) {
    agent_->updateDifficulty(gameTimeSeconds);
}

void GoalkeeperAI::onCollision(Collidable* other, Vec2D normal) {
    (void)other; (void)normal;
    isSaving_    = true;
    saveTimer_   = 0.5f;
    currentAnim_ = SpriteManager::AnimState::SAVE;
}

// ── PAINT ─────────────────────────────────────────────────────────────────────
void GoalkeeperAI::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    SpriteManager& sm   = SpriteManager::instance();
    QString key         = (team_ == Team::HUMAN) ? "gk_human" : "gk_enemy";
    QSize   sprSize     = (team_ == Team::HUMAN) ? QSize(64, 86) : QSize(60, 80);

    // El arquero humano (izquierda) mira a la derecha; el rival (derecha) mira a la izquierda
    bool flipH = (team_ == Team::ENEMY);

    QPixmap frame = sm.getFrame(key, currentAnim_, animFrame_, sprSize, flipH);
    p->drawPixmap(-sprSize.width() / 2, -sprSize.height() / 2, frame);

    // Etiqueta GK
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 6, QFont::Bold));
    QRectF label(-12, sprSize.height()/2 - 3, 24, 10);
    p->setBrush(QColor(0, 0, 0, 160));
    p->drawRoundedRect(label, 3, 3);
    p->drawText(label, Qt::AlignCenter, "GK");
}
