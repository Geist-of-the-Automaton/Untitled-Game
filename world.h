#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
using std::vector;
using std::cout;
using std::endl;
using std::ifstream;
using std::to_string;
using std::string;

#define TSIZE 16

class LightSource
{
public:
    LightSource(int16_t Y, int16_t X, uint8_t R);
    ~LightSource();
    uint8_t mergeLight(uint8_t fromSource, uint8_t fromLayer);
    uint16_t y;
    uint16_t x;
    uint8_t r;
};

class World
{
public:
    World();
    ~World();
    int clampSource(int pos, int max);
    uint8_t isOutdoors;
    uint8_t outdoorLight;
    uint8_t indoorLight;
    uint16_t **tileMatrix;
    uint8_t **lightLayer;
    vector<LightSource> sources;
    uint8_t numSources;
    uint16_t bg_tile;
    uint8_t lanternOn;
    uint8_t lanternRadius;

    int16_t yPlayer;
    int16_t xPlayer;
    int dir;
    uint16_t height;
    uint16_t width;

    uint8_t weather;
    uint8_t wBit = 0;
    uint8_t fallSpeed;
    uint16_t range;
    uint8_t wind;
    uint8_t windDir;
    uint16_t startup;
    int wCycle;

    int sprint;
};



#endif // WORLD_H
