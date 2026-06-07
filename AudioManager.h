#pragma once
#include <QString>
#include <map>

class QSoundEffect;

/**
 * @brief Gestor de efectos de sonido del juego (singleton).
 *
 *  Precarga los .wav (PCM) desde los recursos de Qt con QSoundEffect, que
 *  ofrece baja latencia, ideal para efectos cortos disparados por eventos.
 *
 *  Eventos cubiertos:
 *   - Gol de cualquier equipo          → gol_anotado.wav
 *   - Inicio y fin del partido         → silbato_inicial_final.wav
 *   - Tazmania empieza a girar         → giro_tasmania.wav
 *   - Tazmania termina de girar/mareo  → tasmania_despues_del_giro.wav
 *   - Choque contra Tazmania girando   → choque_contra_tasmania.wav
 */
class AudioManager {
public:
    static AudioManager& instance();

    /** Crea los QSoundEffect una sola vez. Llamar tras crear la QApplication. */
    void loadAll();

    /** Permite silenciar todos los efectos. */
    void setEnabled(bool e) { enabled_ = e; }
    bool isEnabled() const   { return enabled_; }

    // ── Eventos del juego ──────────────────────────────────────────────────
    void playGoal();      // gol de cualquier equipo
    void playWhistle();   // silbato inicial / final
    void playTazSpin();   // Tazmania empieza a girar
    void playTazDizzy();  // Tazmania termina de girar (mareado)
    void playTazHit();    // choque contra Tazmania mientras gira

    // ── Música de fondo (en bucle) ──────────────────────────────────────────
    void playMenuMusic();    // portada Looney Tunes (menú principal)
    void playNivel1Music();  // fondo del nivel 1
    void stopMusic();        // detiene cualquier música de fondo

private:
    AudioManager() = default;

    void play(const QString& key);
    QSoundEffect* makeEffect(const QString& resourcePath, float volume);
    QSoundEffect* makeMusic(const QString& resourcePath, float volume);

    bool loaded_  = false;
    bool enabled_ = true;
    std::map<QString, QSoundEffect*> effects_;
    std::map<QString, QSoundEffect*> music_;
};
