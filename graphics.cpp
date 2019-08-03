#include "graphics.h"
#include "ui_graphics.h"

// old version

Graphics::Graphics(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Graphics)
{

    ui->setupUi(this);
    delete ui->mainToolBar;
    //QSize q = qApp->screens()[0]->size();
    setGeometry(0,0, WIDTH, HEIGHT);
    setMaximumHeight(HEIGHT);
    setMinimumHeight(HEIGHT);
    setMaximumWidth(WIDTH);
    setMinimumWidth(WIDTH);

    byt = SYTILES + 2;
    bxt = SXTILES + 2;

    redraw = new QTimer(this);
    connect(redraw, SIGNAL(timeout()), this, SLOT(repaint()));
    redraw->start(1000 / FRAMERATE);

    loadToBuffer(0, byt, 0, bxt);

    dayLength = 1000;
    startupLength = 2500;
    startupTime = 0;
    lightChange = 0;
    lightDir = 1;
    delay = FRAMERATE / (TSIZE);
    delayTime = 0;

    for (short i = 0; i < WIDTH; ++i)
        wps[i] = static_cast<short>(rand()) % HEIGHT;
    vfx = 0;
    vfx2 = 0;

    m = 0;
    frames = 0;
    sub80 = plus100 = expected = 0;

    frameSkip = 0;
    releaseGuard = -1;

}

void Graphics::changeLight() {
    lightChange = (lightChange + 1) % dayLength;
    if (!lightChange) {
        if (world.outdoorLight >= 255)
            lightDir = -1;
        else if (world.outdoorLight <= 0)
            lightDir = 1;
        world.outdoorLight += lightDir;
    }
}

void Graphics::weatherStartup() {
    if (vfx > 0) {
        vfx -= 15;
    }
    else if (rand() < (10 * world.wind) && world.weather == 1 && world.startup <= 2) {
        vfx = 255;
    }
    startupTime = (startupTime + 1) % (startupLength / world.startup);
    if (!startupTime) {
        if (world.startup > 1) {
            if (world.startup > 1)
               world.startup /= 2;
            if (world.startup == 1) {
                if (rand() > RAND_MAX / 2)
                    world.windDir = !world.windDir;
                ++world.wind;
            }
            for (short i = 0; i < WIDTH; i += world.startup)
                if (i % (world.startup * 2) != 0) {
                    if ((i / 2) % world.startup == 0)
                        wps[i] = rand() % HEIGHT;
                    else if (i % 3 == 0)
                        wps[i] = 0;
                    else {
                        wps[i] = (rand() % 2) ? i + (rand() % 10) : i - (rand() % 10);
                        wps[i] = (wps[i] + (HEIGHT)) % (HEIGHT);
                    }
                }
        }
        else {
            int num = rand();
            if (num < RAND_MAX / 4 && world.wind > 1)
                --world.wind;
            else if (num > 3 * RAND_MAX / 4 && world.wind < 5 && world.wind > 0)
                ++world.wind;
        }
    }
    if (world.wCycle > 0) {
        --world.wCycle;
        if (world.wCycle - ((5 * world.wind) * FRAMERATE) < 0 && world.wind > 0)
            --world.wind;
    }
    else if (world.weather != 0)
        world.weather = 0;
}

void Graphics::weatherChange() {
    world.wBit = 0;
    world.wind = 0;

    while (world.startup * 2 <= (WIDTH))
        world.startup *= 2;
    if (world.weather == 1) {
        world.fallSpeed = 2;
        world.range = SYTILES * 2;
    }
    else if (world.weather == 2) {
        world.fallSpeed = 1;
        world.range = SYTILES * 2000;
    }
    else{
        world.fallSpeed = 0;
        world.startup = 1;
    }
}


void Graphics::drawWorld(QSize size) {
    QImage toDraw(size, QImage::Format_ARGB32_Premultiplied); // this should be declared inside of our graphics header as a pointer
    uint8_t onScreen;
    uint16_t yEnd = (HEIGHT) + TSIZE;
    uint16_t xEnd = (WIDTH) + TSIZE;
    uint32_t color;
    short ml;
    int trueX;
    int yCalculated, xCalculated;

    int tempVar = 0; // This is only being used to show off the character location when moving
                     // The actual system used will be different.

    if (world.lanternOn) {
        int yTrue = (yEnd + TSIZE) / 2;
        int xTrue = (xEnd + TSIZE) / 2;
        uint16_t rTrue = world.lanternRadius * TSIZE;
        double rDist = rTrue * rTrue;
        int yDist, xDist, Y, X, tempColor, x;
        double len, mergedLight;
        for (int y = TSIZE; y < yEnd; ++y) {
            Y = y - TSIZE;
            yCalculated = ((world.yPlayer * TSIZE) + (TSIZE / 2)) - (((HEIGHT) / 2) - Y);
            if (delayTime > delay)
            {
                uint16_t temp = delayTime - delay;
                if (world.dir == UP)
                    yCalculated += (temp / delay);
                else if (world.dir == DOWN)
                    yCalculated -= (temp / delay);
            }
            for (x = TSIZE; x < xEnd; ++x) {
                X = x - TSIZE;
                trueX = X;
                if ((Y / TSIZE) == (yTrue / TSIZE) - 1 && (X / TSIZE) == (xTrue / TSIZE) - 1)
                    tempVar = 1;
                else
                    tempVar = 0;
                xDist = x < xTrue ? xTrue - x : x - xTrue;
                yDist = y < yTrue ? yTrue - y : y - yTrue;
                len = (xDist * xDist) + (yDist * yDist);
                xCalculated = ((world.xPlayer * TSIZE) + (TSIZE / 2)) - (((WIDTH) / 2) - X);
                if (delayTime > delay)
                {
                    uint16_t temp = delayTime - delay;
                    if (world.dir == LEFT)
                        xCalculated += (temp / delay);
                    else if (world.dir == RIGHT)
                        xCalculated -= (temp / delay);
                }
                onScreen = yCalculated >= 0 && yCalculated < ((world.height) * TSIZE) && xCalculated >= 0 && xCalculated < ((world.width) * TSIZE);
                if (onScreen)
                    mergedLight = world.lightLayer[yCalculated][xCalculated];
                else
                    mergedLight = 0;
                if (len <= rDist) {
                    double d = (255 - (255 * (len / rDist)));
                    double e = (mergedLight/ (rDist / len)) + d;
                    if (e > mergedLight && e > d)
                        mergedLight = e;
                    else if (d > mergedLight)
                        mergedLight = d;
                }
                mergedLight += world.isOutdoors ? world.outdoorLight + vfx: world.indoorLight;
                if (mergedLight > 255)
                    mergedLight = 255;
                color = 0xFF000000;
                if (delayTime > 0 && world.startup > 1) {
                    //the below can be put into vars before the loops occur so that it does not
                    // have to do the calculations each cycle
                    if (world.dir == LEFT)
                        trueX = (X + (WIDTH) - (((TSIZE * delay) - delayTime) / delay)) % (WIDTH);
                    else if (world.dir == RIGHT)
                        trueX = (X + (WIDTH) + (((TSIZE * delay) - delayTime) / delay)) % (WIDTH);
                }
                for (uint8_t c = 0; c < 3; ++c) {
                    tempColor = mapLayer[y][x][c];
                    int a = 2*((frames % 255) - 128);
                    if (a < 0)
                        a = -a;
                    a += tempColor;
                    if (a > 255)
                        a = 255;
                    if (tempVar && (c == 1 || c == 2))
                        tempColor = a;
                    ml = static_cast<short>(mergedLight);
                    if (wps[trueX] == Y && onScreen && world.weather && trueX % world.startup == 0) {
                        if (world.weather == 1)
                            tempColor = (tempColor + 255) / 2;
                        else
                            tempColor = ml;
                    }
                    //dull
                   // tempColor = (tempColor + 128) / 2;


                    //lighting color
//                    if (c == 0 && mergedLight > world.outdoorLight)
//                        tempColor -= (255 - static_cast<int>(world.outdoorLight));
//                    else
//                        tempColor -= (255 - static_cast<int>(mergedLight));

//                    layered lighting
//
//                    mergedLight = (int)(mergedLight / (sharpness + 1));
//                    mergedLight *= (sharpness + 1);
//                    tempColor /= (sharpness + 1);
//                    tempColor *= (sharpness + 1);
                    tempColor -= (255 - static_cast<int>(mergedLight));
                    if (tempColor < 0)
                        tempColor = 0;

                    tempColor |= (vfx >> 1);
                    color += static_cast<uint32_t>((tempColor << (8 * c)));
                }
                toDraw.setPixel(X, Y, color);
            }
        }
    } else {
        int yTrue = (yEnd + TSIZE) / 2; // only used for temp positioning
        int xTrue = (xEnd + TSIZE) / 2; // only used for temp positioning
        int Y, X, tempColor, x;
        double mergedLight;
        for (int y = TSIZE; y < yEnd; ++y) {
            Y = y - TSIZE;
            yCalculated = ((world.yPlayer * TSIZE) + (TSIZE / 2)) - (((HEIGHT) / 2) - Y);
            if (delayTime > delay)
            {
                uint16_t temp = delayTime - delay;
                if (world.dir == UP)
                    yCalculated += (temp / delay);
                else if (world.dir == DOWN)
                    yCalculated -= (temp / delay);
            }
            for (x = TSIZE; x < xEnd; ++x) {
                X = x - TSIZE;
                trueX = X;
                if ((Y / TSIZE) == (yTrue / TSIZE) - 1 && (X / TSIZE) == (xTrue / TSIZE) - 1)
                    tempVar = 1;
                else
                    tempVar = 0;
                xCalculated = ((world.xPlayer * TSIZE) + (TSIZE / 2)) - (((WIDTH) / 2) - X);
                if (delayTime > delay)
                {
                    uint16_t temp = delayTime - delay;
                    if (world.dir == LEFT)
                        xCalculated += (temp / delay);
                    else if (world.dir == RIGHT)
                        xCalculated -= (temp / delay);
                }
                onScreen = yCalculated >= 0 && yCalculated < ((world.height) * TSIZE) && xCalculated >= 0 && xCalculated < ((world.width) * TSIZE);
                if (onScreen)
                    mergedLight = world.lightLayer[yCalculated][xCalculated];
                else
                    mergedLight = 0;
                mergedLight += world.isOutdoors ? world.outdoorLight + vfx : world.indoorLight;
                if (mergedLight > 255)
                    mergedLight = 255;
                color = 0xFF000000;
                if (delayTime > 0 && world.startup > 1) {
                    //the below can be put into vars before the loops occur so that it does not have to do the calculations each cycle
                    if (world.dir == LEFT)
                        trueX = (X + (WIDTH) - (((TSIZE * delay) - delayTime) / delay)) % (WIDTH);
                    else if (world.dir == RIGHT)
                        trueX = (X + (WIDTH) + (((TSIZE * delay) - delayTime) / delay)) % (WIDTH);
                }
                for (uint8_t c = 0; c < 3; ++c) {
                    tempColor = mapLayer[y][x][c];
                    int a = 2*((frames % 255) - 128);
                    if (a < 0)
                        a = -a;
                    a += tempColor;
                    if (a > 255)
                        a = 255;
                    if (tempVar && (c == 2))
                        tempColor = a;
                    ml = static_cast<short>(mergedLight);
                    if (wps[trueX] == Y && onScreen && world.weather && trueX % world.startup == 0) {
                        if (world.weather == 1)
                            tempColor = (tempColor + 255) / 2;
                        else
                            tempColor = ml;
                    }
                    tempColor -= (255 - static_cast<int>(mergedLight));
                    if (tempColor < 0)
                        tempColor = 0;
                    tempColor |= (vfx >> 1);
                    color += static_cast<uint32_t>((tempColor << (8 * c)));
                }
                toDraw.setPixel(X, Y, color);
            }
        }
    }
    short h = HEIGHT;
    world.wBit = (world.wBit + 1) % 3;

    for (short i = 0; i < WIDTH; ++i) {
        if (world.weather != 0 && wps[i] > (world.range * ((static_cast<short>(rand()) + world.range) % h)))
            wps[i] = 0;
        else if ((world.weather == 1 || world.wBit == 0) && i % world.startup == 0)
            wps[i] = (wps[i] + world.fallSpeed) % (HEIGHT);
    }
    if (world.windDir == 0) {
        if (world.wind < 4) {
            if (world.wBit < world.wind) {
                short temp = wps[0];
                for (short i = 1; i < WIDTH; ++i)
                    wps[i - 1] = wps[i];
                wps[(WIDTH) - 1] = temp;
            }
        } else {
            for (int8_t i = static_cast<int8_t>(world.wind - 2); i > 0; --i){
                short temp = wps[0];
                for (short j = 1; j < WIDTH; ++j)
                    wps[j - 1] = wps[j];
                wps[(WIDTH) - 1] = temp;
            }
        }
    }
    else {
        if (world.wind < 4) {
            if (world.wBit < world.wind) {
                short temp = wps[(WIDTH) - 1];
                for (short i = (WIDTH) - 2; i >= 0; --i)
                    wps[i + 1] = wps[i];
                wps[0] = temp;
            }
        } else {
            for (int8_t i = static_cast<int8_t>(world.wind - 2); i > 0; --i){
                short temp = wps[(WIDTH) - 1];
                for (short i = (WIDTH) - 2; i >= 0; --i)
                    wps[i + 1] = wps[i];
                wps[0] = temp;
            }
        }
    }


    QPainter toPaint(this);
    toPaint.drawImage(0,0, toDraw);
}

void Graphics::paintEvent(QPaintEvent *event) {

    auto tim = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    m = n;
    n = tim.count();
    long t = (1000 / (n - m));
    //cout << t << " fps" << endl;
    avg += t;
    if (t < 80) {
        ++sub80;
    }
    else if (t > 100) {
        ++plus100;
    }
    else {
        ++expected;
    }
    ++frames;
    if (!frameSkip) {
        ++frameSkip;
        drawWorld((size()));
        return;
    }
    changeLight();
    weatherStartup();
    if (delayTime > 0) {
        movePlayer();
        if (delayTime == 0 && tempKey != world.dir)
            preMove();
    }
    else if (releaseGuard != -1)
        preMove();

    drawWorld(size());
    frameSkip = (frameSkip + 1) % 3;
}

void Graphics::loadToBuffer(int yStart, int yEnd, int xStart, int xEnd) {
    for (int y = yStart; y < yEnd; ++y) {
        int py = world.yPlayer + y - (byt / 2);
        for (int x = xStart; x < xEnd; ++x) {
            int px = world.xPlayer + x - (bxt / 2);
            if (py < 0 || py >= world.height || px < 0 || px >= world.width) {
                for (uint8_t Y = 0; Y < TSIZE; ++Y) {
                    int pmy = y * TSIZE + Y;
                    for (uint8_t X = 0; X < TSIZE; ++X) {
                        int pmx = x * TSIZE + X;
                        for (uint8_t c = 0; c < 3; ++c)
                            mapLayer[pmy][pmx][c] = 0;
                    }
                }
            } else {
                ifstream tile (to_string(world.tileMatrix[py][px]));
                string s;
                for (uint8_t Y = 0; Y < TSIZE; ++Y)
                {
                    int pmy =(y * TSIZE + Y);
                    for (uint8_t X = 0; X < TSIZE; ++X)
                    {
                        int pmx = (x * TSIZE + X);
                        for (int8_t c = 2; c >= 0; --c)
                        {
                            getline(tile, s);
                            mapLayer[pmy][pmx][c] = static_cast<uint8_t>(stoi(s));
                        }
                    }
                }
                tile.close();
            }
        }
    }
}

void Graphics::movePlayer() {
    if ((delayTime % delay) == 0) {
        delayTime -= delay;
        if (delayTime == 0 && world.startup > 1) {
            if (world.dir == LEFT) {
                for (short j = 0; j < TSIZE; ++j) {
                    short temp = wps[(WIDTH) - 1];
                    for (short i = (WIDTH) - 2; i >= 0; --i)
                        wps[i + 1] = wps[i];
                    wps[0] = temp;
                }
            }
            else if (world.dir == RIGHT) {
                for (short j = 0; j < TSIZE; ++j) {
                    short temp = wps[0];
                    for (short i = 1; i < WIDTH; ++i)
                        wps[i - 1] = wps[i];
                    wps[(WIDTH) - 1] = temp;
                }
            }
        }
        if (world.dir == DOWN) {
            for (int16_t y = 1; y < HEIGHT + (2 * TSIZE); ++y) {
                int16_t Y = y - 1;
                for (int16_t x = 0; x < WIDTH + (2 * TSIZE); ++x)
                    for (uint8_t c = 0; c < 3; ++c)
                        mapLayer[Y][x][c] = mapLayer[y][x][c];
            }
            if (delayTime == 0) {
                loadToBuffer(byt - 1, byt, 0, bxt);
            }
            if (world.weather == 1)
                for (short i = 0; i < WIDTH; ++i)
                    if(wps[i] > 0)
                        --wps[i];

        }
        else if (world.dir == UP) {
            for (int16_t y = HEIGHT + (2 * TSIZE) - 2; y >= 0; --y) {
                int16_t Y = y + 1;
                for (int16_t x = 0; x < WIDTH + (2 * TSIZE); ++x)
                    for (uint8_t c = 0; c < 3; ++c)
                        mapLayer[Y][x][c] = mapLayer[y][x][c];
            }
            if (delayTime == 0) {
                loadToBuffer(0, 1, 0, bxt);
            }
            for (short i = 0; i < WIDTH; ++i)
                if(wps[i] < HEIGHT)
                    ++wps[i];
        }
        else if (world.dir == RIGHT) {
            for (int16_t x = 1; x < WIDTH + (2 * TSIZE); ++x) {
                int16_t X = x - 1;
                for (int16_t y = 0; y < HEIGHT + (2 * TSIZE); ++y)
                    for (uint8_t c = 0; c < 3; ++c)
                        mapLayer[y][X][c] = mapLayer[y][x][c];
            }
            if (delayTime == 0) {
                loadToBuffer(0, byt, bxt - 1, bxt);
            }
            if (world.startup == 1) {
                short temp = wps[0];
                for (short i = 1; i < WIDTH; ++i)
                    wps[i - 1] = wps[i];
                wps[(WIDTH) - 1] = temp;
            }
        }
        else if (world.dir == LEFT) {
            for (int16_t x = WIDTH + (2 * TSIZE) - 2; x >= 0; --x) {
                int16_t X = x + 1;
                for (int16_t y = 0; y < HEIGHT + (2 * TSIZE); ++y)
                    for (uint8_t c = 0; c < 3; ++c)
                        mapLayer[y][X][c] = mapLayer[y][x][c];
            }
            if (delayTime == 0) {
                loadToBuffer(0, byt, 0, 1);
            }
            if (world.startup == 1) {
                short temp = wps[(WIDTH) - 1];
                for (short i = (WIDTH) - 2; i >= 0; --i)
                    wps[i + 1] = wps[i];
                wps[0] = temp;
            }
        }

    }
}
void Graphics::preMove() {
    if (tempKey == DOWN) {
            int temp = world.yPlayer + 1;
            if (temp < world.height && world.tileMatrix[temp][world.xPlayer] != 1) {
                world.yPlayer = static_cast<int16_t>(temp);
                delayTime = TSIZE * delay;
            }
            world.dir = tempKey;
        } else if (tempKey == UP) {
            int temp = world.yPlayer - 1;
            if (temp >= 0 && world.tileMatrix[temp][world.xPlayer] != 1) {
                world.yPlayer = static_cast<int16_t>(temp);
                delayTime = TSIZE * delay;
            }
            world.dir = tempKey;
        } else if (tempKey == LEFT) {
            int temp = world.xPlayer - 1;
            if (temp >= 0 && world.tileMatrix[world.yPlayer][temp] != 1) {
                world.xPlayer = static_cast<int16_t>(temp);
                delayTime = TSIZE * delay;
            }
            world.dir = tempKey;
        }
        else if (tempKey == RIGHT) {
            int temp = world.xPlayer + 1;
            if (temp < world.width && world.tileMatrix[world.yPlayer][temp] != 1) {
                world.xPlayer = static_cast<int16_t>(temp);
                delayTime = TSIZE * delay;
            }
            world.dir = tempKey;
        }
}


void Graphics::keyPressEvent(QKeyEvent *event) {
    tempKey = event->key();
    if (delayTime > 0)
        return;

    if (tempKey == Qt::Key_E) {
        if (world.outdoorLight < 255 && world.outdoorLight > 0)
            world.outdoorLight += lightDir;
    }
    else if (tempKey == Qt::Key_R) {
        world.weather = (world.weather + 1) % 3;
        weatherChange();
    }
    else if (tempKey == Qt::Key_T) {
        world.weather = 1;
        world.wCycle = 100 * FRAMERATE;
        weatherChange();
    }

    // the collision handeling below is temporary.
    // The actual system used will be different.


    if (tempKey == LANTERN)
        world.lanternOn = !world.lanternOn;

    preMove();
}

void Graphics::keyReleaseEvent(QKeyEvent *event) {
    releaseGuard = event->key();
    if (releaseGuard == tempKey)
        releaseGuard = -1;

}

Graphics::~Graphics() {
    delete ui;
    delete redraw;
    lightDir = 0; // to be changed to static later. remove extern from shared
    lightChange = 0;
    dayLength = 0;
    for (int y = 0; y < HEIGHT + (2 * TSIZE); ++y)
        for (int x = 0; x < WIDTH + (2 * TSIZE); ++x)
            for (int c = 0; c < 3; ++c)
                mapLayer[y][x][c] = 0;
    double f = static_cast<double>(frames);
    cout << "\n\n average fps ________________________: " << (avg / f) << endl;
    cout << " percent of time at sub-normal fps __: " << ((sub80 * 100) / f) << endl;
    cout << " percent of time at normal fps ______: " << ((expected * 100) / f) << endl;
    cout << " percent of time at super-normal fps : " << ((plus100 * 100) / f) << endl;
}
