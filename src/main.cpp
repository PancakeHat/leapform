#include "raylib.h"
#include <vector>
#include "maps.hpp"
#include <iostream>
#include <cmath>
#include <format>
#include "graphics.hpp"
#include "rlimgui/rlImGui.h"
#include "rlimgui/imgui.h"
#include "editor.hpp"
#include <filesystem>

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, bool& jumping, int& downVelocity, Map& currentMap, std::vector<Entity>& entities, bool& poweredUp);
void debugDrawPlayerColliders(Vector2 playerpos);

std::vector<Tile> *tiles = new std::vector<Tile>;
std::vector<Sprite> sprites;
std::vector<Entity> entities;
std::vector<Map> maps;
Map loadedMap;

bool inMapEditor = false;
bool debugMode = false;

int main()
{
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "window");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    SetExitKey(-1);
    EditorInit();

    // Load sprites
    LoadSpriteToVector("tile_basic.png", "tile_basic", sprites);
    LoadSpriteToVector("player.png", "player", sprites);
    LoadSpriteToVector("door.png", "door", sprites);
    LoadSpriteToVector("spike.png", "spike", sprites);
    LoadSpriteToVector("powerup.png", "powerup", sprites);
    LoadSpriteToVector("powerup_platform.png", "platform", sprites);
    LoadSpriteToVector("player_powered.png", "player_poweredup", sprites);
    LoadSpriteToVector("boss_door.png", "boss_door", sprites);
    LoadSpriteToVector("demon.png", "demon", sprites);
    LoadSpriteToVector("demon_angry.png", "demon_angry", sprites);
    LoadSpriteToVector("powerup_generator.png", "powerup_generator", sprites);

    // Load maps in maps folder
    RegisterMapsInDir("./maps", maps);

    // Load first map
    LoadMap("map0", maps, tiles, entities, loadedMap);

    Vector2 position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
    Vector2 playerVelocity = {0, 0};

    bool playerJumping = false;
    bool canJump = false;
    int jumpTime = 0;
    int downVelocity = 0;
    bool poweredUp = false;

    char debugBuf[32] = "";
    std::string debugLoadMapID;

    while(!WindowShouldClose())
    {
        if(!inMapEditor)
        {
            playerVelocity = {0, 0};
            if(!ImGui::GetIO().WantCaptureMouse)
            {
                if (IsKeyDown(KEY_RIGHT)) { position.x += 4; playerVelocity.x = 1; }
                if (IsKeyDown(KEY_LEFT)) { position.x -= 4; playerVelocity.x = -1; }
                if (IsKeyDown(KEY_UP)) { if(canJump) { playerJumping = true; jumpTime = 0; } }
                if (IsKeyUp(KEY_UP)) {if(playerJumping && jumpTime >= 24) {playerJumping = false; downVelocity = 0; } }
                if (IsKeyDown(KEY_DOWN))
                {
                    if(poweredUp)
                    {
                        entities.push_back({2, {position.x - 80, position.y + 45}, {200, 40}, 0});
                        poweredUp = false;
                    }
                }
                if (IsKeyPressed(KEY_R)) 
                { 
                    LoadMap(loadedMap.mapID, maps, tiles, entities, loadedMap); 
                    position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40 - 40}; 
                    poweredUp = false;
                }
                if (IsKeyPressed(KEY_GRAVE)) { debugMode = !debugMode; }
                if (IsKeyPressed(KEY_M)) { inMapEditor = true; }
            }

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

            for(int i = 0; i < entities.size(); i++)
            {
                entities[i].ticks++;
            }

            checkPlayerWorldCollsions(*tiles, position, playerVelocity, canJump, playerJumping, downVelocity, loadedMap, entities, poweredUp);

            BeginDrawing();
                ClearBackground(GRAY);
                renderTiles(*tiles, sprites);
                RenderEntities(entities, sprites);
                if(!poweredUp)
                    DrawSpriteFromVector("player", position, {40, 40}, sprites);
                else
                    DrawSpriteFromVector("player_poweredup", position, {40, 40}, sprites);

                if(debugMode)
                {
                    debugDrawPlayerColliders(position);
                    DrawText(std::format("Position: {}, {}", position.x, position.y).c_str(), 10, 10, 16, BLACK);
                    DrawText(std::format("Velocity: {}, {}", playerVelocity.x, playerVelocity.y).c_str(), 10, 25, 16, BLACK);
                    DrawText(std::format("JumpTime / FallTime: {} / {}", jumpTime, downVelocity).c_str(), 10, 40, 16, BLACK);
                    DrawText(std::format("Map ID: {}", loadedMap.mapID).c_str(), 10, 55, 16, BLACK);
                    DrawText(std::format("Entity count: {}", entities.size()).c_str(), 10, 70, 16, BLACK);
                }

                rlImGuiBegin();
                    if(debugMode)
                    {
                        ImGui::Begin("Map Options");

                        if(ImGui::InputText("Map ID", debugBuf, IM_ARRAYSIZE(debugBuf)))
                        {
                            debugLoadMapID = debugBuf;
                        }
                        if(ImGui::Button("Load Map"))
                        {
                            LoadMap(debugLoadMapID, maps, tiles, entities, loadedMap);
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        }
                        ImGui::Separator();
                        if(ImGui::Button("Refresh Map List"))
                        {
                            RegisterMapsInDir("./maps", maps);
                        }
                        if(ImGui::Button("Reload Current Map"))
                        {
                            RegisterMapsInDir("./maps", maps);
                            LoadMap(loadedMap.mapID, maps, tiles, entities, loadedMap);
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        }

                        ImGui::End();
                    }
                rlImGuiEnd();
            EndDrawing();
        }
        else
        {
            Editor(inMapEditor, sprites);
        }
    }

    CloseWindow();
    delete tiles;
    UnloadSpritesFromVector(sprites);
    rlImGuiShutdown();
    return 0;
}

int WinMain()
{
    return main();
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

void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, bool& jumping, int& downVelocity, Map& currentMap, std::vector<Entity>& entities, bool& poweredUp) {
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
        else if(tile.type == 2 || tile.type == 7)
        {
            if(checkBoxCollison(playerpos, {40, 40}, tile.pos, {40, 40}))
            {
                LoadMap(currentMap.nextMapID, maps, &tiles, entities, currentMap);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
            }
        }
        else if(tile.type == 3 || tile.type == 4 || tile.type == 5 || tile.type == 6)
        {
            if(checkBoxCollison(playerpos, {40, 40}, tile.pos, {40, 40}))
            {
                LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                poweredUp = false;
            }
        }
    }

    int n = entities.size();
    for(int i = 0; i < n; i++)
    {
        if(entities[i].type == 1)
        {
            if(checkBoxCollison(playerpos, {40, 40}, entities[i].pos, entities[i].size))
            {
                if(!poweredUp)
                {
                    poweredUp = true;
                    entities.erase(std::find(entities.begin(), entities.end(), entities[i]));
                    return;
                }
            }
        }
        else if(entities[i].type == 2)
        {
            if(checkBoxCollison({cBottom.x + playerpos.x, cBottom.y + playerpos.y}, {30, 1}, entities[i].pos, entities[i].size)) { playerpos.y = entities[i].pos.y - 40; canJump = true; downVelocity = 0;}
            if(checkBoxCollison({cTop.x + playerpos.x, cTop.y + playerpos.y}, {30, 1}, entities[i].pos, entities[i].size)) { playerpos.y = entities[i].pos.y + 40; jumping = false; }
            if(checkBoxCollison({cLeft.x + playerpos.x, cLeft.y + playerpos.y}, {1, 30}, entities[i].pos, entities[i].size)) { playerpos.x = entities[i].pos.x + 40; }
            if(checkBoxCollison({cRight.x + playerpos.x, cRight.y + playerpos.y}, {1, 30}, entities[i].pos, entities[i].size)) { playerpos.x = entities[i].pos.x - 40; }

            if(entities[i].ticks >= 375)
            {
                entities.erase(std::find(entities.begin(), entities.end(), entities[i]));
                return;
            }
        }
        else if(entities[i].type == 3)
        {
            if(checkBoxCollison(playerpos, {40, 40}, entities[i].pos, entities[i].size))
            {
                LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                poweredUp = false;
                return;
            }

            if(sqrt(pow(entities[i].target.x - entities[i].pos.x, 2) + pow(entities[i].target.y - entities[i].pos.y, 2)) > 4 && entities[i].target.y != 0)
            {
                float deltaX = entities[i].target.x - entities[i].pos.x;
                float deltaY = entities[i].target.y - entities[i].pos.y;
                float angle = atan2( deltaY, deltaX );
                entities[i].pos.x += 4 * cos( angle );
                entities[i].pos.y += 4 * sin( angle );
            }

            if(entities[i].ticks % 180 == 0)
            {
                entities[i].target = playerpos;
            }
        }
        else if(entities[i].type == 4)
        {
            if(!CheckForPowerupAtPos(entities[i].pos, entities))
                entities[i].target.x++;

            if((int)entities[i].target.x % 240 == 0)
            {
                if(!CheckForPowerupAtPos(entities[i].pos, entities))
                {
                    entities.push_back({1, entities[i].pos, {40, 40}, 0, {0, 0}});
                    entities[i].target.x = 0;
                }
            }
        }
    }
}