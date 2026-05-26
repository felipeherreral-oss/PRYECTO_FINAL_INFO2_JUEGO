#pragma once
#include "Vec2D.h"
#include "GameExceptions.h"

/**
 * @brief Motor de físicas del Nivel 2.
 *
 *  Modelos implementados:
 *  1. Movimiento Circular Uniforme (MCU)  — para Tazmania y defensas orbitantes
 *  2. Colisión Elástica 2D               — choque jugador ↔ Tazmania
 *  3. Movimiento de proyectil del balón   — integración de Verlet simple
 *
 *  Cada modelo es una función estática pura → fácil de testear y justificar.
 */
class PhysicsEngine {
public:
    // ─────────────────────────────────────────────────────────────────────────
    // 1. MOVIMIENTO CIRCULAR UNIFORME
    //    x(t) = cx + R·cos(ω·t + φ₀)
    //    y(t) = cy + R·sin(ω·t + φ₀)
    //
    //  Parámetros:
    //    center  — centro de la órbita
    //    radius  — radio de la órbita (px)
    //    omega   — velocidad angular (rad/s). Positivo = sentido antihorario.
    //    phi0    — fase inicial (rad)
    //    t       — tiempo acumulado (s)
    // ─────────────────────────────────────────────────────────────────────────
    static Vec2D circularPosition(Vec2D center, float radius,
                                  float omega,  float phi0, float t);

    /** Velocidad tangencial derivada del MCU (para física de colisión). */
    static Vec2D circularVelocity(float radius, float omega, float phi0, float t);

    // ─────────────────────────────────────────────────────────────────────────
    // 2. COLISIÓN ELÁSTICA 2D (Conservación de momento + energía cinética)
    //
    //    v1' = v1 - (2·m2/(m1+m2)) · [(v1-v2)·n̂] · n̂
    //    v2' = v2 + (2·m1/(m1+m2)) · [(v1-v2)·n̂] · n̂
    //
    //    n̂ — normal unitaria en la línea de centros
    // ─────────────────────────────────────────────────────────────────────────
    struct ElasticResult {
        Vec2D v1After;  // Nueva velocidad del objeto 1
        Vec2D v2After;  // Nueva velocidad del objeto 2
    };

    static ElasticResult elasticCollision2D(
        Vec2D v1, float m1,
        Vec2D v2, float m2,
        Vec2D normal           // Normal unitaria de obj1 → obj2
    );

    // ─────────────────────────────────────────────────────────────────────────
    // 3. INTEGRACIÓN DE VERLET (para el balón y proyectiles)
    //    x(t+dt) = x(t) + v(t)·dt + ½·a·dt²
    //    v(t+dt) = v(t) + a·dt
    // ─────────────────────────────────────────────────────────────────────────
    static Vec2D verletPosition(Vec2D pos, Vec2D vel, Vec2D accel, float dt);
    static Vec2D verletVelocity(Vec2D vel, Vec2D accel, float dt);

    // ─────────────────────────────────────────────────────────────────────────
    // Utilidades
    // ─────────────────────────────────────────────────────────────────────────
    /** Fricción simple: reduce la velocidad cada frame. coeff ∈ [0,1] */
    static Vec2D applyFriction(Vec2D vel, float coeff, float dt);

    /** Refleja vel respecto a la normal n̂ (rebote en pared). */
    static Vec2D reflect(Vec2D vel, Vec2D normal);

    /** Clamp de posición dentro del campo (limites en px). */
    static Vec2D clampToField(Vec2D pos, float minX, float maxX,
                               float minY, float maxY);
};
