#include "SpriteManager.h"
#include <QImage>
#include <QTransform>

// ─────────────────────────────────────────────────────────────────────────────
QString SpriteManager::makeKey(const QString& ch, AnimState st) {
    return ch + "_" + QString::number(static_cast<int>(st));
}

QPixmap& SpriteManager::loadSheet(const QString& key, const QString& path) {
    if (sheets_.find(key) == sheets_.end()) {
        QPixmap px(path);
        if (px.isNull())
            throw ResourceLoadException(path.toStdString());
        sheets_[key] = px;
    }
    return sheets_[key];
}

void SpriteManager::addFrames(const QString& key, const QPixmap& sheet,
                               const std::vector<QRect>& rects) {
    auto& vec = frames_[key];
    for (const QRect& r : rects) {
        QPixmap clean = removeBackground(sheet.copy(r));
        vec.push_back(clean);
    }
}

// ── Eliminar fondo gris ───────────────────────────────────────────────────────
QPixmap SpriteManager::removeBackground(const QPixmap& src, QColor bgColor, int tol) {
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    int br = bgColor.red(), bg = bgColor.green(), bb = bgColor.blue();
    for (int y = 0; y < img.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            if (std::abs(qRed(line[x])   - br) < tol &&
                std::abs(qGreen(line[x]) - bg) < tol &&
                std::abs(qBlue(line[x])  - bb) < tol)
                line[x] = qRgba(0, 0, 0, 0);
        }
    }
    return QPixmap::fromImage(img);
}

// ─────────────────────────────────────────────────────────────────────────────
// LOAD ALL
// ─────────────────────────────────────────────────────────────────────────────
void SpriteManager::loadAll() {
    if (loaded_) return;

    // Helper que carga silenciosamente (no lanza si falta el archivo)
    auto tryLoad = [&](const QString& key, const QString& path) -> bool {
        QPixmap px(path);
        if (px.isNull()) {
            qWarning("SpriteManager: no se pudo cargar '%s'", path.toStdString().c_str());
            return false;
        }
        sheets_[key] = px;
        return true;
    };

    // ── Cancha ────────────────────────────────────────────────────────────────
    {
        QPixmap f(":/images/cancha_Looney_Tunes.png");
        if (!f.isNull()) fieldPixmap_ = f;
    }

    // ── Balón ─────────────────────────────────────────────────────────────────
    if (tryLoad("balon_sheet", ":/images/Balon.png")) {
        ballPixmap_ = removeBackground(sheets_["balon_sheet"].copy(7, 0, 114, 115),
                                       QColor(200, 200, 200), 40);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // J1 — jugador_humano_como_Gidsel.png (1407×768)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("j1_sheet", ":/images/jugador_humano_como_Gidsel.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["j1_sheet"];
        addFrames(makeKey("j1", AnimState::SHOOT), sh, {
            {28,  21, 249, 369},
            {1021, 21, 345, 369},
        });
        addFrames(makeKey("j1", AnimState::RUN), sh, {
            {293,  21, 236, 369},
            {555,  21, 443, 369},
            {220, 430,1152, 325},
        });
        addFrames(makeKey("j1", AnimState::IDLE), sh, {
            {50, 430, 131, 325},
        });
        addFrames(makeKey("j1", AnimState::DEFEND), sh, {
            {28,  21, 249, 369},
            {293, 21, 236, 369},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // J2 — Matias_Gidsel_espejo.png (1407×768)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("j2_sheet", ":/images/Matias_Gidsel_espejo.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["j2_sheet"];
        addFrames(makeKey("j2", AnimState::SHOOT), sh, {
            {28,  21, 247, 369},
            {1021, 21, 344, 369},
        });
        addFrames(makeKey("j2", AnimState::RUN), sh, {
            {293,  21, 236, 369},
            {555,  21, 443, 369},
            {217, 430,1154, 325},
        });
        addFrames(makeKey("j2", AnimState::IDLE), sh, {
            {52, 430, 127, 325},
        });
        addFrames(makeKey("j2", AnimState::DEFEND), sh, {
            {28,  21, 247, 369},
            {293, 21, 236, 369},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // ARQUERO HUMANO — Arquero_humano.png (896×1183)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("gk_human_sheet", ":/images/Arquero_humano.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["gk_human_sheet"];
        addFrames(makeKey("gk_human", AnimState::IDLE), sh, {
            {21, 611, 122, 563},
        });
        addFrames(makeKey("gk_human", AnimState::SAVE), sh, {
            {163, 611, 707, 563},
            {19,  30,  846, 571},
        });
        addFrames(makeKey("gk_human", AnimState::RUN), sh, {
            {350, 30, 329, 571},
            {679, 30, 217, 571},
        });
        addFrames(makeKey("gk_human", AnimState::SHOOT), sh, {
            {11,  30, 339, 571},
            {350, 30, 329, 571},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TAZMANIA SIN BALÓN — 2 frames de carrera/idle mirando izquierda
    //   tasmania_izquierda_sin_balon.png (1152×918)
    //   Frame 0: QRect(28,153,606,765)
    //   Frame 1: QRect(666,153,486,765)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("taz_left_sheet", ":/images/tasmania_izquierda_sin_balon.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["taz_left_sheet"];
        addFrames(makeKey("taz", AnimState::IDLE), sh, {
            {28,  153, 606, 765},
        });
        addFrames(makeKey("taz", AnimState::RUN), sh, {
            {28,  153, 606, 765},
            {666, 153, 486, 765},
        });
        addFrames(makeKey("taz", AnimState::DEFEND), sh, {
            {28,  153, 606, 765},
            {666, 153, 486, 765},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TAZMANIA CON BALÓN — 2 frames mirando izquierda
    //   tasmania_izquierda_con_balon.png (1152×918)
    //   Frame 0: QRect(41,159,592,728)
    //   Frame 1: QRect(680,159,439,728)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("taz_ball_sheet", ":/images/tasmania_izquierda_con_balon.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["taz_ball_sheet"];
        addFrames(makeKey("taz", AnimState::SHOOT), sh, {
            {41,  159, 592, 728},
            {680, 159, 439, 728},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TAZMANIA GIRANDO (tornado) — Tasmania_Girando.png (1408×768)
    //   Se conserva del set anterior
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("taz_spin_sheet", ":/images/Tasmania_Girando.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["taz_spin_sheet"];
        addFrames(makeKey("taz", AnimState::SPIN), sh, {
            {169, 82, 444, 653},
            {783, 82, 476, 653},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TAZMANIA MAREADO — Tasmania_Mareado.png (1408×768)
    //   Se conserva del set anterior
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("taz_dizzy_sheet", ":/images/Tasmania_Mareado.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["taz_dizzy_sheet"];
        addFrames(makeKey("taz", AnimState::DIZZY), sh, {
            {42,  47, 709, 688},
            {808, 47, 567, 688},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // BUGS BUNNY SIN BALÓN — Bugs_Bunny_lado_izquierdo_sin_balon.png (1152×918)
    //   Valle en x=677  Y=12-887
    //   Frame 0: QRect(0,12,677,875)  — corriendo
    //   Frame 1: QRect(677,12,475,875) — preparado
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("bugs_left_sheet", ":/images/Bugs_Bunny_lado_izquierdo_sin_balon.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["bugs_left_sheet"];
        addFrames(makeKey("bugs", AnimState::IDLE), sh, {
            {677, 12, 475, 875},
        });
        addFrames(makeKey("bugs", AnimState::RUN), sh, {
            {0,   12, 677, 875},
            {677, 12, 475, 875},
        });
        addFrames(makeKey("bugs", AnimState::DEFEND), sh, {
            {677, 12, 475, 875},
            {0,   12, 677, 875},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // BUGS BUNNY CON BALÓN — Bugs_Bunny_lado_izquierdo.png (1152×918)
    //   Valle en x=677  Y=9-887
    //   Frame 0: QRect(0,9,677,878)  — driblando
    //   Frame 1: QRect(677,9,475,878) — sosteniendo balón
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("bugs_ball_sheet", ":/images/Bugs_Bunny_lado_izquierdo.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["bugs_ball_sheet"];
        addFrames(makeKey("bugs", AnimState::SHOOT), sh, {
            {0,   9, 677, 878},
            {677, 9, 475, 878},
        });
    }

    // ─────────────────────────────────────────────────────────────────────────
    // DAFFY DUCK (arquero rival) — Pato_Lucas_Arquero.png (1408×768)
    // ─────────────────────────────────────────────────────────────────────────
    {
        if (!tryLoad("daffy_sheet", ":/images/Pato_Lucas_Arquero.png")) { loaded_ = true; return; }
        QPixmap& sh = sheets_["daffy_sheet"];
        addFrames(makeKey("gk_enemy", AnimState::IDLE), sh, {
            {92,  17, 144, 253},
            {399, 17, 153, 253},
        });
        addFrames(makeKey("gk_enemy", AnimState::RUN), sh, {
            {614, 17, 166, 253},
            {863, 17, 203, 253},
            {1109,17, 218, 253},
        });
        addFrames(makeKey("gk_enemy", AnimState::SAVE), sh, {
            {403, 300, 680, 445},
            {45,  300, 341, 445},
        });
        addFrames(makeKey("gk_enemy", AnimState::CELEBRATE), sh, {
            {1121,300, 250, 445},
        });
        addFrames(makeKey("gk_enemy", AnimState::DEFEND), sh, {
            {45,  300, 341, 445},
            {403, 300, 680, 445},
        });
    }

    loaded_ = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GET FRAME
// ─────────────────────────────────────────────────────────────────────────────
QPixmap SpriteManager::getFrame(const QString& character, AnimState state,
                                 int frameIdx, QSize size, bool flipH) const {
    if (!loaded_)
        throw InvalidGameStateException("SpriteManager::getFrame antes de loadAll()");

    QString key = makeKey(character, state);
    auto it = frames_.find(key);
    if (it == frames_.end() || it->second.empty()) {
        key = makeKey(character, AnimState::IDLE);
        it  = frames_.find(key);
    }
    if (it == frames_.end() || it->second.empty()) {
        QPixmap empty(size);
        empty.fill(Qt::transparent);
        return empty;
    }

    const auto& vec = it->second;
    int idx = frameIdx % static_cast<int>(vec.size());
    QPixmap px = vec[idx].scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (flipH)
        px = px.transformed(QTransform().scale(-1, 1));
    return px;
}

int SpriteManager::frameCount(const QString& character, AnimState state) const {
    auto it = frames_.find(makeKey(character, state));
    if (it == frames_.end()) return 1;
    return static_cast<int>(it->second.size());
}

QPixmap SpriteManager::getField(QSize size) const {
    if (fieldPixmap_.isNull()) {
        QPixmap empty(size);
        empty.fill(QColor(30, 120, 40));
        return empty;
    }
    return fieldPixmap_.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QPixmap SpriteManager::getBall(QSize size) const {
    if (ballPixmap_.isNull()) {
        QPixmap empty(size);
        empty.fill(Qt::transparent);
        return empty;
    }
    return ballPixmap_.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
