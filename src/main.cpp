#include "raylib.h"
#include <vector>
#include "maps.hpp"
#include <iostream>
#include <cmath>
#include <format>
#include "graphics.hpp"

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, bool& jumping, int& downVelocity, Map& currentMap);
void debugDrawPlayerColliders(Vector2 playerpos);

std::vector<Tile> *tiles = new std::vector<Tile>;
std::vector<Sprite> sprites;
std::vector<Map> maps;
Map loadedMap;

bool debugMode = false;

int main()
{
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "window");
    SetTargetFPS(60);

    // Load sprites
    LoadSpriteToVector("tile_basic.png", "tile_basic", sprites);
    LoadSpriteToVector("player.png", "player", sprites);
    LoadSpriteToVector("door.png", "door", sprites);
    LoadSpriteToVector("spike.png", "spike", sprites);

    // Load maps
    LoadMapToVector(map0, maps);
    LoadMapToVector(map1, maps);
    LoadMapToVector(map2, maps);

    LoadMap("mapZero", maps, tiles, loadedMap);

    Vector2 position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40 - 40};
    Vector2 playerVelocity = {0, 0};

    bool playerJumping = false;
    bool canJump = false;
    int jumpTime = 0;
    int downVelocity = 0;

    while(!WindowShouldClose())
    {
        playerVelocity = {0, 0};
        if (IsKeyDown(KEY_RIGHT)) { position.x += 4; playerVelocity.x = 1; }
        if (IsKeyDown(KEY_LEFT)) { position.x -= 4; playerVelocity.x = -1; }
        if (IsKeyDown(KEY_UP)) { if(canJump) { playerJumping = true; jumpTime = 0; } }
        if (IsKeyUp(KEY_UP)) {if(playerJumping && jumpTime >= 24) {playerJumping = false; downVelocity = 0; } }

        if (IsKeyPressed(KEY_GRAVE)) { debugMode = !debugMode; }

        if(downVelocity > 1) canJump = false;

        if(playerJumping)
        {
            canJump = false;

            int change = 10 - jumpTime / 4;
            if(change < 1) change = 1;
            position.y -= change;
            jumpTime++;

            if(jumpTime == 40)
            {
                playerJumping = false;
                downVelocity = 0;
            }
        }

        if(!playerJumping)
        {
            downVelocity += 1;
            if(downVelocity > 10) downVelocity = 10;
            position.y += downVelocity; 
            playerVelocity.y = 1;
        }

        checkPlayerWorldCollsions(*tiles, position, playerVelocity, canJump, playerJumping, downVelocity, loadedMap);

        // std::cout << position.x << ", " << position.y << "\n";

        BeginDrawing();
            ClearBackground(GRAY);
            renderTiles(*tiles, sprites);
            DrawSpriteFromVector("player", position, {40, 40}, sprites);

            if(debugMode)
            {
                debugDrawPlayerColliders(position);
                DrawText(std::format("Position: {}, {}", position.x, position.y).c_str(), 10, 10, 16, BLACK);
                DrawText(std::format("Velocity: {}, {}", playerVelocity.x, playerVelocity.y).c_str(), 10, 25, 16, BLACK);
                DrawText(std::format("JumpTime / FallTime: {}, {}", jumpTime, downVelocity).c_str(), 10, 40, 16, BLACK);
                DrawText(std::format("Map ID: {}", loadedMap.mapID).c_str(), 10, 55, 16, BLACK);
            }
        EndDrawing();
    }

    CloseWindow();
    delete tiles;
    UnloadSpritesFromVector(sprites);
    return 0;
}

void debugDrawPlayerColliders(Vector2 playerpos)
{
    Vector2 cBottom = {5, 39};
    Vector2 cTop = {5, 0};
    Vector2 cLeft = {0, 5};
    Vector2 cRight = {39, 5};

    DrawRectangle(cBottom.x + playerpos.x, cBottom.y + playerpos.y, 30, 1, RED);
    DrawRectangle(cTop.x + playerpos.x, cTop.y + playerpos.y, 30, 1, ORANGE);
    DrawRectangle(cLeft.x + playerpos.x, cLeft.y + playerpos.y, 1, 30, GREEN);
    DrawRectangle(cRight.x + playerpos.x, cRight.y + playerpos.y, 1, 30, YELLOW);
}

void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, bool& jumping, int& downVelocity, Map& currentMap) {
    Vector2 cBottom = {5, 39};
    Vector2 cTop = {5, 0};
    Vector2 cLeft = {0, 5};
    Vector2 cRight = {39, 5};

    for(Tile tile : tiles)
    {
        if(tile.type == 1)
        {
            if(checkBoxCollison({cBottom.x + playerpos.x, cBottom.y + playerpos.y}, {30, 1}, tile.pos, {40, 40})) { playerpos.y = tile.pos.y - 40; canJump = true; downVelocity = 0;}
            if(checkBoxCollison({cTop.x + playerpos.x, cTop.y + playerpos.y}, {30, 1}, tile.pos, {40, 40})) { playerpos.y = tile.pos.y + 40; jumping = false; }
            if(checkBoxCollison({cLeft.x + playerpos.x, cLeft.y + playerpos.y}, {1, 30}, tile.pos, {40, 40})) { playerpos.x = tile.pos.x + 40; }
            if(checkBoxCollison({cRight.x + playerpos.x, cRight.y + playerpos.y}, {1, 30}, tile.pos, {40, 40})) { playerpos.x = tile.pos.x - 40; }
        }
        else if(tile.type == 2)
        {
            if(checkBoxCollison(playerpos, {40, 40}, tile.pos, {40, 40}))
            {
                LoadMap(currentMap.nextMapID, maps, &tiles, currentMap);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40 - 40};
            }
        }
    }
}