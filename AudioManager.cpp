#include "AudioManager.h"
#include <QSoundEffect>
#include <QUrl>

// Reproduce un efecto en cuanto esté cargado (evita perder el primer play si
// el QSoundEffect aún se está cargando al arrancar la app).
static void playWhenReady(QSoundEffect* fx) {
    if (!fx) return;
    if (fx->status() == QSoundEffect::Ready) {
        fx->play();
        return;
    }
    QObject::connect(fx, &QSoundEffect::statusChanged, fx, [fx]() {
        if (fx->status() == QSoundEffect::Ready && !fx->isPlaying())
            fx->play();
    });
}

AudioManager& AudioManager::instance() {
    static AudioManager inst;
    return inst;
}

QSoundEffect* AudioManager::makeEffect(const QString& path, float volume) {
    auto* fx = new QSoundEffect();      // vive durante toda la app (singleton)
    fx->setSource(QUrl(path));
    fx->setVolume(volume);
    return fx;
}

QSoundEffect* AudioManager::makeMusic(const QString& path, float volume) {
    auto* fx = new QSoundEffect();
    fx->setSource(QUrl(path));
    fx->setVolume(volume);
    fx->setLoopCount(QSoundEffect::Infinite);   // música en bucle continuo
    return fx;
}

void AudioManager::loadAll() {
    if (loaded_) return;

    effects_["goal"]      = makeEffect("qrc:/sounds/gol_anotado.wav",                0.95f);
    effects_["whistle"]   = makeEffect("qrc:/sounds/silbato_inicial_final.wav",      0.95f);
    effects_["taz_spin"]  = makeEffect("qrc:/sounds/giro_tasmania.wav",              0.85f);
    effects_["taz_dizzy"] = makeEffect("qrc:/sounds/tasmania_despues_del_giro.wav",  0.85f);
    effects_["taz_hit"]   = makeEffect("qrc:/sounds/choque_contra_tasmania.wav",     0.95f);

    music_["menu"]    = makeMusic("qrc:/sounds/looney_tunes_portada.wav",             0.55f);
    music_["nivel1"]  = makeMusic("qrc:/sounds/looney_tunes_primer_level_fondo.wav",  0.50f);

    loaded_ = true;
}

void AudioManager::play(const QString& key) {
    if (!enabled_) return;
    auto it = effects_.find(key);
    if (it != effects_.end() && it->second)
        it->second->play();
}

void AudioManager::playGoal()     { play("goal"); }
void AudioManager::playWhistle()  { play("whistle"); }
void AudioManager::playTazSpin()  { play("taz_spin"); }
void AudioManager::playTazDizzy() { play("taz_dizzy"); }
void AudioManager::playTazHit()   { play("taz_hit"); }

void AudioManager::stopMusic() {
    for (auto& kv : music_)
        if (kv.second) kv.second->stop();
}

void AudioManager::playMenuMusic() {
    if (!enabled_) return;
    stopMusic();
    auto it = music_.find("menu");
    if (it != music_.end()) playWhenReady(it->second);
}

void AudioManager::playNivel1Music() {
    if (!enabled_) return;
    stopMusic();
    auto it = music_.find("nivel1");
    if (it != music_.end()) playWhenReady(it->second);
}
