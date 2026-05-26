#pragma once
#include "Vec2D.h"
#include <vector>
#include <array>
#include <map>

/**
 * @brief Agente Inteligente con aprendizaje adaptativo.
 *
 *  Arquitectura PRAL (Percepción → Razonamiento → Acción → Aprendizaje):
 *
 *  PERCEPCIÓN   : recibe posición del balón, jugador y dirección de ataque.
 *  RAZONAMIENTO : calcula zona probable de impacto; evalúa historial.
 *  ACCIÓN       : devuelve la velocidad/dirección deseada del agente.
 *  APRENDIZAJE  : actualiza tabla de frecuencias de zona de gol recibido.
 *
 *  El nivel de dificultad comienza bajo y crece con el tiempo de juego
 *  y con la cantidad de goles recibidos → partida empieza fácil, se pone
 *  más difícil de forma orgánica.
 *
 *  Contenedor STL usado: std::array (zonas de tiro, tamaño fijo),
 *                        std::map   (historial de lanzamientos por jugador)
 */
class AIAgent {
public:
    // Zonas del arco: IZQUIERDA, CENTRO, DERECHA
    enum class GoalZone { LEFT = 0, CENTER = 1, RIGHT = 2, TOTAL = 3 };

    // Tipo de agente (arquero o defensa de campo)
    enum class AgentType { GOALKEEPER, FIELD_DEFENDER };

    // ── Constructor ───────────────────────────────────────────────────────────
    /**
     * @param type       Tipo de agente.
     * @param fieldW     Ancho del campo (px) — para normalizar posiciones.
     * @param fieldH     Alto del campo (px).
     */
    explicit AIAgent(AgentType type, float fieldW, float fieldH);

    // ── PERCEPCIÓN ────────────────────────────────────────────────────────────
    /**
     * Actualiza la percepción del agente.
     * @param ballPos       Posición actual del balón.
     * @param ballVel       Velocidad actual del balón.
     * @param playerPos     Posición del jugador que tiene el balón.
     * @param isShooting    true si el jugador acaba de lanzar.
     */
    void perceive(Vec2D ballPos, Vec2D ballVel,
                  Vec2D playerPos, bool isShooting);

    // ── RAZONAMIENTO ──────────────────────────────────────────────────────────
    /**
     * Evalúa la situación y calcula la acción óptima.
     * @param myPos         Posición actual del agente.
     * @param goalCenter    Centro del arco que defiende (px).
     * @param dt            Tiempo del frame (s).
     * @return Velocidad deseada para el agente.
     */
    Vec2D reason(Vec2D myPos, Vec2D goalCenter, float dt);

    // ── ACCIÓN ────────────────────────────────────────────────────────────────
    /**
     * Aplica la velocidad calculada, respetando el speed cap actual.
     * @return Velocidad efectiva después de aplicar límites de dificultad.
     */
    Vec2D act(Vec2D desiredVelocity);

    // ── APRENDIZAJE ───────────────────────────────────────────────────────────
    /**
     * Registra un gol recibido para ajustar la estrategia futura.
     * @param ballImpactX   Coordenada X donde entró el balón.
     * @param goalLeft      X del poste izquierdo del arco.
     * @param goalRight     X del poste derecho del arco.
     */
    void learnFromGoal(float ballImpactX, float goalLeft, float goalRight);

    /**
     * Registra un lanzamiento interceptado (refuerzo positivo).
     */
    void learnFromSave(GoalZone zone);

    // ── Dificultad dinámica ───────────────────────────────────────────────────
    /**
     * Incrementa la dificultad según el tiempo de juego transcurrido.
     * Llamado periódicamente por GameManager.
     * @param gameTimeSeconds  Tiempo total del partido en segundos.
     */
    void updateDifficulty(float gameTimeSeconds);

    /** Retorna velocidad máxima actual del agente (px/s). */
    float getMaxSpeed() const { return currentMaxSpeed_; }

    /** Retorna el nivel de dificultad actual [0.0 – 1.0]. */
    float getDifficultyLevel() const { return difficultyLevel_; }

    // ── Utilidades ────────────────────────────────────────────────────────────
    GoalZone predictedZone() const { return predictedZone_; }

private:
    AgentType  type_;
    float      fieldW_, fieldH_;

    // Percepción
    Vec2D  perceivedBallPos_;
    Vec2D  perceivedBallVel_;
    Vec2D  perceivedPlayerPos_;
    bool   playerIsShooting_  = false;

    // Aprendizaje: conteo de goles por zona (índice = GoalZone)
    std::array<int, 3> goalsByZone_ = {0, 0, 0};  // LEFT, CENTER, RIGHT
    std::array<int, 3> savesByZone_ = {0, 0, 0};

    // Razonamiento
    GoalZone predictedZone_ = GoalZone::CENTER;
    Vec2D    targetPos_;

    // Dificultad dinámica
    float difficultyLevel_  = 0.15f;  // Empieza en 15% (fácil)
    float currentMaxSpeed_  = 80.f;   // px/s inicial (lento)
    float reactionDelay_    = 0.5f;   // Demora de reacción inicial (s)
    float reactionTimer_    = 0.f;

    // Parámetros máximos (dificultad 1.0)
    static constexpr float MAX_SPEED_HARD   = 280.f;
    static constexpr float MAX_SPEED_EASY   = 80.f;
    static constexpr float MAX_DELAY_EASY   = 0.6f;
    static constexpr float MIN_DELAY_HARD   = 0.05f;

    // Calcula la zona del arco basado en posición de impacto
    GoalZone classifyZone(float x, float goalLeft, float goalRight) const;

    // Calcula la zona más probable basada en historial
    GoalZone getMostLikelyZone() const;

    // Predice punto de impacto del balón en el eje Y del arco
    float predictBallImpactX(Vec2D goalCenter) const;
};
