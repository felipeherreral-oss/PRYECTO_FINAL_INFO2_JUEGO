#include <QApplication>
#include <QSurfaceFormat>
#include "GameWindow.h"
#include "GameExceptions.h"
#include <QMessageBox>

/**
 * @brief Punto de entrada del Nivel 2 — Estadio Intergaláctico.
 *
 *  Configura el formato de superficie para mejor calidad visual,
 *  crea la ventana principal y arranca el event loop de Qt.
 */
int main(int argc, char* argv[]) {
    // Habilitar anti-aliasing global
    QSurfaceFormat fmt;
    fmt.setSamples(4);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("Handball Intergalactic");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("UdeA - Informática II 2026-1");

    // Estilo oscuro para toda la aplicación
    app.setStyle("Fusion");
    QPalette dark;
    dark.setColor(QPalette::Window,    QColor(20, 20, 40));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base,      QColor(10, 10, 25));
    dark.setColor(QPalette::Text,      Qt::white);
    app.setPalette(dark);

    try {
        GameWindow window;
        window.show();
        return app.exec();

    } catch (const GameException& e) {
        QMessageBox::critical(nullptr, "Error crítico",
                              QString("Error irrecuperable:\n%1").arg(e.what()));
        return 1;
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Error crítico",
                              QString("Excepción estándar:\n%1").arg(e.what()));
        return 2;
    }
}
