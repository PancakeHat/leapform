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
#include <regex>
#include "errors.hpp"

#pragma once

bool StringEndsIn(std::string str, std::string ending)
{
    for(int i = 0; i < ending.length(); i++)
    {
        if(ending[i] != str[str.length() - ending.length() + i])
            return false;
    }

    return true;
}

bool StringStartsWith(std::string str, std::string beginning)
{
    for(int i = 0; i < beginning.length(); i++)
    {
        if(beginning[i] != str[i])
            return false;
    }

    return true;
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

std::string RemoveFileEnding(std::string name)
{
    std::string n = name;
    for(int i = name.length() - 1; i > 0; i--)
    {
        if(n[i] == '.')
        {
            n.pop_back();
            return n;
        }
        n.pop_back();
    }

    return "";
}

// loads sprites and backgrounds into their respective lists
// this is kind of a complicated function because the pack system needs a fallback if the sprite isnt defined
/* heres an outline of how it works
    

    get list of all assets -> does asset exist in pack
                              | yes -> remove id from default id list and load sprite -- |
                              | no -> dont load sprite --------------------------------- |
                                                                                         v
                                                                        load all ids left in deafult id list


*/
void LoadSpritesFromDir(std::string spriteDir, std::vector<Sprite>& sprites, std::vector<Sprite>& backgrounds, ErrorHandler& eh)
{
    // make path look pretty
    std::replace( spriteDir.begin(), spriteDir.end(), '\\', '/');

    // check if using default assets
    if(StringEndsIn(spriteDir, "#default"))
        spriteDir = "./assets";

    // check if dirs exist
    if(!std::filesystem::exists(std::filesystem::path(spriteDir)))
    {
        ThrowNewError(std::format("Can't find asset directory {}", spriteDir), ERROR_FATAL, false, eh); 
        return;
    }

    if(!std::filesystem::exists(std::filesystem::path((spriteDir + "/backgrounds"))))
    {
        ThrowNewError(std::format("Can't find background directory {}", spriteDir + "/backgrounds"), ERROR_FATAL, false, eh); std::cout << "errd";
        return;
    }


    sprites.clear();
    backgrounds.clear();

    std::vector<std::string> defaultIDs;

    // make a list of all default sprites
    for(const auto & entry : std::filesystem::directory_iterator("./assets"))
    {
        std::string s = entry.path().string();
        std::replace( s.begin(), s.end(), '\\', '/');
        if(StringEndsIn(s, ".png"))
        {
            std::string id = s;
            id.erase(id.begin(), id.begin() + 9);
            id = RemoveFileEnding(id);
            defaultIDs.push_back(id);
        }
    }

    // load sprites from their directory
    for(const auto & entry : std::filesystem::directory_iterator(spriteDir))
    {
        std::string s = entry.path().string();
        std::replace( s.begin(), s.end(), '\\', '/');
        if(StringEndsIn(s, ".png"))
        {
            std::string id = s;
            id.erase(id.begin(), id.begin() + (spriteDir + "/").length());
            id = RemoveFileEnding(id);
            for(int i = 0; i < defaultIDs.size(); i++)
            {
                if(defaultIDs[i] == id)
                {
                    defaultIDs.erase(defaultIDs.begin() + i);
                }
            }
            LoadSpriteToVector(s, id, sprites, eh);
            std::cout << "GAME: Registered sprite " << id << " (from pack)\n";
        }
    }

    for(std::string s : defaultIDs)
    {
        LoadSpriteToVector(std::format("./assets/{}.png", s), s, sprites, eh);
        std::cout << "GAME: Registered sprite " << s << " (default)\n";
    }

    // load backgrounds from their directory
    std::string backgroundDir = spriteDir + "/backgrounds";
    for(const auto & entry : std::filesystem::directory_iterator(backgroundDir))
    {
        std::string s = entry.path().string();
        std::replace( s.begin(), s.end(), '\\', '/');
        if(StringEndsIn(s, ".png"))
        {
            std::string id = s;
            id.erase(id.begin(), id.begin() + (backgroundDir + "/").length());
            id = RemoveFileEnding(id);
            LoadSpriteToVector(s, id, backgrounds, eh);
            std::cout << "GAME: Registered background " << id << "\n";
        }
    }
}

Pack LoadPackFromFile(std::string fileName, ErrorHandler& eh)
{
    std::replace( fileName.begin(), fileName.end(), '\\', '/');

    Pack pack;

    if(!std::filesystem::exists(std::filesystem::path(fileName)))
    {
        ThrowNewError(std::format("Can't find pack at {}", fileName), ERROR_FATAL, false, eh); 
        return pack;
    }


    std::ifstream file(fileName);
    if(!file.is_open())
    {
        std::cerr << fileName << "\n";
        return pack;
    }

    std::getline(file, pack.name);
    std::getline(file, pack.mapDir);
    std::getline(file, pack.assetDir);
    std::getline(file, pack.firstMapID);

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
    return LoadMapFromFile(std::format("./{}.map", fileName));
}

void RegisterMapsInDir(std::string mapDir, std::vector<Map>& maps, ErrorHandler& eh)
{
    std::replace( mapDir.begin(), mapDir.end(), '\\', '/');

    if(StringEndsIn(mapDir, "#default"))
        mapDir = "./maps";

    if(!std::filesystem::exists(std::filesystem::path(mapDir)))
    {
        ThrowNewError(std::format("Can't find map directory {}", mapDir), ERROR_FATAL, false, eh); 
        return;
    }

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

void RegisterAllPacks(std::string packDir, std::vector<Pack>& packs, ErrorHandler& eh)
{
    if(!std::filesystem::exists(std::filesystem::path(packDir)))
    {
        ThrowNewError(std::format("Pack directory {} does not exist", packDir), ERROR_FATAL, false, eh);
        return;
    }

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
                packs.push_back(LoadPackFromFile(s, eh));
                std::cout << "GAME: Registered pack " << packs.back().name << "\n";
            }
        }
    }
}

void LoadPackToGame(std::string packName, std::vector<Pack> packs, std::vector<Map>& maps, std::vector<Sprite>& sprites, std::vector<Sprite>& backgrounds, Pack& currentPack, ErrorHandler& eh)
{
    for(Pack p : packs)
    {
        if(p.name == packName)
        {
            std::cout << "GAME: Loading pack " << p.name << "\n";
            LoadSpritesFromDir(p.rootDir.string() + p.assetDir, sprites, backgrounds, eh);
            RegisterMapsInDir(p.rootDir.string() + p.mapDir, maps, eh);
            currentPack = p;
            return;
        }
    }

    ThrowNewError(std::format("Failed to find pack with name {}", packName), ERROR_FATAL, false, eh);
}