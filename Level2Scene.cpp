#include "Level2Scene.h"
#include "SpriteManager.h"
#include "AudioManager.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPainter>
#include <QDateTime>
#include <QFont>
#include <cmath>
#include <algorithm>


// CONSTRUCTOR
Level2Scene::Level2Scene(QObject* parent, int difficultyPreset)
    : QGraphicsScene(parent)
    , manager_(std::make_unique<GameManager>(300))
    , loopTimer_(new QTimer(this))
    , difficultyPreset_(difficultyPreset)
{
    try {
        try {
            SpriteManager::instance().loadAll();
        } catch (const ResourceLoadException& e) {
            qWarning("Advertencia de sprite: %s", e.what());
        }

        setSceneRect(0, 0, SCENE_W, SCENE_H);

        setupField();
        setupPlayers();
        setupBall();
        setupHUD();
        applyDifficultyPreset();
        connectSignals();

        connect(loopTimer_, &QTimer::timeout, this, &Level2Scene::gameLoop);
        loopTimer_->start(LOOP_INTERVAL_MS);

        lastTime_ = QDateTime::currentMSecsSinceEpoch();
        manager_->startMatch();

        // Silbato inicial del partido
        AudioManager::instance().loadAll();
        AudioManager::instance().playWhistle();

    } catch (const GameException& e) {
        qWarning("Error al inicializar Level2Scene: %s", e.what());
    }
}


// SETUP DEL CAMPO
void Level2Scene::setupField() {
    drawBackground();
}

void Level2Scene::drawBackground() {
    // Imagen de cancha estirada a toda la escena
    QPixmap fieldPx = SpriteManager::instance().getField(
        QSize(int(SCENE_W), int(SCENE_H)));

    auto* bgItem = new QGraphicsPixmapItem(fieldPx);
    bgItem->setPos(0, 0);
    bgItem->setZValue(-20);
    addItem(bgItem);

    // Líneas de gol (boca vertical) — refuerzo visual sutil sobre la cancha
    auto postPen = QPen(QColor(255, 255, 255, 120), 3, Qt::SolidLine, Qt::RoundCap);
    addLine(HUMAN_GOAL_X, GOAL_TOP, HUMAN_GOAL_X, GOAL_BOT, postPen)->setZValue(-8);
    addLine(ENEMY_GOAL_X, GOAL_TOP, ENEMY_GOAL_X, GOAL_BOT, postPen)->setZValue(-8);

    // Etiquetas de equipo
    auto* lblHuman = addText("HUMANOS", QFont("Arial", 10, QFont::Bold));
    lblHuman->setDefaultTextColor(QColor(80, 160, 255, 220));
    lblHuman->setPos(PLAY_LEFT + 8, PLAY_TOP - 26);
    lblHuman->setZValue(-7);

    auto* lblEnemy = addText("LOONEY TUNES", QFont("Arial", 10, QFont::Bold));
    lblEnemy->setDefaultTextColor(QColor(255, 90, 90, 220));
    lblEnemy->setPos(PLAY_RIGHT - 120, PLAY_TOP - 26);
    lblEnemy->setZValue(-7);
}


// SETUP DE JUGADORES
void Level2Scene::setupPlayers() {
    const float bMinX = PLAY_LEFT  + 18.f;
    const float bMaxX = PLAY_RIGHT - 18.f;
    const float bMinY = PLAY_TOP    + 18.f;
    const float bMaxY = PLAY_BOTTOM - 18.f;

    //  Equipo humano (lado izquierdo, atacan a la DERECHA)
    auto* p1 = new HumanPlayer({PLAY_CX - 150.f, PLAY_CY - 70.f},
                               HumanPlayer::PlayerNumber::ONE);
    p1->setBounds(bMinX, bMaxX, bMinY, bMaxY);
    p1->setGoalCenter({ENEMY_GOAL_X, GOAL_Y_CENTER});
    p1->setIdleTarget({PLAY_CX, PLAY_CY + 80.f});
    addItem(p1);
    humanPlayers_.push_back(p1);

    auto* p2 = new HumanPlayer({PLAY_CX - 150.f, PLAY_CY + 70.f},
                               HumanPlayer::PlayerNumber::TWO);
    p2->setBounds(bMinX, bMaxX, bMinY, bMaxY);
    p2->setGoalCenter({ENEMY_GOAL_X, GOAL_Y_CENTER});
    p2->setIdleTarget({PLAY_CX, PLAY_CY - 80.f});
    addItem(p2);
    humanPlayers_.push_back(p2);

    // Arquero humano: X fija justo delante del arco izquierdo, se mueve en Y
    humanGoalkeeper_ = new GoalkeeperAI(
        {HUMAN_GOAL_X + GK_INSET, GOAL_Y_CENTER},
        GoalkeeperAI::Team::HUMAN,
        GOAL_TOP, GOAL_BOT,
        PLAY_W, PLAY_H);
    addItem(humanGoalkeeper_);

    // ─── Equipo rival Looney Tunes (lado derecho, atacan a la IZQUIERDA) ──────
    // Tazmania: órbita (MCU) en la zona derecha
    auto* taz = new EnemyPlayer(
        {PLAY_CX + 120.f, PLAY_CY},
        EnemyPlayer::EnemyType::TAZMANIA,
        /* orbitCenter */ {PLAY_CX + 120.f, PLAY_CY},
        /* radius */      95.f,
        /* omega */       1.2f,
        /* phi0 */        0.f,
        PLAY_W, PLAY_H);
    taz->setPlayBounds(bMinX, bMaxX, bMinY, bMaxY);
    addItem(taz);
    enemyPlayers_.push_back(taz);   // index 0 = Tazmania

    // Bugs Bunny: agente inteligente (defensa + ataque)
    auto* bugs = new EnemyPlayer(
        {PLAY_CX + 90.f, PLAY_CY - 80.f},
        EnemyPlayer::EnemyType::BUGS_BUNNY,
        {PLAY_CX + 90.f, PLAY_CY - 80.f}, 0.f, 0.f, 0.f,
        PLAY_W, PLAY_H);
    bugs->setPlayBounds(bMinX, bMaxX, bMinY, bMaxY);
    addItem(bugs);
    enemyPlayers_.push_back(bugs);  // index 1 = Bugs Bunny

    // Arquero rival (Pato Lucas): X fija delante del arco derecho, se mueve en Y
    enemyGoalkeeper_ = new GoalkeeperAI(
        {ENEMY_GOAL_X - GK_INSET, GOAL_Y_CENTER},
        GoalkeeperAI::Team::ENEMY,
        GOAL_TOP, GOAL_BOT,
        PLAY_W, PLAY_H);
    addItem(enemyGoalkeeper_);

    // Jugador 1 empieza activo
    humanPlayers_[0]->setActiveControl(true);
    humanPlayers_[1]->setActiveControl(false);
    activePlayerIdx_ = 0;
}

void Level2Scene::setupBall() {
    ball_ = std::make_unique<Ball>(Vec2D{PLAY_CX, PLAY_CY});
    ball_->setBoundsCheck(PLAY_LEFT, PLAY_RIGHT, PLAY_TOP, PLAY_BOTTOM);
    addItem(ball_.get());

    humanPlayers_[0]->giveBall(ball_.get());
}

void Level2Scene::setupHUD() {
    hud_ = new HUD(SCENE_W, SCENE_H);
    addItem(hud_);
}


// PERFIL DE DIFICULTAD (Fácil / Normal / Difícil)
void Level2Scene::applyDifficultyPreset() {
    float base, ramp, aggro;
    switch (difficultyPreset_) {
    case 0:  base = 0.10f; ramp = 240.f; aggro = 0.85f; break;  // Fácil
    case 2:  base = 0.32f; ramp = 110.f; aggro = 1.25f; break;  // Difícil
    default: base = 0.16f; ramp = 180.f; aggro = 1.00f; break;  // Normal
    }
    enemyAggro_ = aggro;

    // Rivales: empiezan lentos y aceleran con el tiempo (agente que aprende)
    for (auto* ep : enemyPlayers_)
        ep->getAgent()->setProfile(base, ramp);
    enemyGoalkeeper_->getAgent()->setProfile(base, ramp);

    // Arquero humano: arranca más capaz para proteger al jugador
    humanGoalkeeper_->getAgent()->setProfile(std::max(0.40f, base), 150.f);
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
            [this](float){
                float t = manager_->getGameTime();
                for (auto* ep : enemyPlayers_) ep->updateDifficulty(t);
                enemyGoalkeeper_->updateDifficulty(t);
                humanGoalkeeper_->updateDifficulty(t);
            });
}


// GAME LOOP
void Level2Scene::gameLoop() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    float dt   = std::min(float(now - lastTime_) / 1000.f, 0.05f);
    lastTime_  = now;

    manager_->tick(dt);
    if (!manager_->isPlaying()) return;

    gameTimeAccum_ += dt;

    updateGame(dt);
    hud_->update(dt);
    update();
}

void Level2Scene::updateGame(float dt) {
    // 1. Apoyo / posicionamiento de los humanos sin balón
    updateHumanSupport();

    // 2. Jugadores humanos
    for (auto* p : humanPlayers_) p->update(dt);

    // 3. Balón
    ball_->update(dt);

    // 4. IA (arqueros + rivales)
    updateAI(dt);

    // 4b. Nadie de campo puede entrar a la zona azul (área de arco)
    enforceGoalAreas();

    // 5. Colisiones
    checkCollisions();

    // 6. Posesión
    checkBallPickup();

    // 7. Goles (antes de fuera de límites para no anular un gol)
    checkGoals();

    // 8. Balón fuera del terreno → al centro
    checkOutOfBounds();

    // 9. HUD
    hud_->setPossession(humanHolder() != nullptr);
    hud_->setActivePlayer(activePlayerIdx_ + 1);
    hud_->setDifficultyLevel(enemyGoalkeeper_->getDifficulty());
}


// IA
void Level2Scene::updateAI(float dt) {
    Vec2D ballPos = ball_->getPosition();
    Vec2D ballVel = ball_->getVelocity();
    Vec2D activeHumanPos = humanPlayers_[activePlayerIdx_]->getPosition();
    bool  humanShooting  = humanPlayers_[activePlayerIdx_]->isShooting();

    // Arqueros (siempre activos, siguen/predicen el balón en Y)
    humanGoalkeeper_->updateAI(ballPos, ballVel, activeHumanPos, humanShooting, dt);
    enemyGoalkeeper_->updateAI(ballPos, ballVel, activeHumanPos, humanShooting, dt);
    humanGoalkeeper_->update(dt);
    enemyGoalkeeper_->update(dt);

    Vec2D enemyGoalCenter = {ENEMY_GOAL_X, GOAL_Y_CENTER};

    if (humanHolder()) {
        // Los humanos tienen el balón → rivales defienden siguiendo el balón
        Vec2D holderPos = humanHolder()->getPosition();
        bool  holderShooting = humanHolder()->isShooting();
        for (auto* ep : enemyPlayers_) {
            ep->updateAI(ballPos, ballVel, holderPos, holderShooting,
                         enemyGoalCenter, dt);
            ep->update(dt);
        }
    }
    else if (enemyHolder()) {
        // Los rivales tienen el balón → atacan y se pasan
        runEnemyOffense(enemyHolder(), dt);
        for (auto* ep : enemyPlayers_) ep->update(dt);
    }
    else {
        // Balón suelto → el rival más cercano va por él
        runEnemyLooseChase(dt);
        for (auto* ep : enemyPlayers_) ep->update(dt);
    }
}

// ── Ataque rival: dribla hacia el arco humano, se pasa con su compañero y tira ─
void Level2Scene::runEnemyOffense(EnemyPlayer* holder, float dt) {
    if (!holder) return;

    EnemyPlayer* mate = nullptr;
    for (auto* ep : enemyPlayers_) if (ep != holder) { mate = ep; break; }

    Vec2D humanGoal = {HUMAN_GOAL_X, GOAL_Y_CENTER};
    float diff = holder->getAgent()->getDifficultyLevel();

    // Velocidades: empiezan lentas (diff bajo) y suben con el tiempo
    float dribbleSpd = (60.f  + 150.f * diff) * enemyAggro_;
    float mateSpd    = (70.f  + 140.f * diff) * enemyAggro_;
    float passSpd    = (240.f + 220.f * diff) * enemyAggro_;
    float shootSpd   = (340.f + 180.f * diff) * enemyAggro_;
    const float shootRange = 300.f;

    Vec2D holderPos = holder->getPosition();
    holder->steerTo(humanGoal, dribbleSpd, dt);

    // El compañero se ofrece adelantado para recibir
    if (mate) {
        float side = (mate->getPosition().y < GOAL_Y_CENTER) ? -1.f : 1.f;
        mate->steerTo({HUMAN_GOAL_X + 180.f, GOAL_Y_CENTER + side * 90.f},
                      mateSpd, dt);
    }

    float distToGoal = (humanGoal - holderPos).length();

    enemyDecisionTimer_ -= dt;
    if (enemyDecisionTimer_ <= 0.f) {
        enemyDecisionTimer_ = 1.0f + (1.f - diff) * 1.2f;  // decide más seguido si es difícil

        if (distToGoal < shootRange) {
            // Tiro al arco humano (impreciso al principio, afinado con el tiempo)
            ballShotByHuman_ = false;
            Ball* b = holder->dropBall();
            if (b) {
                float inacc = (1.f - diff) * 70.f;
                float ty = GOAL_Y_CENTER +
                           std::sin(gameTimeAccum_ * 5.1f) * (inacc + 25.f);
                ty = std::max(GOAL_TOP + 12.f, std::min(GOAL_BOT - 12.f, ty));
                b->shoot({HUMAN_GOAL_X - 6.f, ty}, shootSpd);
            }
        }
        else if (mate && mate->getPosition().x < holderPos.x &&
                 std::sin(gameTimeAccum_ * 2.3f) > 0.f) {
            // Pase al compañero adelantado
            Vec2D matePos = mate->getPosition();
            Ball* b = holder->dropBall();
            if (b) {
                b->pass(matePos, passSpd);
                // Empujar el balón fuera del radio del pasador para que no lo recupere él mismo
                Vec2D dir = (matePos - holderPos).normalized();
                b->setPosition(holderPos + dir * 34.f);
            }
        }
    }
}

//  Balón suelto: el rival más cercano lo persigue para recuperar posesión
void Level2Scene::runEnemyLooseChase(float dt) {
    Vec2D ballPos = ball_->getPosition();
    EnemyPlayer* chaser = nearestEnemyTo(ballPos);

    Vec2D enemyGoalCenter = {ENEMY_GOAL_X, GOAL_Y_CENTER};
    for (auto* ep : enemyPlayers_) {
        if (ep == chaser) {
            float d = ep->getAgent()->getDifficultyLevel();
            float spd = (90.f + 150.f * d) * enemyAggro_;
            ep->steerTo(ballPos, spd, dt);
        } else {
            // El otro mantiene postura defensiva siguiendo el balón
            ep->updateAI(ballPos, ball_->getVelocity(), ballPos, false,
                         enemyGoalCenter, dt);
        }
    }
}

//  Apoyo de los humanos sin balón
void Level2Scene::updateHumanSupport() {
    HumanPlayer* holder = humanHolder();

    if (holder) {
        // Quien tiene el balón es el jugador controlado
        for (int i = 0; i < int(humanPlayers_.size()); ++i) {
            if (humanPlayers_[i] == holder && i != activePlayerIdx_) {
                humanPlayers_[activePlayerIdx_]->setActiveControl(false);
                activePlayerIdx_ = i;
                humanPlayers_[activePlayerIdx_]->setActiveControl(true);
            }
        }
        // El compañero se desmarca adelante para recibir
        HumanPlayer* mate = getInactivePlayer();
        if (mate) {
            Vec2D h = holder->getPosition();
            float side = (mate->getPosition().y < PLAY_CY) ? -1.f : 1.f;
            float tx = std::min(ENEMY_GOAL_X - 120.f, h.x + 160.f);
            mate->setIdleTarget({tx, PLAY_CY + side * 90.f});
        }
    }
    else if (enemyHolder()) {
        // El rival ataca: el humano sin control se ubica defensivamente
        Vec2D ballPos = ball_->getPosition();
        Vec2D humanGoal = {HUMAN_GOAL_X, GOAL_Y_CENTER};
        HumanPlayer* mate = getInactivePlayer();
        if (mate)
            mate->setIdleTarget((ballPos + humanGoal) * 0.5f);
    }
    else {
        // Balón suelto: el compañero se acerca al balón / se ofrece al ataque
        Vec2D ballPos = ball_->getPosition();
        HumanPlayer* mate = getInactivePlayer();
        if (mate) {
            if (ball_->getState() == Ball::State::PASSED)
                mate->setIdleTarget(ballPos);
            else {
                Vec2D enemyGoal = {ENEMY_GOAL_X, GOAL_Y_CENTER};
                mate->setIdleTarget((ballPos + enemyGoal) * 0.5f);
            }
        }
    }
}

// COLISIONES
void Level2Scene::checkCollisions() {
    //  Humanos vs Tazmania (colisión elástica) / vs Bugs (empuje suave)
    for (auto* hp : humanPlayers_) {
        for (auto* ep : enemyPlayers_) {
            if (!hp->overlaps(*ep)) continue;
            Vec2D normal = (ep->getPosition() - hp->getPosition()).normalized();

            if (ep->getType() == EnemyPlayer::EnemyType::TAZMANIA) {
                if (ep->isSpinning()) {
                    //  Rebote contra el TORNADO de Tazmania
                    // n̂ apunta desde Tazmania hacia el humano (dirección de rebote).
                    Vec2D bounceN = (hp->getPosition() - ep->getPosition()).normalized();
                    const float spinKick = 320.f;   // impulso de expulsión del giro

                    //  v_out = [v - 2(v·n̂)n̂] + n̂·vKick   (reflexión elástica + expulsión)
                    Vec2D vOut = PhysicsEngine::bounceOffSpinner(
                        hp->getVelocity(), bounceN, spinKick);
                    hp->setVelocity(vOut);

                    // Separar al humano para que no quede atrapado dentro del tornado
                    float overlap = (hp->getRadius() + ep->getRadius()) -
                                    hp->getPosition().distanceTo(ep->getPosition());
                    if (overlap > 0.f)
                        hp->setPosition(hp->getPosition() + bounceN * (overlap + 2.f));

                    // Si llevaba el balón, lo pierde por el impacto
                    if (hp->hasBall()) {
                        Ball* b = hp->releaseBall();
                        if (b) b->release(bounceN * 240.f);
                    }
                    AudioManager::instance().playTazHit();
                } else {
                    // Tazmania quieta/orbitando: colisión elástica normal
                    resolveElasticCollision(hp, ep);
                    if (hp->hasBall()) {
                        Ball* b = hp->releaseBall();
                        if (b) b->release(normal * 200.f);
                    }
                }
            } else {
                Vec2D push = normal * 150.f;
                hp->setVelocity(hp->getVelocity() - push * 0.5f);
                ep->setVelocity(ep->getVelocity() + push * 0.3f);
            }
        }
    }

    //  Balón lanzado vs arqueros (atajada)
    if (ball_->getState() == Ball::State::SHOT) {
        if (ballShotByHuman_ && ball_->overlaps(*enemyGoalkeeper_)) {
            Vec2D n = (ball_->getPosition() - enemyGoalkeeper_->getPosition()).normalized();
            ball_->onCollision(enemyGoalkeeper_, n);
            enemyGoalkeeper_->notifySave();
            hud_->showMessage("¡ATAJADA!", QColor(255, 140, 0), 1.2f);
        }
        if (!ballShotByHuman_ && ball_->overlaps(*humanGoalkeeper_)) {
            Vec2D n = (ball_->getPosition() - humanGoalkeeper_->getPosition()).normalized();
            ball_->onCollision(humanGoalkeeper_, n);
            humanGoalkeeper_->notifySave();
            hud_->showMessage("¡ATAJADA!", QColor(80, 200, 255), 1.2f);
        }
    }

    //  Tiro humano bloqueado por un defensa rival
    if (ball_->getState() == Ball::State::SHOT && ballShotByHuman_) {
        for (auto* ep : enemyPlayers_) {
            if (ep->overlaps(*ball_)) {
                Vec2D n = (ball_->getPosition() - ep->getPosition()).normalized();
                ball_->onCollision(ep, n);
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

    auto* ga = dynamic_cast<GameEntity*>(a);
    auto* gb = dynamic_cast<GameEntity*>(b);
    if (ga && gb) {
        float overlap = (ga->getRadius() + gb->getRadius()) -
                        a->getPosition().distanceTo(b->getPosition());
        if (overlap > 0.f) {
            Vec2D sep = normal * (overlap * 0.5f + 1.f);
            ga->setPosition(ga->getPosition() - sep);
            gb->setPosition(gb->getPosition() + sep);
        }
    }
}

// POSESIÓN DEL BALÓN
void Level2Scene::checkBallPickup() {
    Ball::State st = ball_->getState();
    if (st == Ball::State::HELD) return;
    if (st == Ball::State::SHOT) return;   // no se atrapa un tiro (sólo se ataja/bloquea)

    // Humanos pueden recibir balón libre o un pase
    for (int i = 0; i < int(humanPlayers_.size()); ++i) {
        HumanPlayer* hp = humanPlayers_[i];
        if (hp->hasBall()) continue;
        if (!hp->overlaps(*ball_)) continue;

        hp->giveBall(ball_.get());
        if (i != activePlayerIdx_) {
            humanPlayers_[activePlayerIdx_]->setActiveControl(false);
            activePlayerIdx_ = i;
            humanPlayers_[activePlayerIdx_]->setActiveControl(true);
        }
        return;
    }

    // Rivales recogen sólo balón libre (un pase llega primero a su destinatario)
    if (st == Ball::State::FREE) {
        for (auto* ep : enemyPlayers_) {
            if (ep->isDizzy() || ep->hasBall()) continue;
            if (!ep->overlaps(*ball_)) continue;
            ep->takeBall(ball_.get());
            ball_->pickup();
            enemyDecisionTimer_ = 0.8f;
            return;
        }
    }
}

// DETECCIÓN DE GOLES
void Level2Scene::checkGoals() {
    Ball::State st = ball_->getState();
    if (st != Ball::State::SHOT && st != Ball::State::FREE) return;

    Vec2D b = ball_->getPosition();
    bool inMouth = (b.y > GOAL_TOP && b.y < GOAL_BOT);
    if (!inMouth) return;

    // GOL HUMANO (entra en el arco rival, a la derecha)
    if (b.x > ENEMY_GOAL_X - 6.f) {
        enemyGoalkeeper_->notifyGoalScored(b.y);
        manager_->registerHumanGoal();
        hud_->setScore(manager_->getHumanGoals(), manager_->getEnemyGoals());
        hud_->showMessage("¡¡GOL!!", Qt::yellow, 2.5f);
        AudioManager::instance().playGoal();
        return;
    }

    // GOL RIVAL (entra en el arco humano, a la izquierda)
    if (b.x < HUMAN_GOAL_X + 6.f) {
        humanGoalkeeper_->notifyGoalScored(b.y);
        manager_->registerEnemyGoal();
        hud_->setScore(manager_->getHumanGoals(), manager_->getEnemyGoals());
        hud_->showMessage("¡Gol rival!", QColor(255, 90, 90), 2.5f);
        AudioManager::instance().playGoal();
        return;
    }
}

// FUERA DE LÍMITES → BALÓN AL CENTRO
void Level2Scene::checkOutOfBounds() {
    if (ball_->isOwned()) return;  // en posesión: no aplica

    Vec2D b = ball_->getPosition();
    bool inMouth = ballInsideGoalMouth();

    bool outY = (b.y < PLAY_TOP - 4.f) || (b.y > PLAY_BOTTOM + 4.f);
    bool outX = (b.x < PLAY_LEFT - 4.f) || (b.x > PLAY_RIGHT + 4.f);

    // Si sale por la línea de gol DENTRO de la boca, es asunto de checkGoals
    if (outX && inMouth) return;

    if (outX || outY) {
        ball_->setPosition({PLAY_CX, PLAY_CY});
        ball_->release({0.f, 0.f});
        enemyDecisionTimer_ = 1.0f;
        hud_->showMessage("¡Balón al centro!", QColor(220, 220, 220), 1.0f);
    }
}

// ZONA AZUL (área de arco): empuja a los jugadores de campo fuera del semicírculo
void Level2Scene::enforceGoalAreas() {
    const Vec2D leftGoal  = {HUMAN_GOAL_X, GOAL_Y_CENTER};
    const Vec2D rightGoal = {ENEMY_GOAL_X, GOAL_Y_CENTER};

    auto pushOut = [](GameEntity* e, Vec2D center, float radius) {
        Vec2D d = e->getPosition() - center;
        float dist = d.length();
        if (dist < radius) {
            Vec2D dir = (dist > 1e-3f) ? d.normalized() : Vec2D{1.f, 0.f};
            e->setPosition(center + dir * radius);
            // Anular la componente de velocidad que entra al área
            Vec2D v = e->getVelocity();
            float vn = v.dot(dir);
            if (vn < 0.f) e->setVelocity(v - dir * vn);
        }
    };

    // Sólo jugadores de CAMPO (los arqueros sí pueden estar dentro del área)
    for (auto* hp : humanPlayers_) {
        pushOut(hp, leftGoal,  AREA_RADIUS);
        pushOut(hp, rightGoal, AREA_RADIUS);
    }
    for (auto* ep : enemyPlayers_) {
        pushOut(ep, leftGoal,  AREA_RADIUS);
        pushOut(ep, rightGoal, AREA_RADIUS);
    }
}

bool Level2Scene::ballInsideGoalMouth() const {
    float y = ball_->getPosition().y;
    return (y > GOAL_TOP && y < GOAL_BOT);
}

// SLOTS
void Level2Scene::onGoalByHuman() { doKickoff(false); }  // saca el rival
void Level2Scene::onGoalByEnemy() { doKickoff(true);  }  // saca el humano

void Level2Scene::onGameOver(int h, int e) {
    loopTimer_->stop();
    AudioManager::instance().playWhistle();   // silbato final del partido
    QString result = (h > e) ? "¡GANASTE!" : (h == e) ? "¡EMPATE!" : "¡PERDISTE!";
    QColor  col    = (h > e) ? Qt::green   : (h == e) ? Qt::yellow  : QColor(255, 90, 90);
    hud_->showMessage(result + QString("\n%1 - %2").arg(h).arg(e), col, 999.f);
    emit levelCompleted(h > e);
}

void Level2Scene::onKickoff() {
    // Las posiciones ya se reubicaron en doKickoff(); nada más que hacer aquí.
}

// KICKOFF Y RESET
void Level2Scene::doKickoff(bool humanKickoff) {
    resetPositions();
    if (humanKickoff) {
        humanPlayers_[0]->giveBall(ball_.get());
        activePlayerIdx_ = 0;
        humanPlayers_[0]->setActiveControl(true);
        humanPlayers_[1]->setActiveControl(false);
    } else {
        // Saca el rival: Bugs Bunny toma el balón en el centro
        ball_->release({0.f, 0.f});
        enemyPlayers_[1]->takeBall(ball_.get());
        ball_->pickup();
        enemyDecisionTimer_ = 1.2f;
    }
}

void Level2Scene::resetPositions() {
    ball_->setPosition({PLAY_CX, PLAY_CY});
    ball_->release({0.f, 0.f});

    humanPlayers_[0]->setPosition({PLAY_CX - 150.f, PLAY_CY - 70.f});
    humanPlayers_[0]->setVelocity(Vec2D::zero());
    humanPlayers_[1]->setPosition({PLAY_CX - 150.f, PLAY_CY + 70.f});
    humanPlayers_[1]->setVelocity(Vec2D::zero());

    humanGoalkeeper_->setPosition({HUMAN_GOAL_X + GK_INSET, GOAL_Y_CENTER});
    humanGoalkeeper_->setVelocity(Vec2D::zero());

    enemyPlayers_[0]->setPosition({PLAY_CX + 120.f, PLAY_CY});
    enemyPlayers_[0]->setVelocity(Vec2D::zero());
    enemyPlayers_[1]->setPosition({PLAY_CX + 90.f, PLAY_CY - 80.f});
    enemyPlayers_[1]->setVelocity(Vec2D::zero());

    enemyGoalkeeper_->setPosition({ENEMY_GOAL_X - GK_INSET, GOAL_Y_CENTER});
    enemyGoalkeeper_->setVelocity(Vec2D::zero());
}

// INPUT
void Level2Scene::keyPressEvent(QKeyEvent* e) {
    if (!manager_->isPlaying()) return;
    int key = e->key();

    // Cambiar jugador activo
    if (key == Qt::Key_Tab) {
        switchActivePlayer();
        return;
    }

    HumanPlayer* active   = getActivePlayer();
    HumanPlayer* inactive = getInactivePlayer();

    // Pase: G (J1) o L (J2)
    if (key == Qt::Key_G || key == Qt::Key_L) {
        if (active && inactive && active->hasBall()) {
            Vec2D from = active->getPosition();
            Vec2D to   = inactive->getPosition();
            active->passToBuddy(inactive);
            // Empujar el balón para que el pasador no lo recupere de inmediato
            Vec2D dir = (to - from).normalized();
            ball_->setPosition(from + dir * 34.f);
            // El receptor pasa a ser el jugador controlado
            humanPlayers_[activePlayerIdx_]->setActiveControl(false);
            activePlayerIdx_ = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
            humanPlayers_[activePlayerIdx_]->setActiveControl(true);
        }
        return;
    }

    // Marcar que el último tiro fue del humano (para atajadas/bloqueos)
    bool isShootKey = (key == Qt::Key_F || key == Qt::Key_K);
    if (isShootKey && active && active->hasBall())
        ballShotByHuman_ = true;

    if (active) active->handleKeyPress(key);
}

void Level2Scene::keyReleaseEvent(QKeyEvent* e) {
    for (auto* p : humanPlayers_)
        if (p->isActive()) p->handleKeyRelease(e->key());
}

void Level2Scene::switchActivePlayer() {
    humanPlayers_[activePlayerIdx_]->setActiveControl(false);
    activePlayerIdx_ = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
    humanPlayers_[activePlayerIdx_]->setActiveControl(true);
}

// UTILIDADES
HumanPlayer* Level2Scene::getActivePlayer() {
    if (humanPlayers_.empty()) return nullptr;
    return humanPlayers_[activePlayerIdx_];
}

HumanPlayer* Level2Scene::getInactivePlayer() {
    if (humanPlayers_.size() < 2) return nullptr;
    int idx = (activePlayerIdx_ + 1) % int(humanPlayers_.size());
    return humanPlayers_[idx];
}

HumanPlayer* Level2Scene::humanHolder() {
    for (auto* p : humanPlayers_) if (p->hasBall()) return p;
    return nullptr;
}

EnemyPlayer* Level2Scene::enemyHolder() {
    for (auto* ep : enemyPlayers_) if (ep->hasBall()) return ep;
    return nullptr;
}

EnemyPlayer* Level2Scene::nearestEnemyTo(Vec2D p) {
    EnemyPlayer* best = nullptr;
    float bestD = 1e9f;
    // Preferir no-mareados
    for (auto* ep : enemyPlayers_) {
        if (ep->isDizzy()) continue;
        float d = ep->getPosition().distanceTo(p);
        if (d < bestD) { bestD = d; best = ep; }
    }
    if (best) return best;
    for (auto* ep : enemyPlayers_) {
        float d = ep->getPosition().distanceTo(p);
        if (d < bestD) { bestD = d; best = ep; }
    }
    return best;
}
