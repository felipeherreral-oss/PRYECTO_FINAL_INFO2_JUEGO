#include "Level2Scene.h"
#include "SpriteManager.h"
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QDateTime>
#include <QFont>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// CONSTRUCTOR
// ─────────────────────────────────────────────────────────────────────────────
Level2Scene::Level2Scene(QObject* parent)
    : QGraphicsScene(parent)
    , manager_(std::make_unique<GameManager>(300))
    , loopTimer_(new QTimer(this))
{
    try {
        // Cargar sprites primero — si falla muestra qué archivo falta
        try {
            SpriteManager::instance().loadAll();
        } catch (const ResourceLoadException& e) {
            // Los sprites no son críticos para que el juego funcione
            // (se usará el fallback de QPainter). Solo advertimos.
            qWarning("Advertencia de sprite: %s", e.what());
        }

        setSceneRect(0, 0,
                     FIELD_W + 2 * FIELD_MARGIN,
                     FIELD_H + 2 * FIELD_MARGIN);

        setupField();
        setupPlayers();
        setupBall();
        setupHUD();
        connectSignals();

        // Iniciar game loop
        connect(loopTimer_, &QTimer::timeout, this, &Level2Scene::gameLoop);
        loopTimer_->start(LOOP_INTERVAL_MS);

        lastTime_ = QDateTime::currentMSecsSinceEpoch();
        manager_->startMatch();

    } catch (const GameException& e) {
        // En producción: mostrar diálogo de error, no crashear
        qWarning("Error al inicializar Level2Scene: %s", e.what());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SETUP DEL CAMPO
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::setupField() {
    // Fondo: verde estadio intergaláctico con efecto espacial
    // El dibujo se hace en drawBackground()
    drawBackground();
}

void Level2Scene::drawBackground() {
    float totalW = FIELD_W + 2 * FIELD_MARGIN;
    float totalH = FIELD_H + 2 * FIELD_MARGIN;

    // ─── Imagen de cancha como fondo completo ─────────────────────────────────
    QPixmap fieldPx = SpriteManager::instance().getField(
        QSize(int(totalW), int(totalH)));

    auto* bgItem = new QGraphicsPixmapItem(fieldPx);
    bgItem->setPos(0, 0);
    bgItem->setZValue(-20);
    addItem(bgItem);

    // ─── Overlay semitransparente para delimitar límites jugables ────────────
    // (la imagen ya tiene la cancha centrada, solo reforzamos los postes)

    // Postes arco humano (izquierda) — blanco
    float goalTop = GOAL_Y_CENTER - GOAL_WIDTH * 0.5f;
    float goalBot = GOAL_Y_CENTER + GOAL_WIDTH * 0.5f;
    auto postPen  = QPen(QColor(255, 255, 255, 220), 5, Qt::SolidLine,
                         Qt::RoundCap);

    addLine(HUMAN_GOAL_X, goalTop, HUMAN_GOAL_X, goalBot, postPen)->setZValue(-8);
    addLine(ENEMY_GOAL_X, goalTop, ENEMY_GOAL_X, goalBot, postPen)->setZValue(-8);

    // Etiquetas de equipo
    auto* lblHuman = addText("HUMANOS ▶", QFont("Arial", 9, QFont::Bold));
    lblHuman->setDefaultTextColor(QColor(80, 160, 255, 200));
    lblHuman->setPos(HUMAN_GOAL_X + 10, FIELD_MARGIN + 8);
    lblHuman->setZValue(-7);

    auto* lblEnemy = addText("◀ LOONEY TUNES", QFont("Arial", 9, QFont::Bold));
    lblEnemy->setDefaultTextColor(QColor(255, 80, 80, 200));
    lblEnemy->setPos(ENEMY_GOAL_X - 130, FIELD_MARGIN + 8);
    lblEnemy->setZValue(-7);
}

// ─────────────────────────────────────────────────────────────────────────────
// SETUP DE JUGADORES
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::setupPlayers() {
    float cx = FIELD_MARGIN + FIELD_W * 0.5f;
    float cy = FIELD_MARGIN + FIELD_H * 0.5f;

    // ─── Equipo humano ────────────────────────────────────────────────────────
    // Jugadores humanos pueden moverse por TODO el campo (hasta el arco rival)
    auto* p1 = new HumanPlayer({cx - 120.f, cy - 60.f}, HumanPlayer::PlayerNumber::ONE);
    p1->setBounds(FIELD_MARGIN + 5.f,
                  FIELD_MARGIN + FIELD_W - 5.f,   // ← hasta el arco rival
                  FIELD_MARGIN + 5.f,
                  FIELD_MARGIN + FIELD_H - 5.f);
    p1->setGoalCenter({ENEMY_GOAL_X, GOAL_Y_CENTER});
    p1->setIdleTarget({cx - 150.f, cy + 80.f});
    addItem(p1);
    humanPlayers_.push_back(p1);

    auto* p2 = new HumanPlayer({cx - 120.f, cy + 60.f}, HumanPlayer::PlayerNumber::TWO);
    p2->setBounds(FIELD_MARGIN + 5.f,
                  FIELD_MARGIN + FIELD_W - 5.f,   // ← hasta el arco rival
                  FIELD_MARGIN + 5.f,
                  FIELD_MARGIN + FIELD_H - 5.f);
    p2->setGoalCenter({ENEMY_GOAL_X, GOAL_Y_CENTER});
    p2->setIdleTarget({cx - 80.f, cy - 80.f});
    addItem(p2);
    humanPlayers_.push_back(p2);

    // Arquero humano: DENTRO del arco (x positivo = dentro del campo)
    // HUMAN_GOAL_X está en el borde izquierdo; el arquero va justo delante
    humanGoalkeeper_ = new GoalkeeperAI(
        {HUMAN_GOAL_X + 22.f, GOAL_Y_CENTER},      // ← 22px dentro del campo
        GoalkeeperAI::Team::HUMAN,
        HUMAN_GOAL_X + 5.f,                         // poste iz (dentro)
        HUMAN_GOAL_X + 5.f + GOAL_WIDTH,            // poste der
        FIELD_W, FIELD_H);
    addItem(humanGoalkeeper_);

    // ─── Equipo rival Looney Tunes ────────────────────────────────────────────
    // Tazmania: defensa circular en la derecha del campo
    auto* taz = new EnemyPlayer(
        {cx + 150.f, cy},
        EnemyPlayer::EnemyType::TAZMANIA,
        /* orbitCenter */ {cx + 220.f, cy},
        /* radius */      120.f,
        /* omega */        1.2f,   // rad/s
        /* phi0 */         0.f,
        FIELD_W, FIELD_H);
    addItem(taz);
    enemyPlayers_.push_back(taz);

    // Bugs Bunny: defensa posicional
    auto* bugs = new EnemyPlayer(
        {cx + 80.f, cy - 120.f},
        EnemyPlayer::EnemyType::BUGS_BUNNY,
        {cx + 80.f, cy - 120.f}, 0.f, 0.f, 0.f,
        FIELD_W, FIELD_H);
    addItem(bugs);
    enemyPlayers_.push_back(bugs);

    // Arquero rival: DENTRO del arco (x negativo = dentro del campo desde la derecha)
    enemyGoalkeeper_ = new GoalkeeperAI(
        {ENEMY_GOAL_X - 22.f, GOAL_Y_CENTER},       // ← 22px dentro del campo
        GoalkeeperAI::Team::ENEMY,
        ENEMY_GOAL_X - 5.f - GOAL_WIDTH,            // poste iz
        ENEMY_GOAL_X - 5.f,                          // poste der (dentro)
        FIELD_W, FIELD_H);
    addItem(enemyGoalkeeper_);

    // Jugador 1 empieza activo
    humanPlayers_[0]->setActiveControl(true);
    humanPlayers_[1]->setActiveControl(false);
    activePlayerIdx_ = 0;
}

void Level2Scene::setupBall() {
    float cx = FIELD_MARGIN + FIELD_W * 0.5f;
    float cy = FIELD_MARGIN + FIELD_H * 0.5f;

    ball_ = std::make_unique<Ball>(Vec2D{cx, cy});
    ball_->setBoundsCheck(FIELD_MARGIN, FIELD_MARGIN + FIELD_W,
                           FIELD_MARGIN, FIELD_MARGIN + FIELD_H);
    addItem(ball_.get());

    // El jugador 1 empieza con el balón
    humanPlayers_[0]->giveBall(ball_.get());
}

void Level2Scene::setupHUD() {
    float totalW = FIELD_W + 2 * FIELD_MARGIN;
    float totalH = FIELD_H + 2 * FIELD_MARGIN;
    hud_ = new HUD(totalW, totalH);
    addItem(hud_);
}

void Level2Scene::connectSignals() {
    connect(manager_.get(), &GameManager::goalScoredByHuman,
            this, &Level2Scene::onGoalByHuman);
    connect(manager_.get(), &GameManager::goalScoredByEnemy,
            this, &Level2Scene::onGoalByEnemy);
    connect(manager_.get(), &GameManager::gameOver,
            this, &Level2Scene::onGameOver);
    connect(manager_.get(), &GameManager::kickoffReady,
            this, &Level2Scene::onKickoff);
    connect(manager_.get(), &GameManager::timeUpdated,
            [this](int s){ hud_->setTimeLeft(s); });
    connect(manager_.get(), &GameManager::difficultyUpdated,
            [this](float d){
                hud_->setDifficultyLevel(d);
                for (auto* ep : enemyPlayers_) ep->updateDifficulty(manager_->getGameTime());
                enemyGoalkeeper_->updateDifficulty(manager_->getGameTime());
                humanGoalkeeper_->updateDifficulty(manager_->getGameTime());
            });
}

// ─────────────────────────────────────────────────────────────────────────────
// GAME LOOP
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::gameLoop() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    float dt   = std::min(float(now - lastTime_) / 1000.f, 0.05f); // Cap a 50ms
    lastTime_  = now;

    manager_->tick(dt);
    if (!manager_->isPlaying()) return;

    gameTimeAccum_ += dt;

    updateGame(dt);
    hud_->update(dt);
    update(); // Repintar la escena
}

void Level2Scene::updateGame(float dt) {
    // 1. Actualizar jugadores humanos
    for (auto* p : humanPlayers_) p->update(dt);

    // 2. Actualizar balón
    ball_->update(dt);

    // 3. Actualizar IA
    updateAI(dt);

    // 4. Colisiones
    checkCollisions();

    // 5. Posesión del balón
    checkBallPickup();

    // 6. Verificar goles
    checkGoals();

    // 7. Actualizar HUD
    bool humanBall = false;
    for (auto* p : humanPlayers_) if (p->hasBall()) humanBall = true;
    hud_->setPossession(humanBall || enemyGoalkeeper_->isActive() == false);
    hud_->setActivePlayer(activePlayerIdx_ + 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// IA
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::updateAI(float dt) {
    Vec2D ballPos  = ball_->getPosition();
    Vec2D ballVel  = ball_->getVelocity();
    Vec2D humanPos = humanPlayers_[activePlayerIdx_]->getPosition();
    bool  humanShooting = humanPlayers_[activePlayerIdx_]->isShooting();

    // ─── Arquero humano ───────────────────────────────────────────────────────
    humanGoalkeeper_->updateAI(ballPos, ballVel, humanPos, humanShooting, dt);

    // ─── Defensas rivales ─────────────────────────────────────────────────────
    Vec2D enemyGoalCenter = {ENEMY_GOAL_X, GOAL_Y_CENTER};
    for (auto* ep : enemyPlayers_) {
        ep->updateAI(ballPos, ballVel, humanPos, humanShooting, enemyGoalCenter, dt);
        ep->update(dt);
    }

    // ─── Arquero rival ────────────────────────────────────────────────────────
    enemyGoalkeeper_->updateAI(ballPos, ballVel, humanPos, humanShooting, dt);
    enemyGoalkeeper_->update(dt);
    humanGoalkeeper_->update(dt);

    // ─── Lógica del rival con el balón ───────────────────────────────────────
    if (enemyHasBall_) {
        handleEnemyBallPossession(dt);
    }
}

void Level2Scene::handleEnemyBallPossession(float dt) {
    enemyBallTimer_ -= dt;

    // El rival que tiene el balón intenta acercarse al arco humano y lanzar
    // Buscar qué enemigo tiene el balón
    EnemyPlayer* ballHolder = nullptr;
    for (auto* ep : enemyPlayers_) {
        if (ep->hasBall()) { ballHolder = ep; break; }
    }
    if (!ballHolder) {
        enemyHasBall_ = false;
        return;
    }

    Vec2D humanGoalCenter = {HUMAN_GOAL_X, GOAL_Y_CENTER};
    Vec2D toGoal = humanGoalCenter - ballHolder->getPosition();
    float distToGoal = toGoal.length();

    if (distToGoal < 300.f && enemyBallTimer_ <= 0.f) {
        // Lanzar al arco
        Ball* b = ballHolder->dropBall();
        if (b) {
            // Añadir algo de variación al lanzamiento
            Vec2D target = humanGoalCenter;
            float variation = 40.f * (1.f - ballHolder->getAgent()->getDifficultyLevel());
            // variación determinista
            float vOff = std::sin(gameTimeAccum_ * 3.7f) * variation;
            target.y += vOff;
            b->shoot(target, 480.f);
            enemyHasBall_ = false;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// COLISIONES
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::checkCollisions() {
    // ─── Jugadores humanos vs Tazmania (colisión elástica) ────────────────────
    for (auto* hp : humanPlayers_) {
        for (auto* ep : enemyPlayers_) {
            if (hp->overlaps(*ep)) {
                Vec2D normal = (ep->getPosition() - hp->getPosition()).normalized();

                if (ep->getType() == EnemyPlayer::EnemyType::TAZMANIA) {
                    // Colisión elástica con conservación de energía
                    resolveElasticCollision(hp, ep);

                    // Si el humano tenía el balón, lo suelta
                    if (hp->hasBall()) {
                        Ball* b = hp->releaseBall();
                        Vec2D pushVel = normal * 200.f;
                        b->release(pushVel);
                    }
                } else {
                    // Bugs/Daffy: empuje suave, no colisión elástica completa
                    Vec2D push = normal * 150.f;
                    hp->setVelocity(hp->getVelocity() - push * 0.5f);
                    ep->setVelocity(ep->getVelocity() + push * 0.3f);
                }
            }
        }
    }

    // ─── Balón vs Arquero rival ───────────────────────────────────────────────
    if (ball_->getState() == Ball::State::SHOT) {
        if (ball_->overlaps(*enemyGoalkeeper_)) {
            Vec2D normal = (ball_->getPosition() - enemyGoalkeeper_->getPosition()).normalized();
            ball_->onCollision(enemyGoalkeeper_, normal);
            enemyGoalkeeper_->notifySave();
            hud_->showMessage("¡ATAJADA!", QColor(255, 140, 0), 1.5f);
        }
        // Balón vs Arquero humano (si el rival lanzó)
        if (ball_->overlaps(*humanGoalkeeper_)) {
            Vec2D normal = (ball_->getPosition() - humanGoalkeeper_->getPosition()).normalized();
            ball_->onCollision(humanGoalkeeper_, normal);
            humanGoalkeeper_->notifySave();
            hud_->showMessage("¡GRAN ATAJADA!", QColor(80, 200, 255), 1.5f);
        }
    }

    // ─── Balón vs Defensas rivales ───────────────────────────────────────────
    if (ball_->getState() == Ball::State::SHOT ||
        ball_->getState() == Ball::State::PASSED) {
        for (auto* ep : enemyPlayers_) {
            if (ball_->overlaps(*ep)) {
                Vec2D normal = (ball_->getPosition() - ep->getPosition()).normalized();
                ball_->onCollision(ep, normal);
                // El defensa puede capturar el balón si está libre
                if (!enemyHasBall_ && ball_->getState() == Ball::State::FREE) {
                    ep->takeBall(ball_.get());
                    ball_->pickup();
                    enemyHasBall_ = true;
                    enemyBallTimer_ = 1.5f + std::sin(gameTimeAccum_) * 0.5f;
                }
                break;
            }
        }
    }
}

void Level2Scene::resolveElasticCollision(Collidable* a, Collidable* b) {
    Vec2D normal = (b->getPosition() - a->getPosition()).normalized();
    if (normal.lengthSq() < 0.01f) return;

    auto result = PhysicsEngine::elasticCollision2D(
        a->getVelocity(), a->getMass(),
        b->getVelocity(), b->getMass(),
        normal);

    a->setVelocity(result.v1After);
    b->setVelocity(result.v2After);

    // Separar los objetos para evitar penetración
    float overlap = (dynamic_cast<GameEntity*>(a)->getRadius() +
                     dynamic_cast<GameEntity*>(b)->getRadius()) -
                    a->getPosition().distanceTo(b->getPosition());
    if (overlap > 0.f) {
        Vec2D sep = normal * (overlap * 0.5f + 1.f);
        if (auto* ga = dynamic_cast<GameEntity*>(a))
            ga->setPosition(ga->getPosition() - sep);
        if (auto* gb = dynamic_cast<GameEntity*>(b))
            gb->setPosition(gb->getPosition() + sep);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// POSESIÓN DEL BALÓN
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::checkBallPickup() {
    if (ball_->isOwned()) return;
    if (ball_->getState() == Ball::State::SHOT) return;

    // ─── Pase: el receptor recoge el balón si está cerca ─────────────────────
    // Cualquier jugador humano puede recoger el balón libre o en pase
    for (int i = 0; i < int(humanPlayers_.size()); ++i) {
        HumanPlayer* hp = humanPlayers_[i];
        if (hp->hasBall()) continue;  // ya tiene balón
        if (!hp->overlaps(*ball_)) continue;

        hp->giveBall(ball_.get());
        enemyHasBall_ = false;

        // El que recoge el balón se vuelve el jugador activo
        if (i != activePlayerIdx_) {
            humanPlayers_[activePlayerIdx_]->setActiveControl(false);
            activePlayerIdx_ = i;
            humanPlayers_[activePlayerIdx_]->setActiveControl(true);
        }
        return;
    }

    // ─── Rival recoge balón libre ─────────────────────────────────────────────
    if (!enemyHasBall_ && ball_->getState() == Ball::State::FREE) {
        for (auto* ep : enemyPlayers_) {
            if (ep->overlaps(*ball_) && !ep->hasBall()) {
                ep->takeBall(ball_.get());
                ball_->pickup();
                enemyHasBall_  = true;
                enemyBallTimer_ = 1.5f;
                return;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DETECCIÓN DE GOLES
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::checkGoals() {
    if (ball_->getState() != Ball::State::SHOT &&
        ball_->getState() != Ball::State::FREE) return;

    Vec2D bPos = ball_->getPosition();
    float goalTop = GOAL_Y_CENTER - GOAL_WIDTH * 0.5f;
    float goalBot = GOAL_Y_CENTER + GOAL_WIDTH * 0.5f;

    bool inVertRange = (bPos.y > goalTop && bPos.y < goalBot);

    // ─── GOL HUMANO (balón entra en arco rival) ───────────────────────────────
    if (inVertRange && bPos.x > ENEMY_GOAL_X - 10.f) {
        enemyGoalkeeper_->notifyGoalScored(bPos.x);
        manager_->registerHumanGoal();
        hud_->setScore(manager_->getHumanGoals(), manager_->getEnemyGoals());
        hud_->showMessage("¡¡GOL!!", Qt::yellow, 2.5f);
        return;
    }

    // ─── GOL RIVAL (balón entra en arco humano) ───────────────────────────────
    if (inVertRange && bPos.x < HUMAN_GOAL_X + 10.f) {
        humanGoalkeeper_->notifyGoalScored(bPos.x);
        manager_->registerEnemyGoal();
        hud_->setScore(manager_->getHumanGoals(), manager_->getEnemyGoals());
        hud_->showMessage("¡Goool rival!", QColor(255, 80, 80), 2.5f);
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SLOTS
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::onGoalByHuman() {
    doKickoff(false); // El rival saca tras recibir gol
}

void Level2Scene::onGoalByEnemy() {
    doKickoff(true);  // El humano saca
}

void Level2Scene::onGameOver(int h, int e) {
    loopTimer_->stop();
    QString result = (h > e) ? "¡GANASTE!" : (h == e) ? "¡EMPATE!" : "¡PERDISTE!";
    QColor  col    = (h > e) ? Qt::green   : (h == e) ? Qt::yellow  : QColor(255, 80, 80);
    hud_->showMessage(result + QString("\n%1 - %2").arg(h).arg(e), col, 999.f);
    emit levelCompleted(h > e);
}

void Level2Scene::onKickoff() {
    // Esperar a que el GameManager dé señal y luego colocar
    // (ya se hizo en doKickoff)
}

// ─────────────────────────────────────────────────────────────────────────────
// KICKOFF Y RESET
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::doKickoff(bool humanKickoff) {
    resetPositions();
    if (humanKickoff) {
        humanPlayers_[0]->giveBall(ball_.get());
        activePlayerIdx_ = 0;
        humanPlayers_[0]->setActiveControl(true);
        humanPlayers_[1]->setActiveControl(false);
    } else {
        // Rival saca: colocar balón en el centro y darlo a un enemigo
        ball_->release({0.f, 0.f});
        enemyPlayers_[0]->takeBall(ball_.get());
        ball_->pickup();
        enemyHasBall_ = true;
        enemyBallTimer_ = 1.2f;
    }
}

void Level2Scene::resetPositions() {
    float cx = FIELD_MARGIN + FIELD_W * 0.5f;
    float cy = FIELD_MARGIN + FIELD_H * 0.5f;

    ball_->setPosition({cx, cy});
    ball_->release({0.f, 0.f});
    enemyHasBall_ = false;

    humanPlayers_[0]->setPosition({cx - 120.f, cy - 60.f});
    humanPlayers_[0]->setVelocity(Vec2D::zero());
    humanPlayers_[1]->setPosition({cx - 120.f, cy + 60.f});
    humanPlayers_[1]->setVelocity(Vec2D::zero());

    humanGoalkeeper_->setPosition({HUMAN_GOAL_X + 22.f, GOAL_Y_CENTER});
    humanGoalkeeper_->setVelocity(Vec2D::zero());

    enemyPlayers_[0]->setPosition({cx + 150.f, cy});
    enemyPlayers_[0]->setVelocity(Vec2D::zero());
    enemyPlayers_[1]->setPosition({cx + 80.f, cy - 120.f});
    enemyPlayers_[1]->setVelocity(Vec2D::zero());

    enemyGoalkeeper_->setPosition({ENEMY_GOAL_X - 22.f, GOAL_Y_CENTER});
    enemyGoalkeeper_->setVelocity(Vec2D::zero());
}

// ─────────────────────────────────────────────────────────────────────────────
// INPUT
// ─────────────────────────────────────────────────────────────────────────────
void Level2Scene::keyPressEvent(QKeyEvent* e) {
    if (!manager_->isPlaying()) return;

    int key = e->key();

    // ── Cambiar jugador activo con Tab ────────────────────────────────────────
    if (key == Qt::Key_Tab) {
        switchActivePlayer();
        return;
    }

    // ── Pase: G (J1 activo) o L (J2 activo) ──────────────────────────────────
    bool isPassKey = (key == Qt::Key_G || key == Qt::Key_L);
    if (isPassKey) {
        HumanPlayer* active   = getActivePlayer();
        HumanPlayer* inactive = getInactivePlayer();
        if (active && inactive && active->hasBall()) {
            active->passToBuddy(inactive);
            // El receptor se vuelve activo inmediatamente para que el jugador
            // pueda controlarlo en cuanto reciba el pase
            humanPlayers_[activePlayerIdx_]->setActiveControl(false);
            activePlayerIdx_ = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
            humanPlayers_[activePlayerIdx_]->setActiveControl(true);
        }
        return;
    }

    // ── Movimiento y tiro: propagar solo al jugador activo ────────────────────
    HumanPlayer* active = getActivePlayer();
    if (active) active->handleKeyPress(key);
}

void Level2Scene::keyReleaseEvent(QKeyEvent* e) {
    for (auto* p : humanPlayers_) {
        if (p->isActive()) p->handleKeyRelease(e->key());
    }
}

void Level2Scene::switchActivePlayer() {
    humanPlayers_[activePlayerIdx_]->setActiveControl(false);
    activePlayerIdx_ = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
    humanPlayers_[activePlayerIdx_]->setActiveControl(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// UTILIDADES
// ─────────────────────────────────────────────────────────────────────────────
HumanPlayer* Level2Scene::getActivePlayer() {
    if (humanPlayers_.empty()) return nullptr;
    return humanPlayers_[activePlayerIdx_];
}

HumanPlayer* Level2Scene::getInactivePlayer() {
    if (humanPlayers_.size() < 2) return nullptr;
    int inactiveIdx = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
    return humanPlayers_[inactiveIdx];
}
