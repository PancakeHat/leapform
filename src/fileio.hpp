#include <stdint.h>
#include "raylib.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <format>
#include <filesystem>
#include <cmath>
#include <fstream>
#include "maps.hpp"

#pragma once

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

Pack LoadPackFromFile(std::string fileName)
{
    Pack pack;

    std::ifstream file(fileName);
    if(!file.is_open())
    {
        std::cerr << fileName << "\n";
        return pack;
    }

    std::getline(file, pack.name);
    std::getline(file, pack.mapDir);
    std::getline(file, pack.assetDir);

    file.close();
    std::filesystem::path filePath = fileName;
    std::filesystem::path rootDir = filePath.parent_path();

    pack.rootDir = rootDir;

    return pack;
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

bool StringEndsIn(std::string str, std::string ending)
{
    for(int i = 0; i < ending.length(); i++)
    {
        if(ending[i] != str[str.length() - ending.length() + i])
            return false;
    }

    return true;
}

void RegisterAllPacks(std::string packDir, std::vector<Pack>& packs)
{
    std::vector<std::string> packDirs;

    for(const auto & entry : std::filesystem::directory_iterator(packDir))
    {
        std::string s = entry.path().string();
        if(std::filesystem::is_directory(s))
            packDirs.push_back(s);
    }

    for(std::string dir : packDirs)
    {
        for(const auto & entry : std::filesystem::directory_iterator(dir))
        {
            std::string s = entry.path().string();
            if(StringEndsIn(s, ".pack"))
            {
                packs.push_back(LoadPackFromFile(s));
                std::cout << "GAME: Loaded pack " << packs.back().name << "\n";
            }
        }
    }
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