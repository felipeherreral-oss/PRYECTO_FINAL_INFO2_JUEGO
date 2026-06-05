#pragma once
#include <QPixmap>
#include <QRect>
#include <QColor>
#include <QString>
#include <vector>
#include <map>
#include "GameExceptions.h"

/**
 * @brief Gestor centralizado de sprites.
 *
 *  - Carga cada spritesheet UNA sola vez (caché por nombre).
 *  - Extrae frames individuales con QPixmap::copy().
 *  - Elimina el fondo gris automáticamente con removeBackground().
 *  - Escala los frames al tamaño de juego deseado.
 *
 *  Contenedor STL: std::map<QString, QPixmap> para caché de sheets.
 *                  std::vector<QPixmap>        para secuencias de frames.
 *
 *  Uso:
 *    SpriteManager& sm = SpriteManager::instance();
 *    QPixmap frame = sm.getFrame("j1", AnimState::RUN, 2);
 */
class SpriteManager {
public:
    // ── Estados de animación ──────────────────────────────────────────────────
    enum class AnimState {
        IDLE,       // Quieto
        RUN,        // Corriendo
        SHOOT,      // Lanzando
        SAVE,       // Atajando (arquero)
        DEFEND,     // Defendiendo
        SPIN,       // Tazmania girando (tornado)
        DIZZY,      // Tazmania mareado
        CELEBRATE   // Celebración (gol)
    };

    // ── Singleton ─────────────────────────────────────────────────────────────
    static SpriteManager& instance() {
        static SpriteManager inst;
        return inst;
    }

    /** Carga todos los spritesheets. Llamar una vez al inicio. */
    void loadAll();

    /**
     * Retorna el frame número @p frameIdx de la animación @p state
     * del personaje @p character.
     * @param character  "j1", "j2", "gk_human", "taz", "bugs", "daffy", "gk_enemy"
     * @param state      Estado de animación deseado.
     * @param frameIdx   Índice del frame (0-based, se hace módulo automático).
     * @param size       Tamaño final del pixmap (escala al vuelo).
     * @param flipH      Si true, espeja horizontalmente (para mirar izquierda).
     */
    QPixmap getFrame(const QString& character, AnimState state,
                     int frameIdx, QSize size = {64, 64},
                     bool flipH = false) const;

    /** Número de frames disponibles para una animación dada. */
    int frameCount(const QString& character, AnimState state) const;

    /** Retorna el sprite del balón escalado al tamaño dado. */
    QPixmap getBall(QSize size) const;

    /** Retorna el fondo de cancha escalado al tamaño dado. */
    QPixmap getField(QSize size) const;

    bool isLoaded() const { return loaded_; }

private:
    SpriteManager() = default;

    // ── Datos internos ────────────────────────────────────────────────────────
    // Clave: nombre del personaje + estado → vector de frames recortados
    std::map<QString, std::vector<QPixmap>> frames_;

    // Caché de sheets cargados
    std::map<QString, QPixmap> sheets_;

    QPixmap fieldPixmap_;
    QPixmap ballPixmap_;
    bool    loaded_ = false;

    // ── Helpers privados ──────────────────────────────────────────────────────

    /** Carga un sheet desde recursos Qt y lo guarda en caché. */
    QPixmap& loadSheet(const QString& key, const QString& resourcePath);

    /**
     * Recorta una lista de QRect del sheet dado y los agrega
     * a frames_[key] con fondo eliminado.
     */
    void addFrames(const QString& key, const QPixmap& sheet,
                   const std::vector<QRect>& rects);

    /**
     * Elimina el fondo gris del sprite usando una máscara de color.
     * Compara cada píxel con @p bgColor con tolerancia @p tol.
     */
    static QPixmap removeBackground(const QPixmap& src,
                                     QColor bgColor = QColor(190, 190, 195),
                                     int tol = 35);

    /** Construye la clave interna para el mapa de frames. */
    static QString makeKey(const QString& character, AnimState state);
};
