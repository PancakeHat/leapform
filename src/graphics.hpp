#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include "errors.hpp"
#include <format>

#pragma once

struct Sprite {
    Texture2D img;
    std::string id;
    int width;
    int height;
};

void LoadSpriteToVector(std::string fileName, std::string id, std::vector<Sprite>& sprites, ErrorHandler& eh)
{
    Sprite sprite;
    sprite.id = id;

    std::string normalName = fileName;

    Image i = LoadImage(normalName.c_str()); 

    if(i.width == 0)
    {
        ThrowNewError(std::format("Failed to load sprite {}", id), ERROR_NONFATAL, true, eh);
    }

    sprite.width = i.width;
    sprite.height = i.height;

    sprite.img = LoadTextureFromImage(i);
    UnloadImage(i);

    sprites.push_back(sprite);
}

// void LoadSpriteToVector(std::string fileName, std::string id, std::vector<Sprite>& sprites)
// {
//     Sprite sprite;
//     sprite.id = id;

//     std::string overrideName = "./override/" + fileName;
//     std::string normalName = "./assets/" + fileName;

//     Image i = LoadImage(overrideName.c_str());
//     if(i.width == 0)
//     {
//         i = LoadImage(normalName.c_str()); 
//         std::cout << "GAME: Registered texture " << normalName << "\n";
//     }
//     else 
//     {
//         std::cout << "GAME: Registered texture " << overrideName << "\n";
//     }

//     sprite.width = i.width;
//     sprite.height = i.height;

//     sprite.img = LoadTextureFromImage(i);
//     UnloadImage(i);

//     sprites.push_back(sprite);
// }

void DrawOutlinedText(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor) {
    DrawText(text, posX - outlineSize, posY - outlineSize, fontSize, outlineColor);
    DrawText(text, posX + outlineSize, posY - outlineSize, fontSize, outlineColor);
    DrawText(text, posX - outlineSize, posY + outlineSize, fontSize, outlineColor);
    DrawText(text, posX + outlineSize, posY + outlineSize, fontSize, outlineColor);
    DrawText(text, posX, posY, fontSize, color);
}

void DrawOutlinedTextEx(const char *text, int posX, int posY, int fontSize, Color color, int outlineSize, Color outlineColor, Font font, float spacing) {
    DrawTextEx(font, text, {(float)posX - (float)outlineSize, (float)posY - (float)outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, {(float)posX + (float)outlineSize, (float)posY - (float)outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, {(float)posX - (float)outlineSize, (float)posY + (float)outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, {(float)posX + (float)outlineSize, (float)posY + (float)outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, {(float)posX, + (float)posY}, fontSize, spacing, color);
}

Sprite GetSpriteFromVector(std::string id, std::vector<Sprite> sprites)
{
    for(Sprite sprite : sprites)
    {
        if(sprite.id == id)
            return sprite;
    }

    return {0, 0, 0, 0};
}

void DrawSpriteFromVector(std::string id, Vector2 position, Vector2 size, std::vector<Sprite> sprites)
{
    if(id == "")
        return;

    Sprite s = GetSpriteFromVector(id, sprites);

    Rectangle src = {0, 0, (float)s.width, (float)s.height};
    Rectangle dest = {position.x, position.y, size.x, size.y};

    DrawTexturePro(s.img, src, dest, {0, 0}, 0, WHITE);
}

void DrawSpriteDirect(Texture2D img, Vector2 position, Vector2 size)
{
    Rectangle src = {0, 0, (float)img.width, (float)-img.height};
    Rectangle dest = {position.x, position.y, size.x, size.y};

    DrawTexturePro(img, src, dest, {0, 0}, 0, WHITE);
}

void DrawSpriteFromVectorRotation(std::string id, Vector2 position, Vector2 size, std::vector<Sprite> sprites, float rotation)
{
    if(id == "")
        return;

    Sprite s = GetSpriteFromVector(id, sprites);

    Rectangle src = {0, 0, (float)s.width, (float)s.height};
    Rectangle dest = {position.x + 20, position.y + 20, size.x, size.y};

    DrawTexturePro(s.img, src, dest, {20, 20}, rotation, WHITE);
}

void DrawSpriteFromVectorAlpha(std::string id, Vector2 position, Vector2 size, std::vector<Sprite> sprites, unsigned char opacity)
{
    if(id == "")
        return;

    Sprite s = GetSpriteFromVector(id, sprites);

    Rectangle src = {0, 0, (float)s.width, (float)s.height};
    Rectangle dest = {position.x, position.y, size.x, size.y};

    DrawTexturePro(s.img, src, dest, {0, 0}, 0, Color{opacity, opacity, opacity, opacity});
}


void UnloadSpritesFromVector(std::vector<Sprite>& sprites)
{
    for(Sprite sprite : sprites)
    {
        UnloadTexture(sprite.img);
    }
}