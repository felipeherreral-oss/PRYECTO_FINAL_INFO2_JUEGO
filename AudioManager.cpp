#include "AudioManager.h"
#include <QSoundEffect>
#include <QUrl>

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

void AudioManager::loadAll() {
    if (loaded_) return;

    effects_["goal"]      = makeEffect("qrc:/sounds/gol_anotado.wav",                0.95f);
    effects_["whistle"]   = makeEffect("qrc:/sounds/silbato_inicial_final.wav",      0.95f);
    effects_["taz_spin"]  = makeEffect("qrc:/sounds/giro_tasmania.wav",              0.85f);
    effects_["taz_dizzy"] = makeEffect("qrc:/sounds/tasmania_despues_del_giro.wav",  0.85f);
    effects_["taz_hit"]   = makeEffect("qrc:/sounds/choque_contra_tasmania.wav",     0.95f);

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
