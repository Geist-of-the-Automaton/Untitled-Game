#include "world.h"

int World::clampSource(int pos, int max) {
    return pos < 0 ? 0 : (pos > max ? max : pos);
}

uint8_t LightSource::mergeLight(uint8_t fromSource, uint8_t fromLayer)
{
    // only for use when lantern is out
    short temp = (255 - fromSource) + fromLayer;
    return temp < 0 ? 0 : (temp > 255 ? 255 : static_cast<uint8_t>(temp));
}

World::World()
{
    lanternOn = 1;
    lanternRadius = 4;
    isOutdoors = 1;
    outdoorLight = 255;
    indoorLight = 255;
    yPlayer = 8;
    xPlayer = 8;
    height = 20;
    width = 20;
    bg_tile = 0;
    weather = 0;
    fallSpeed = 0;
    wBit = 0;
    range = 0;
    wind = 0;
    windDir = 0;
    startup = 1;

    sprint = 0;

    tileMatrix = new uint16_t*[height];
    for (uint8_t i = 0; i < height; ++i)
        tileMatrix[i] = new uint16_t[width];
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            int i = rand() % 255;
            tileMatrix[y][x] = i <= 50;
        }
    int tempX = 5;
    for (int tempY = 8; tempY < 11; ++tempY)
        tileMatrix[tempY][tempX] = tempY - 5;
    for (int tempY = 4; tempY < 7; ++tempY)
        tileMatrix[tempY][tempX] = tempY - 1;
    tempX = 9;
    for (int tempY = 6; tempY < 9; ++tempY)
        tileMatrix[tempY][tempX] = tempY + 3;
    for (int tempY = 10; tempY < 13; ++tempY)
        tileMatrix[tempY][tempX] = tempY - 1;

    for (int tempY = 0; tempY < height; ++tempY)
        for (tempX = 6; tempX < 9; ++tempX)
            tileMatrix[tempY][tempX] = tempX;

    for (int tempY = 17; tempY < height; ++tempY)
        for (tempX = 17; tempX < width; ++tempX)
            tileMatrix[tempY][tempX] = 12;

    numSources = 4;
    sources.push_back(LightSource(5, 5, 3));
    sources.push_back(LightSource(9, 5, 3));
    sources.push_back(LightSource(7, 9, 3));
    sources.push_back(LightSource(11, 9, 3));

    lightLayer = new uint8_t*[height * TSIZE];
    for (uint16_t i = 0; i < height * TSIZE; ++i)
        lightLayer[i] = new uint8_t[width * TSIZE];
    for (int y = 0; y < height * TSIZE; ++y)
        for (int x = 0; x < width * TSIZE; ++x)
            lightLayer[y][x] = 0;

    uint8_t half = TSIZE / 2;
    int ph = height * TSIZE;
    int pw = width * TSIZE;
    for (uint8_t i = 0; i < numSources; ++i) {
        LightSource l = sources.at(i);
        uint16_t yTrue = (l.y * TSIZE) + half;
        uint16_t xTrue = (l.x * TSIZE) + half;
        uint16_t rTrue = l.r * TSIZE;
        double rDist = rTrue * rTrue;
        int yStart = yTrue - rTrue;
        int yEnd = yTrue + rTrue;
        int xStart = xTrue - rTrue;
        int xEnd = xTrue + rTrue;
        yStart = clampSource(yStart, ph);
        yEnd = clampSource(yEnd, ph);
        xStart = clampSource(xStart, pw);
        xEnd = clampSource(xEnd, pw);
        for (int y = yStart; y < yEnd; ++y)
            for (int x = xStart; x < xEnd; ++x) {
                int X = x < xTrue ? xTrue - x : x - xTrue;
                int Y = y < yTrue ? yTrue - y : y - yTrue;
                double len = (X * X) + (Y * Y);
                if (len <= rDist) {
                    double mergedLight = lightLayer[y][x];
                    double d = (255 - (255 * (len / rDist)));
                    double e = (mergedLight/ (rDist / len)) + d;
                    //double e = (mergedLight + d) / (rDist / len);
                    if (e > mergedLight && e > d)
                        mergedLight = e;
                    else if (d > mergedLight)
                        mergedLight = d;


                   // double mergedLight = lightLayer[y][x] + (255 - (255 * (len / rDist)));
                    lightLayer[y][x] = mergedLight < 0 ? 0 : (mergedLight > 255 ? 255 : static_cast<uint8_t>(mergedLight));
                }
            }
    }
}

World::~World() {
    isOutdoors = 0;
    outdoorLight = 0;
    indoorLight = 0;
    yPlayer = 0;
    xPlayer = 0;
    height = 0;
    width = 0;
    bg_tile = 0;
    numSources = 0;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            tileMatrix[y][x] = 0;
    for (uint8_t i = 0; i < height; ++i)
        delete [] tileMatrix[i];
    delete [] tileMatrix;
    for (int y = 0; y < height * TSIZE; ++y)
        for (int x = 0; x < width * TSIZE; ++x)
            lightLayer[y][x] = 0;
    for (uint8_t i = 0; i < height * TSIZE; ++i)
        delete [] lightLayer[i];
    delete [] lightLayer;
    sources.clear();

}

LightSource::LightSource(int16_t Y, int16_t X, uint8_t R) {
    y = Y;
    x = X;
    r = R;
}

LightSource::~LightSource() {
    y = 0;
    x = 0;
    r = 0;
}
