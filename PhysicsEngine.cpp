#include "PhysicsEngine.h"
#include <cmath>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// 1. MOVIMIENTO CIRCULAR UNIFORME
// ─────────────────────────────────────────────────────────────────────────────
Vec2D PhysicsEngine::circularPosition(Vec2D center, float radius,
                                       float omega,  float phi0, float t) {
    if (radius < 0.f)
        throw PhysicsException("El radio de la órbita no puede ser negativo.");

    return {
        center.x + radius * std::cos(omega * t + phi0),
        center.y + radius * std::sin(omega * t + phi0)
    };
}

Vec2D PhysicsEngine::circularVelocity(float radius, float omega,
                                       float phi0, float t) {
    // Derivada de la posición respecto al tiempo:
    // vx = -R·ω·sin(ω·t + φ₀)
    // vy =  R·ω·cos(ω·t + φ₀)
    return {
        -radius * omega * std::sin(omega * t + phi0),
         radius * omega * std::cos(omega * t + phi0)
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. COLISIÓN ELÁSTICA 2D
// ─────────────────────────────────────────────────────────────────────────────
PhysicsEngine::ElasticResult PhysicsEngine::elasticCollision2D(
        Vec2D v1, float m1,
        Vec2D v2, float m2,
        Vec2D normal)
{
    float totalMass = m1 + m2;
    if (totalMass < 1e-6f)
        throw PhysicsException("La masa total en colisión elástica no puede ser cero.");

    // Componente relativa en la dirección normal
    Vec2D dv = v1 - v2;
    float dvDotN = dv.dot(normal);

    // Si los objetos ya se alejan, no hacemos nada
    if (dvDotN > 0.f) {
        return {v1, v2};
    }

    float impulse = (2.f * dvDotN) / totalMass;

    Vec2D v1After = v1 - normal * (impulse * m2);
    Vec2D v2After = v2 + normal * (impulse * m1);

    return {v1After, v2After};
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. INTEGRACIÓN DE VERLET
// ─────────────────────────────────────────────────────────────────────────────
Vec2D PhysicsEngine::verletPosition(Vec2D pos, Vec2D vel, Vec2D accel, float dt) {
    if (dt < 0.f)
        throw PhysicsException("dt no puede ser negativo en integración Verlet.");
    return pos + vel * dt + accel * (0.5f * dt * dt);
}

Vec2D PhysicsEngine::verletVelocity(Vec2D vel, Vec2D accel, float dt) {
    return vel + accel * dt;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilidades
// ─────────────────────────────────────────────────────────────────────────────
Vec2D PhysicsEngine::applyFriction(Vec2D vel, float coeff, float dt) {
    // Reducción exponencial: v' = v · (1 - coeff·dt)
    float factor = 1.f - coeff * dt;
    if (factor < 0.f) factor = 0.f;
    return vel * factor;
}

Vec2D PhysicsEngine::reflect(Vec2D vel, Vec2D normal) {
    // r = v - 2(v·n̂)n̂
    float dot = vel.dot(normal);
    return vel - normal * (2.f * dot);
}

Vec2D PhysicsEngine::clampToField(Vec2D pos, float minX, float maxX,
                                   float minY, float maxY) {
    Vec2D clamped = pos;
    if (clamped.x < minX) clamped.x = minX;
    if (clamped.x > maxX) clamped.x = maxX;
    if (clamped.y < minY) clamped.y = minY;
    if (clamped.y > maxY) clamped.y = maxY;
    return clamped;
}
