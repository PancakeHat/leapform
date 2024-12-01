#include <stdint.h>
#include "raylib.h"
#include <vector>
#include <string>
#include "graphics.hpp"
#include <iostream>
#include <algorithm>
#include <format>
#include <filesystem>
#include <cmath>
#include <fstream>

#pragma once

struct Tile {
    uint8_t type;
    Vector2 pos;
};

struct Entity {
    uint8_t type;
    Vector2 pos;
    Vector2 size;
    int ticks;
    Vector2 target;
};

bool operator ==(const Entity& x, const Entity& y)
{
    return (x.pos.x == y.pos.x && x.pos.y == y.pos.y && x.type == y.type);
}

bool operator ==(Entity& x, const Entity& y)
{
    return (x.pos.x == y.pos.x && x.pos.y == y.pos.y && x.type == y.type);
}

bool operator ==(const Tile& x, const Tile& y)
{
    return (x.pos.x == y.pos.x && x.pos.y == y.pos.y && x.type == y.type);
}

bool operator ==(Tile& x, const Tile& y)
{
    return (x.pos.x == y.pos.x && x.pos.y == y.pos.y && x.type == y.type);
}

Vector2 operator *(Vector2& x, int y)
{
    return {x.x * y, x.y * y};
}

Vector2 operator /(Vector2& x, int y)
{
    return {x.x / y, x.y / y};
}

struct Map {
    uint8_t tiles[300];
    Vector2 playerSpawn;
    std::vector<Entity> entities;
    std::string mapID;
    std::string nextMapID;
    std::string backgroundID;
};


void RemoveTileFromVectorByPos(Vector2 pos, std::vector<Tile>& tiles)
{
    for(int i = 0; i < tiles.size(); i++)
    {
        if(tiles[i].pos.x == pos.x && tiles[i].pos.y == pos.y)
        {
            tiles.erase(std::find(tiles.begin(), tiles.end(), tiles[i]));
        } 
    }
}

void RemoveEntityFromVectorByPos(Vector2 pos, std::vector<Entity>& entities)
{
    for(int i = 0; i < entities.size(); i++)
    {
        if(entities[i].pos.x == pos.x && entities[i].pos.y == pos.y)
        {
            entities.erase(std::find(entities.begin(), entities.end(), entities[i]));
        } 
    }
}

bool CheckForPowerupAtPos(Vector2 pos, std::vector<Entity>& entities)
{
    for(int i = 0; i < entities.size(); i++)
    {
        if(entities[i].pos.x == pos.x && entities[i].pos.y == pos.y && entities[i].type == 1)
        {
            return true;
        } 
    }
    
    return false;
}

void MakeMapFromTiles(std::vector<Tile> tiles, Map& map)
{
    for(int i = 0; i < sizeof(map.tiles); i++)
    {
        map.tiles[i] = 0;
    }

    for(Tile t : tiles)
    {
        int index = (20 * (t.pos.y / 40) + (t.pos.x / 40));
        map.tiles[index] = t.type;
    }
}

Entity DeserializeEntity(std::string entityString)
{
    int index = 0;
    std::string buf = "";
    Entity entity;

    while(entityString[index] != ',')
    {
        buf += entityString[index];
        index++;
    }
    entity.type = std::stoi(buf);
    buf = "";
    index++;

    while(entityString[index] != ',')
    {
        buf += entityString[index];
        index++;
    }
    entity.pos.x = std::stoi(buf);
    buf = "";
    index++;

    while(entityString[index] != ',')
    {
        buf += entityString[index];
        index++;
    }
    entity.pos.y = std::stoi(buf);
    buf = "";
    index++;

    while(entityString[index] != ',')
    {
        buf += entityString[index];
        index++;
    }
    entity.size.x = std::stoi(buf);
    buf = "";
    index++;

    while(entityString[index] != ';')
    {
        buf += entityString[index];
        index++;
    }
    entity.size.y = std::stoi(buf);
    buf = "";
    index++;

    entity.ticks = 1; // stop events from insta-triggering
    entity.target = {0, 0};
    return entity;
}

std::string SerializeEntity(Entity entity)
{
    return std::format("{},{},{},{},{};", entity.type, entity.pos.x, entity.pos.y, entity.size.x, entity.size.y);
}

void makeTilesFromMap(Map map, std::vector<Tile> *tiles)
{
    tiles->clear();

    for(int i = 0; i < 15; i++)
    {
        for(int j = 0; j < 20; j++)
        {
            Tile t;
            t.pos.x = j * 40;
            t.pos.y = i * 40;
            t.type = map.tiles[j + (i * 20)];
            tiles->push_back(t);
        }   
    }
}


Map LoadMapFromFile(std::string fileName)
{
    Map map;

    std::string rawTiles;
    std::string px;
    std::string py;
    std::vector<std::string> savedEntities;
    std::string eBuff;

    // std::ifstream file(std::format("./maps/{}.map", fileName));

    std::ifstream file(fileName);
    if(!file.is_open())
    {
        std::cerr << fileName << "\n";
        return map;
    }

    std::getline(file, map.mapID);
    std::getline(file, map.nextMapID);
    std::getline(file, map.backgroundID);
    std::getline(file, px);
    std::getline(file, py);
    std::getline(file, rawTiles);

    while(std::getline(file, eBuff))
    {
        savedEntities.push_back(eBuff);
        eBuff = "";
    }

    file.close();

    map.playerSpawn = {(float)std::stoi(px), (float)std::stoi(py)};

    std::string currentBit = "";
    int virtualIndex = 0;
    for(int i = 0; i < rawTiles.length(); i++)
    {
        if(rawTiles[i] != ',' && rawTiles[i] != ';')
        {
            currentBit += rawTiles[i];
        }
        else if(rawTiles[i] == ';')
        {
            break;
        }
        else if(rawTiles[i] == ',')
        {
            map.tiles[virtualIndex] = std::stoi(currentBit);
            currentBit = "";
            virtualIndex++;
        }
    }

    for(std::string s : savedEntities)
    {
        map.entities.push_back(DeserializeEntity(s));
    }

    return map;
}

Map LoadMapFromFilePathShorthand(std::string fileName)
{
    return LoadMapFromFile(std::format("./maps/{}.map", fileName));
}

void renderTiles(std::vector<Tile> tiles, std::vector<Sprite> sprites)
{

    if(tiles.size() == 0)
        return;

    for(Tile tile : tiles)
    {
        std::string id;
        switch (tile.type)
        {
            case 0: id = ""; break;
            case 1: id = "tile_basic"; break;
            case 2: id = "door"; break;
            case 3: id = "spikeU"; break;
            case 4: id = "spikeR"; break;
            case 5: id = "spikeD"; break;
            case 6: id = "spikeL"; break;
            case 7: id = "boss_door"; break;
            default: id = ""; break;
        }
        if(id == "spikeU" || id == "spikeD" || id == "spikeL" || id == "spikeR")
        {
            DrawSpriteFromVectorRotation("spike", tile.pos, {40, 40}, sprites, (tile.type - 3) * 90);
        }
        else
        {
            DrawSpriteFromVector(id, tile.pos, {40, 40}, sprites);
        }
    }
}

void RenderEntities(std::vector<Entity> entities, std::vector<Sprite> sprites)
{
    for(Entity e : entities)
    {
        std::string id;
        switch (e.type)
        {
            case 0: id = ""; break;
            case 1: id = "powerup"; break;
            case 2: id = "platform"; break;
            case 3: id = "demon"; break;
            case 4: id = "powerup_generator"; break;
            case 5: id = "boss_demon"; break;
            case 6: id = "boss_alter"; break;
            case 7: id = "warning"; break;
            case 8: id = "laser"; break;
            default: id = ""; break;
        }

        if(e.type == 2)
        {
            int alpha;
            int t = e.ticks;

            if(t < 120)
                alpha = 255;
            else
                alpha = 255 - (t - 120);

            if(alpha < 0) { alpha = 0; }

            DrawSpriteFromVectorAlpha(id, e.pos, e.size, sprites, alpha);
        }
        else if(e.type == 3)
        {
            if(e.ticks % 60 <= 20)
                DrawSpriteFromVector("demon_angry", e.pos, e.size, sprites);
            else
                DrawSpriteFromVector("demon", e.pos, e.size, sprites);
        }
        else if(e.type == 5)
        {
            if(e.target.x == 5)
                DrawSpriteFromVector("boss_demon", e.pos, e.size, sprites);
            else if(e.target.x == 4)
                DrawSpriteFromVector("boss_demon", {e.pos.x + (160 - (100 + (e.target.y))) / 2, e.pos.y}, {100 + (e.target.y), 100 + (e.target.y)}, sprites);
            else if(e.target.x == 3)
                DrawSpriteFromVector("boss_demon", {e.pos.x + (160 - (80 + (e.target.y / 3))) / 2, e.pos.y}, {80 + (e.target.y / 3), 80 + (e.target.y / 3)}, sprites);
            else if(e.target.x == 2)
                DrawSpriteFromVector("boss_demon", {e.pos.x + (160 - (60 + (e.target.y / 3))) / 2, e.pos.y}, {60 + (e.target.y / 3), 60 + (e.target.y / 3)}, sprites);
            else if(e.target.x == 1)
                DrawSpriteFromVector("boss_demon", {e.pos.x + (160 - (40 + (e.target.y / 3))) / 2, e.pos.y}, {40 + (e.target.y / 3), 40 + (e.target.y / 3)}, sprites);
            else if(e.target.x == 0)
                DrawSpriteFromVector("boss_demon", {e.pos.x + (160 - (0 + (e.target.y * 0.6f))) / 2, e.pos.y}, {0 + (e.target.y * 0.6f), 0 + (e.target.y * 0.6f)}, sprites);
            else
                DrawSpriteFromVector("boss_demon", e.pos, e.size, sprites);
        }
        else if(e.type == 6)
        {
            DrawSpriteFromVector("boss_alter", {e.pos.x + 20, e.pos.y}, e.size, sprites);
        }
        else if(e.type == 7)
        {
            if(e.ticks % 30 < 15)
                DrawSpriteFromVector("warning", e.pos, e.size, sprites);
        }
        else
        {
            DrawSpriteFromVector(id, e.pos, e.size, sprites);
        }
    }
}

int IndexOfFirstEntityOfType(uint8_t type, std::vector<Entity> entities)
{
    for(int i = 0; i < entities.size(); i++)
    {
        if(entities[i].type == type)
            return i;
    }

    return -1;
}

// long ass function name
int IndexOfFirstEntityWithTypeAndTargetY(uint8_t type, int id, std::vector<Entity> entities)
{
    for(int i = 0; i < entities.size(); i++)
    {
        if(entities[i].target.y == id && entities[i].type == type)
            return i;
    }

    return -1;
}

int LaserFindHandlerWithID(uint8_t id, std::vector<Entity> entities)
{
    for(int i = 0; i < entities.size(); i++)
    {
        if(entities[i].target.y == id && entities[i].type == 8)
            return i;
    }

    return -1;
}

bool checkBoxCollison(Vector2 posA, Vector2 sizeA, Vector2 posB, Vector2 sizeB)
{
    if(posA.x < posB.x + sizeB.x &&
        posA.x + sizeA.x > posB.x &&
        posA.y < posB.y + sizeB.y &&
        posA.y + sizeA.y > posB.y
    ) { return true; }
    else { return false; }
}

bool CheckBoxCircleCollision(Vector2 circleCenter, float circleRadius, Vector2 rectPos, Vector2 rectSize)
{
    Vector2 circleDistance;
    circleDistance.x = abs(circleCenter.x - rectPos.x);
    circleDistance.y = abs(circleCenter.y - rectPos.y);

    if(circleDistance.x > (rectSize.x / 2 + circleRadius)) { return false; }
    if(circleDistance.y > (rectSize.y / 2 + circleRadius)) { return false; }

    if(circleDistance.x <= (rectSize.x / 2)) { return true; }
    if(circleDistance.y <= (rectSize.y / 2)) { return true; }

    float cornerDistance = pow((circleDistance.x - rectSize.x / 2), 2) + pow((circleDistance.y - rectSize.y / 2), 2);
    return (cornerDistance <= (pow(circleRadius, 2)));
}

void LoadMap(std::string mapID, std::vector<Map>& maps, std::vector<Tile> *tiles, std::vector<Entity>& entities, Map &loadedMap)
{
    std::cout << "GAME: Loading map " << mapID << "\n";
    for(Map m : maps)
    {
        if(m.mapID == mapID)
        {
            makeTilesFromMap(m, tiles);
            entities = m.entities;
            loadedMap = m;
        }
    }
}

void LoadMapToVector(Map map, std::vector<Map>& maps)
{
    std::cout << "GAME: Registered map " << map.mapID << "\n";
    maps.push_back(map);
}

void RegisterMapsInDir(std::string mapDir, std::vector<Map>& maps)
{
    maps.clear();

    for(const auto & entry : std::filesystem::directory_iterator(mapDir))
    {
        std::string s = entry.path().string();
        std::replace( s.begin(), s.end(), '\\', '/');
        if(s[s.length() - 4] == '.' && s[s.length() - 3] == 'm' && s[s.length() - 2] == 'a' && s[s.length() - 1] == 'p')
        {
            LoadMapToVector(LoadMapFromFile(s), maps);
        }
    }
}