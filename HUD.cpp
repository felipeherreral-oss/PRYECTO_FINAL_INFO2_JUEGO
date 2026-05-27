#include "HUD.h"
#include <QPainter>
#include <QLinearGradient>
#include <cmath>

HUD::HUD(float screenW, float screenH)
    : screenW_(screenW), screenH_(screenH)
{
    setZValue(100); // Siempre encima de todo
}

void HUD::setScore(int h, int e) {
    if (h > humanGoals_) { flashTimer_ = 1.f; humanScored_ = true; }
    if (e > enemyGoals_) { flashTimer_ = 1.f; humanScored_ = false; }
    humanGoals_ = h;
    enemyGoals_ = e;
}
void HUD::setTimeLeft(int s)          { timeLeft_     = s; }
void HUD::setDifficultyLevel(float d) { difficulty_   = d; }
void HUD::setPossession(bool h)       { humanBall_    = h; }
void HUD::setActivePlayer(int n)      { activePlayer_ = n; }

void HUD::showMessage(const QString& msg, QColor color, float dur) {
    msgText_  = msg;
    msgColor_ = color;
    msgTimer_ = dur;
}

void HUD::update(float dt) {
    if (msgTimer_   > 0.f) msgTimer_   -= dt;
    if (flashTimer_ > 0.f) flashTimer_ -= dt;
}

QRectF HUD::boundingRect() const {
    return QRectF(0, 0, screenW_, screenH_);
}

void HUD::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);

    // Flash de gol (parpadeo de fondo)
    if (flashTimer_ > 0.f) {
        float alpha = std::sin(flashTimer_ * 12.f) * 0.5f + 0.5f;
        QColor flashColor = humanScored_
                           ? QColor(0, 100, 255, int(alpha * 60))
                           : QColor(200, 0, 0, int(alpha * 60));
        p->fillRect(QRectF(0, 0, screenW_, screenH_), flashColor);
    }

    drawScoreboard(p);
    drawTimer(p);
    drawDifficultyBar(p);
    drawControls(p);
    drawPossession(p);
    if (msgTimer_ > 0.f) drawMessage(p);
}

void HUD::drawScoreboard(QPainter* p) {
    // Panel central superior
    QRectF panel(screenW_ * 0.5f - 110, 8, 220, 56);

    // Fondo del panel
    QLinearGradient bg(panel.topLeft(), panel.bottomLeft());
    bg.setColorAt(0, QColor(20, 20, 60, 220));
    bg.setColorAt(1, QColor(10, 10, 40, 200));
    p->setBrush(bg);
    p->setPen(QPen(QColor(100, 140, 255, 180), 2));
    p->drawRoundedRect(panel, 10, 10);

    // Logos de equipo
    // Humano: "GIDSEL"
    p->setPen(QColor(80, 160, 255));
    p->setFont(QFont("Arial", 8, QFont::Bold));
    p->drawText(QRectF(panel.left() + 5, panel.top() + 5, 80, 14),
                Qt::AlignCenter, "HUMANOS");

    // Rival: "LOONEY TUNES"
    p->setPen(QColor(255, 80, 80));
    p->drawText(QRectF(panel.right() - 85, panel.top() + 5, 80, 14),
                Qt::AlignCenter, "LOONEY");

    // Marcador (grande)
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 28, QFont::Bold));
    QString score = QString("%1 - %2").arg(humanGoals_).arg(enemyGoals_);
    p->drawText(QRectF(panel.left(), panel.top() + 18, panel.width(), 34),
                Qt::AlignCenter, score);

    // Divisor central
    p->setPen(QPen(QColor(200, 200, 255, 100), 1));
    p->drawLine(QPointF(panel.center().x(), panel.top() + 5),
                QPointF(panel.center().x(), panel.bottom() - 5));
}

void HUD::drawTimer(QPainter* p) {
    int mins = timeLeft_ / 60;
    int secs = timeLeft_ % 60;
    QString timeStr = QString("%1:%2")
                      .arg(mins, 2, 10, QChar('0'))
                      .arg(secs, 2, 10, QChar('0'));

    // Cambiar color si quedan menos de 30 segundos
    QColor timeColor = (timeLeft_ <= 30) ? QColor(255, 80, 80) : QColor(255, 220, 100);

    QRectF tPanel(screenW_ * 0.5f - 45, 70, 90, 26);
    p->setBrush(QColor(20, 20, 20, 180));
    p->setPen(QPen(timeColor.darker(120), 1));
    p->drawRoundedRect(tPanel, 6, 6);

    p->setPen(timeColor);
    p->setFont(QFont("Courier New", 14, QFont::Bold));
    p->drawText(tPanel, Qt::AlignCenter, timeStr);
}

void HUD::drawDifficultyBar(QPainter* p) {
    // Barra de dificultad en la esquina superior derecha
    QRectF barArea(screenW_ - 130, 10, 120, 40);
    p->setBrush(QColor(20, 20, 20, 160));
    p->setPen(QPen(QColor(200, 200, 200, 80), 1));
    p->drawRoundedRect(barArea, 5, 5);

    p->setPen(QColor(200, 200, 200, 200));
    p->setFont(QFont("Arial", 7));
    p->drawText(QRectF(barArea.left() + 4, barArea.top() + 3, barArea.width() - 8, 12),
                Qt::AlignLeft, "IA Rival:");

    // Barra de progreso de dificultad
    QRectF bar(barArea.left() + 4, barArea.top() + 18, barArea.width() - 8, 14);
    p->setBrush(QColor(50, 50, 50));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(bar, 4, 4);

    // Relleno con gradiente rojo→verde invertido (más difícil = más rojo)
    float fillW = (bar.width() - 2) * difficulty_;
    QLinearGradient barGrad(bar.left(), 0, bar.right(), 0);
    barGrad.setColorAt(0.0, QColor(40, 200, 40));
    barGrad.setColorAt(0.5, QColor(220, 200, 0));
    barGrad.setColorAt(1.0, QColor(255, 40, 40));
    p->setBrush(barGrad);
    p->drawRoundedRect(QRectF(bar.left() + 1, bar.top() + 1, fillW, bar.height() - 2), 3, 3);

    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 6, QFont::Bold));
    p->drawText(bar, Qt::AlignCenter, QString("%1%").arg(int(difficulty_ * 100)));
}

void HUD::drawControls(QPainter* p) {
    // Panel de controles en esquina inferior izquierda
    QRectF panel(8, screenH_ - 80, 200, 72);
    p->setBrush(QColor(10, 10, 30, 170));
    p->setPen(QPen(QColor(80, 120, 200, 120), 1));
    p->drawRoundedRect(panel, 6, 6);

    p->setPen(QColor(180, 210, 255, 220));
    p->setFont(QFont("Arial", 7, QFont::Bold));

    QRectF row1(panel.left() + 6, panel.top() + 5, panel.width() - 12, 12);
    QRectF row2(panel.left() + 6, panel.top() + 20, panel.width() - 12, 12);
    QRectF row3(panel.left() + 6, panel.top() + 35, panel.width() - 12, 12);
    QRectF row4(panel.left() + 6, panel.top() + 50, panel.width() - 12, 12);

    p->drawText(row1, Qt::AlignLeft, "J1: WASD  |  J2: ↑↓←→");
    p->drawText(row2, Qt::AlignLeft, "J1 Tiro: F  |  J2 Tiro: K");
    p->drawText(row3, Qt::AlignLeft, "J1 Pase: G  |  J2 Pase: L");

    // Jugador activo
    p->setPen(Qt::yellow);
    p->setFont(QFont("Arial", 7, QFont::Bold));
    p->drawText(row4, Qt::AlignLeft,
                QString("Activo: Jugador %1").arg(activePlayer_));
}

void HUD::drawPossession(QPainter* p) {
    // Barra de posesión en la esquina inferior derecha
    QString possText = humanBall_ ? "⚽ Tu equipo tiene el balón" : "🔴 Rival tiene el balón";
    QColor  possColor = humanBall_ ? QColor(80, 200, 255) : QColor(255, 100, 80);

    QRectF possPanel(screenW_ - 210, screenH_ - 30, 200, 22);
    p->setBrush(QColor(0, 0, 0, 150));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(possPanel, 5, 5);

    p->setPen(possColor);
    p->setFont(QFont("Arial", 8, QFont::Bold));
    p->drawText(possPanel, Qt::AlignCenter, possText);
}

void HUD::drawMessage(QPainter* p) {
    if (msgText_.isEmpty()) return;

    // Alpha parpadeante al final del tiempo
    float alpha = (msgTimer_ < 0.5f) ? (msgTimer_ / 0.5f) : 1.f;

    // Fondo semitransparente
    QRectF msgRect(screenW_ * 0.5f - 150, screenH_ * 0.5f - 40, 300, 70);
    p->setBrush(QColor(0, 0, 0, int(180 * alpha)));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(msgRect, 12, 12);

    // Texto
    QColor c = msgColor_;
    c.setAlphaF(alpha);
    p->setPen(c);
    p->setFont(QFont("Arial", 30, QFont::Bold));
    p->drawText(msgRect, Qt::AlignCenter, msgText_);
}
