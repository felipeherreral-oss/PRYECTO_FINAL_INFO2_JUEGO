#include "AIAgent.h"
#include "GameExceptions.h"
#include <algorithm>
#include <cmath>

AIAgent::AIAgent(AgentType type, float fieldW, float fieldH)
    : type_(type), fieldW_(fieldW), fieldH_(fieldH)
{
    if (fieldW <= 0.f || fieldH <= 0.f)
        throw PhysicsException("Dimensiones de campo inválidas para AIAgent.");

    goalsByZone_.fill(0);
    savesByZone_.fill(0);
    difficultyLevel_ = 0.15f;
    currentMaxSpeed_ = MAX_SPEED_EASY;
    reactionDelay_   = MAX_DELAY_EASY;
}

// ─────────────────────────────────────────────────────────────────────────────
// PERCEPCIÓN
// ─────────────────────────────────────────────────────────────────────────────
void AIAgent::perceive(Vec2D ballPos, Vec2D ballVel,
                       Vec2D playerPos, bool isShooting) {
    perceivedBallPos_    = ballPos;
    perceivedBallVel_    = ballVel;
    perceivedPlayerPos_  = playerPos;
    playerIsShooting_    = isShooting;
}

// ─────────────────────────────────────────────────────────────────────────────
// RAZONAMIENTO
// ─────────────────────────────────────────────────────────────────────────────
Vec2D AIAgent::reason(Vec2D myPos, Vec2D goalCenter, float dt) {
    // Timer de reacción: simula demora humana proporcional a la dificultad
    if (reactionTimer_ > 0.f) {
        reactionTimer_ -= dt;
        // Mientras espera, mantiene posición actual
        return Vec2D::zero();
    }

    Vec2D target;

    if (type_ == AgentType::GOALKEEPER) {
        // ── ARQUERO ──────────────────────────────────────────────────────────
        if (playerIsShooting_) {
            // El balón ya viene: predecir zona de impacto
            float impactX = predictBallImpactX(goalCenter);

            // Sesgo por aprendizaje: si hay zona frecuente, cubrir antes
            GoalZone likely = getMostLikelyZone();

            // Ponderar predicción física vs. sesgo histórico
            float learnWeight = std::min(1.f, difficultyLevel_ * 1.5f);

            float biasX = goalCenter.x;
            if (likely == GoalZone::LEFT)   biasX = goalCenter.x - 40.f;
            if (likely == GoalZone::RIGHT)  biasX = goalCenter.x + 40.f;

            float finalX = impactX * (1.f - learnWeight) + biasX * learnWeight;

            target = {finalX, goalCenter.y};
            predictedZone_ = likely;
        } else {
            // Sin tiro: posicionarse según posición del jugador
            // A mayor dificultad, se anticipa más al centro
            float ratio = (perceivedPlayerPos_.x - goalCenter.x) / (fieldW_ * 0.5f);
            ratio = std::max(-1.f, std::min(1.f, ratio));
            float anticipate = 35.f * difficultyLevel_;
            target = {goalCenter.x + ratio * anticipate, goalCenter.y};
        }

        // Reiniciar timer de reacción (modelar demora)
        reactionTimer_ = reactionDelay_ * (1.f - difficultyLevel_ * 0.8f);

    } else {
        // ── DEFENSA DE CAMPO ─────────────────────────────────────────────────
        // Moverse hacia el balón o interceptar trayectoria
        Vec2D toBall = perceivedBallPos_ - myPos;
        float distToBall = toBall.length();

        if (distToBall < 120.f) {
            // Presionar balón
            target = perceivedBallPos_;
        } else {
            // Posición defensiva entre balón y arco propio
            target = (perceivedBallPos_ + goalCenter) * 0.5f;
        }

        reactionTimer_ = reactionDelay_ * 0.5f * (1.f - difficultyLevel_);
    }

    targetPos_ = target;

    // Dirección hacia el objetivo
    Vec2D dir = (targetPos_ - myPos);
    float dist = dir.length();
    if (dist < 2.f) return Vec2D::zero();

    return dir.normalized() * currentMaxSpeed_;
}

// ─────────────────────────────────────────────────────────────────────────────
// ACCIÓN
// ─────────────────────────────────────────────────────────────────────────────
Vec2D AIAgent::act(Vec2D desiredVelocity) {
    float speed = desiredVelocity.length();
    if (speed > currentMaxSpeed_) {
        return desiredVelocity.normalized() * currentMaxSpeed_;
    }
    return desiredVelocity;
}

// ─────────────────────────────────────────────────────────────────────────────
// APRENDIZAJE
// ─────────────────────────────────────────────────────────────────────────────
void AIAgent::learnFromGoal(float ballImpactX, float goalLeft, float goalRight) {
    GoalZone z = classifyZone(ballImpactX, goalLeft, goalRight);
    int idx = static_cast<int>(z);
    goalsByZone_[idx]++;

    // Sube dificultad más rápido si recibe muchos goles por la misma zona
    int maxGoals = *std::max_element(goalsByZone_.begin(), goalsByZone_.end());
    if (maxGoals >= 3) {
        // Aprendizaje acelerado: la IA ya sabe dónde atacas
        difficultyLevel_ = std::min(1.f, difficultyLevel_ + 0.08f);
        currentMaxSpeed_ = MAX_SPEED_EASY + (MAX_SPEED_HARD - MAX_SPEED_EASY) * difficultyLevel_;
        reactionDelay_   = MAX_DELAY_EASY - (MAX_DELAY_EASY - MIN_DELAY_HARD) * difficultyLevel_;
    }
}

void AIAgent::learnFromSave(GoalZone zone) {
    int idx = static_cast<int>(zone);
    if (idx < 3) savesByZone_[idx]++;
}

// ─────────────────────────────────────────────────────────────────────────────
// DIFICULTAD DINÁMICA
// ─────────────────────────────────────────────────────────────────────────────
void AIAgent::updateDifficulty(float gameTimeSeconds) {
    // Curva suave: dificultad crece linealmente durante los primeros 3 minutos
    // y se estabiliza cerca de 0.85 (no llega al 100% para que siempre sea posible ganar)
    constexpr float RAMP_DURATION = 180.f;  // 3 minutos para alcanzar máximo
    float timeFactor = std::min(1.f, gameTimeSeconds / RAMP_DURATION);

    // Base de tiempo (sin aprendizaje)
    float baseDifficulty = 0.15f + 0.70f * timeFactor;

    // El aprendizaje puede agregar hasta 0.15 adicional
    int totalGoals = goalsByZone_[0] + goalsByZone_[1] + goalsByZone_[2];
    float learningBonus = std::min(0.15f, totalGoals * 0.02f);

    difficultyLevel_ = std::min(1.f, baseDifficulty + learningBonus);

    // Actualizar parámetros derivados
    currentMaxSpeed_ = MAX_SPEED_EASY + (MAX_SPEED_HARD - MAX_SPEED_EASY) * difficultyLevel_;
    reactionDelay_   = MAX_DELAY_EASY - (MAX_DELAY_EASY - MIN_DELAY_HARD) * difficultyLevel_;
}

// ─────────────────────────────────────────────────────────────────────────────
// PRIVADOS
// ─────────────────────────────────────────────────────────────────────────────
AIAgent::GoalZone AIAgent::classifyZone(float x, float goalLeft, float goalRight) const {
    float third = (goalRight - goalLeft) / 3.f;
    if (x < goalLeft + third)          return GoalZone::LEFT;
    if (x < goalLeft + 2.f * third)   return GoalZone::CENTER;
    return GoalZone::RIGHT;
}

AIAgent::GoalZone AIAgent::getMostLikelyZone() const {
    // Retorna la zona con más goles recibidos (sesgo de aprendizaje)
    int maxCount = 0;
    GoalZone best = GoalZone::CENTER;
    for (int i = 0; i < 3; ++i) {
        if (goalsByZone_[i] > maxCount) {
            maxCount = goalsByZone_[i];
            best = static_cast<GoalZone>(i);
        }
    }
    return best;
}

float AIAgent::predictBallImpactX(Vec2D goalCenter) const {
    // Predicción lineal: ¿dónde estará el balón en Y del arco?
    Vec2D bPos = perceivedBallPos_;
    Vec2D bVel = perceivedBallVel_;

    if (std::abs(bVel.y) < 1.f) return bPos.x; // Balón casi sin movimiento vertical

    // Tiempo hasta alcanzar Y del arco
    float dy = goalCenter.y - bPos.y;
    float t  = dy / bVel.y;

    if (t < 0.f) return goalCenter.x; // Balón se aleja

    // X predicha (sin deflexión lateral)
    float predictedX = bPos.x + bVel.x * t;

    // Añadir ruido inversamente proporcional a la dificultad (baja dificultad = mucho error)
    float noiseFactor = (1.f - difficultyLevel_) * 60.f;
    // El ruido es determinista basado en posición (no necesitamos rand() aquí)
    float noise = std::sin(bPos.x * 0.1f + bPos.y * 0.07f) * noiseFactor;

    return predictedX + noise;
}
