#include "raylib.h"
#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <filesystem>
#include "errors.hpp"
#include <fstream>
#include "stringutils.hpp"

#pragma once

struct GameSound {
    std::string id;
    Sound sound;
};

void UnloadSoundsFromVector(std::vector<GameSound>& sounds)
{
    for(GameSound s : sounds)
    {
        UnloadSound(s.sound);
    }
}

GameSound GetSoundFromVector(std::string id, std::vector<GameSound> sounds)
{
    for(GameSound s : sounds)
    {
        if(s.id == id)
        {
            return s;
        }
    }

    return {NULL, NULL};
}

void LoadSoundToVector(std::string fileName, std::string id, std::vector<GameSound>& sounds, ErrorHandler& eh)
{
    GameSound sound;
    sound.id = id;

    std::string normalName = fileName;

    Sound i = LoadSound(normalName.c_str()); 

    if(i.frameCount == 0)
    {
        ThrowNewError(std::format("Failed to load sprite {}", id), ERROR_NONFATAL, true, eh);
    }

    sound.sound = i;

    sounds.push_back(sound);
}

void LoadSoundsFromDir(std::string assetDir, std::vector<GameSound>& sounds, ErrorHandler& eh)
{
    // make path look pretty
    std::replace( assetDir.begin(), assetDir.end(), '\\', '/');

    // check if using default assets
    if(StringEndsIn(assetDir, "#default"))
        assetDir = "./assets";

    if(!std::filesystem::exists(std::filesystem::path(assetDir + "/sounds")))
    {
        ThrowNewError(std::format("Can't find sound directory {}", assetDir + "/sounds"), ERROR_FATAL, false, eh);
        return;
    }

    sounds.clear();

    std::vector<std::string> defaultIDs;

    // make a list of all default sprites
    for(const auto & entry : std::filesystem::directory_iterator("./assets/sounds"))
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
    for(const auto & entry : std::filesystem::directory_iterator(assetDir + "/sounds"))
    {
        std::string s = entry.path().string();
        std::replace( s.begin(), s.end(), '\\', '/');
        if(StringEndsIn(s, ".wav"))
        {
            std::string id = s;
            id.erase(id.begin(), id.begin() + (assetDir + "/sounds/").length());
            id = RemoveFileEnding(id);
            for(int i = 0; i < defaultIDs.size(); i++)
            {
                if(defaultIDs[i] == id)
                {
                    defaultIDs.erase(defaultIDs.begin() + i);
                }
            }
            LoadSoundToVector(s, id, sounds, eh);
            std::cout << "GAME: Registered sound " << id << " (from pack)\n";
        }
    }

    for(std::string s : defaultIDs)
    {
        LoadSoundToVector(std::format("./assets/sounds/{}.wav", s), s, sounds, eh);
        std::cout << "GAME: Registered sound " << s << " (default)\n";
    }
}