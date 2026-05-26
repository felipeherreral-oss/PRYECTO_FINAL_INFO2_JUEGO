#pragma once
#include <stdexcept>
#include <string>

/**
 * @brief Excepciones personalizadas del juego.
 *        Usadas para errores de inicialización, recursos y estado inválido.
 */

class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& msg)
        : std::runtime_error("[GameException] " + msg) {}
};

/** Se lanza cuando un recurso gráfico o de audio no se puede cargar */
class ResourceLoadException : public GameException {
public:
    explicit ResourceLoadException(const std::string& resourcePath)
        : GameException("No se pudo cargar el recurso: " + resourcePath) {}
};

/** Se lanza cuando el estado del juego es inválido (p. ej. puntero nulo) */
class InvalidGameStateException : public GameException {
public:
    explicit InvalidGameStateException(const std::string& detail)
        : GameException("Estado inválido del juego: " + detail) {}
};

/** Se lanza cuando se intenta una operación fuera del campo */
class OutOfBoundsException : public GameException {
public:
    explicit OutOfBoundsException(const std::string& entity)
        : GameException(entity + " salió de los límites del campo.") {}
};

/** Se lanza cuando los parámetros de física son inconsistentes */
class PhysicsException : public GameException {
public:
    explicit PhysicsException(const std::string& detail)
        : GameException("Error en motor de físicas: " + detail) {}
};
