#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <cstdint>
#include <QKeyEvent>
#include <QPaintEvent>
#include <iostream>
#include <fstream>
#include <QImage>
#include <QScreen>
#include "world.h"
#include <time.h>
#include <chrono>
#include <thread>         // std::this_thread::sleep_for
using std::cout;
using std::endl;
using namespace std::chrono;
using std::chrono::milliseconds;

#define FRAMERATE 100
#define SYTILES 13
#define SXTILES 17
#define HEIGHT SYTILES * TSIZE
#define WIDTH SXTILES * TSIZE

#define UP Qt::Key_W
#define DOWN Qt::Key_S
#define RIGHT Qt::Key_D
#define LEFT Qt::Key_A
#define LANTERN Qt::Key_F



namespace Ui {
class Graphics;
}

class Graphics : public QMainWindow
{
    Q_OBJECT

public:
    explicit Graphics(QWidget *parent = nullptr);
    ~Graphics();
    Ui::Graphics *ui;
    QTimer *redraw;
    World world;
    int8_t lightDir; // to be changed to static later. remove extern from shared
    uint16_t lightChange;
    uint16_t startupTime;
    uint32_t dayLength;
    uint16_t startupLength;
    uint8_t delay;
    uint16_t delayTime;
    uint8_t mapLayer[HEIGHT + (2 * TSIZE)][WIDTH + (2 * TSIZE)][3];
    int byt;
    int bxt;
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void changeLight();
    void drawWorld(QSize size);
    void preMove();
    void movePlayer();
    void loadToBuffer(int yStart, int yEnd, int xStart, int xEnd);
    void weatherChange();
    void weatherStartup();

    short wps[WIDTH];
    int vfx;
    int vfx2;

    long long m, n;
    long sub80;
    long expected;
    long plus100;
    long avg;
    int frames;

    int tempKey;
    int releaseGuard;
    int frameSkip;


private:
};

#endif // GRAPHICS_H
