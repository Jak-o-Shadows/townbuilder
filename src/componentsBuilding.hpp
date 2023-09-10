#pragma once

struct Location {
    int x;
    int y;
};

struct Resources {
    int fish;
    int stone;
    int wood;
};


struct BuildingUI {
    int sizeX;
    int sizeY;
    int doorX;
    int doorY;
};

struct Building {};
struct Nature {};