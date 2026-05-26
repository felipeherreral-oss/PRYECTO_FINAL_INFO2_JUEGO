#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QTimer>
#include <QString>

/**
 * @brief HUD (Heads-Up Display) del Nivel 2.
 *
 *  Muestra: marcador, tiempo restante, dificultad actual de la IA,
 *           controles, y mensajes de eventos (¡GOL!, ¡ATAJADA!, etc.)
 */
class HUD : public QGraphicsItem {
public:
    explicit HUD(float screenW, float screenH);

    // ── Actualización de estado ───────────────────────────────────────────────
    void setScore(int humanGoals, int enemyGoals);
    void setTimeLeft(int seconds);
    void setDifficultyLevel(float diff);   // [0.0 – 1.0]
    void setPossession(bool humanHasBall);
    void setActivePlayer(int playerNum);   // 1 o 2

    /** Mostrar un mensaje flotante temporal (p. ej. "¡GOL!"). */
    void showMessage(const QString& msg, QColor color = Qt::yellow, float durationSec = 2.f);

    /** Actualizar lógica del HUD (bajar timer de mensaje). */
    void update(float dt);

    // ── QGraphicsItem ─────────────────────────────────────────────────────────
    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    float screenW_, screenH_;

    int   humanGoals_  = 0;
    int   enemyGoals_  = 0;
    int   timeLeft_    = 300;  // 5 minutos
    float difficulty_  = 0.15f;
    bool  humanBall_   = false;
    int   activePlayer_= 1;

    // Mensaje flotante
    QString msgText_;
    QColor  msgColor_  = Qt::yellow;
    float   msgTimer_  = 0.f;

    // Flash de gol
    float   flashTimer_= 0.f;
    bool    humanScored_= false;

    void drawScoreboard(QPainter* p);
    void drawTimer(QPainter* p);
    void drawDifficultyBar(QPainter* p);
    void drawControls(QPainter* p);
    void drawMessage(QPainter* p);
    void drawPossession(QPainter* p);
};
