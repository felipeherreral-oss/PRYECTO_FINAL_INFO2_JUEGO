#ifndef MENUINICIO_H
#define MENUINICIO_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>

class MenuInicio : public QGraphicsView {
    Q_OBJECT
public:
    MenuInicio(QWidget *parent = nullptr);
    ~MenuInicio();

protected:
    // Detecta cuando se presionan las teclas para iniciar el juego
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene *escena;
};

#endif // MENUINICIO_H
