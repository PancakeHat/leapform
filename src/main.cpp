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
#include <stdio.h>
#include <stdlib.h>
#include "fileio.hpp"
#include "errors.hpp"
#include "menu.hpp"

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, 
    bool& jumping, int& downVelocity, Map& currentMap, std::vector<Entity>& entities, bool& poweredUp, bool& physicsToggle);
void debugDrawPlayerColliders(Vector2 playerpos);

std::vector<Tile> *tiles = new std::vector<Tile>;
std::vector<Sprite> sprites;
std::vector<Sprite> backgrounds;
std::vector<Entity> entities;
std::vector<Map> maps;
std::vector<Pack> packs;
std::vector<GameSound> sounds;
Pack loadedPack;
Map loadedMap;

ErrorHandler errors;

int fadeoutCounter;

bool inMapEditor = false;
bool inMainMenu = true;
bool debugMode = false;
bool pauseMenu = false;

int frames = 0;

int main()
{
    ForceErrorHandlerOverride(true, errors);
    std::cout << "GAME: Starting game\n";
    SetTraceLogLevel(TraceLogLevel::LOG_ERROR);

    RegisterAllPacks("./packs", packs, errors);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "window");
    InitAudioDevice();
    SetTargetFPS(60);

    LoadPackToGame("Default Pack", packs, maps, sprites, backgrounds, sounds, loadedPack, errors);

    rlImGuiSetup(true);

    SetExitKey(-1);
    EditorInit();
    MenuInit(inMainMenu, packs);

    srand(time(0));

    // Load first map
    LoadMap(loadedPack.firstMapID, maps, tiles, entities, loadedMap, errors);

    Vector2 position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
    Vector2 playerVelocity = {0, 0};

    bool playerJumping = false;
    bool canJump = false;
    int jumpTime = 0;
    int downVelocity = 0;
    bool poweredUp = false;
    bool playerPhysics = true;
    int jumpKey = KEY_UP;

    char debugBuf[32] = "";
    char packnameBuf[48] = "";
    std::string debugLoadMapID;
    std::string debugLoadPackName;

    bool forceCloseGame = false;

    // the texture we render to then scale
    RenderTexture2D screen = LoadRenderTexture(SCREENWIDTH, SCREENHEIGHT);

    while(!WindowShouldClose())
    {

        if(forceCloseGame)
        {
            break;
        }

        if(!inMapEditor && !inMainMenu)
        {
            if(!errors.activeError)
            {
                if(!pauseMenu)
                {
                    if(menuLoadPack)
                    {
                        LoadPackToGame(modPackName, packs, maps, sprites, backgrounds, sounds, loadedPack, errors);
                        LoadMap(loadedPack.firstMapID, maps, tiles, entities, loadedMap, errors);
                        position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        menuLoadPack = false;
                        menuOpenSync = false;
                    }

                    playerVelocity = {0, 0};
                    if(!ImGui::GetIO().WantCaptureMouse)
                    {
                        if(playerPhysics)
                        {
                            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) { position.x += 4; playerVelocity.x = 1; }
                            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) { position.x -= 4; playerVelocity.x = -1; }
                            if (IsKeyDown(KEY_UP)) { if(canJump) { playerJumping = true; jumpTime = 0; jumpKey = KEY_UP; PlaySound(GetSoundFromVector("jump", sounds).sound); } }
                            if (IsKeyDown(KEY_W)) { if(canJump) { playerJumping = true; jumpTime = 0; jumpKey = KEY_W; PlaySound(GetSoundFromVector("jump", sounds).sound); } }
                            if (IsKeyUp(jumpKey)) {if(playerJumping && jumpTime >= 24) {playerJumping = false; downVelocity = 0; } }
                        }
                        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
                        {
                            if(poweredUp)
                            {
                                PlaySound(GetSoundFromVector("platform", sounds).sound);
                                entities.push_back({2, {position.x - 80, position.y + 45}, {200, 40}, 0});
                                poweredUp = false;
                            }
                        }
                        if (IsKeyPressed(KEY_R)) 
                        { 
                            LoadMap(loadedMap.mapID, maps, tiles, entities, loadedMap, errors); 
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40 - 40}; 
                            poweredUp = false;
                        }
                        if (IsKeyPressed(KEY_GRAVE)) { debugMode = !debugMode; }
                        if (IsKeyPressed(KEY_M)) { inMapEditor = true; SetWindowTitle("Map Editor"); }
                        if (IsKeyPressed(KEY_ESCAPE)) { pauseMenu = true; pauseMenuSync = true; }
                    }

                    if(playerPhysics)
                    {
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
                    }

                    for(int i = 0; i < entities.size(); i++)
                    {
                        entities[i].ticks++;
                    }

                    checkPlayerWorldCollsions(*tiles, position, playerVelocity, canJump, playerJumping, downVelocity, loadedMap, entities, poweredUp, playerPhysics);

                }
                else
                {
                    if (IsKeyPressed(KEY_ESCAPE)) { pauseMenu = false; pauseMenuSync = false; }
                }

                // BeginDrawing();
                BeginTextureMode(screen);
                    ClearBackground(GRAY);

                    DrawSpriteFromVector(loadedMap.backgroundID, {0, 0}, {800, 600}, backgrounds);

                    renderTiles(*tiles, sprites);
                    RenderEntities(entities, sprites);

                    if(!poweredUp)
                        DrawSpriteFromVector("player", position, {40, 40}, sprites);
                    else
                        DrawSpriteFromVector("player_powered", position, {40, 40}, sprites);

                    if(IndexOfFirstEntityOfType(6, entities) != -1 && entities[IndexOfFirstEntityOfType(6, entities)].target.y > 0 && !pauseMenu)
                    {
                        Entity e = entities[IndexOfFirstEntityOfType(6, entities)];
                        int alpha = 255;
                        if(e.target.y <= 100)
                            alpha = (int)(255 - (255 - (e.target.y * 2.55)));
                        DrawSpriteFromVectorAlpha("shield", {e.pos.x - 20, e.pos.y - 40}, {120, 120}, sprites, alpha);
                    }

                    if(fadeoutCounter > 0)
                        DrawRectangle(0, 0, SCREENWIDTH, SCREENHEIGHT, Color{0, 0, 0, (unsigned char)(fadeoutCounter)});

                    if(pauseMenu)
                    {
                        PauseMenu(pauseMenu, inMainMenu, GetSoundFromVector("select", sounds));
                    }

                    if(debugMode)
                    {
                        debugDrawPlayerColliders(position);
                        DrawText(std::format("Position: {}, {}", position.x, position.y).c_str(), 10, 10, 16, BLACK);
                        DrawText(std::format("Velocity: {}, {}", playerVelocity.x, playerVelocity.y).c_str(), 10, 25, 16, BLACK);
                        DrawText(std::format("JumpTime / FallTime: {} / {}", jumpTime, downVelocity).c_str(), 10, 40, 16, BLACK);
                        DrawText(std::format("Map ID: {}", loadedMap.mapID).c_str(), 10, 55, 16, BLACK);
                        DrawText(std::format("Entity count: {}", entities.size()).c_str(), 10, 70, 16, BLACK);
                        DrawText(std::format("Player Physics: {}", playerPhysics).c_str(), 10, 85, 16, BLACK);
                        DrawText(std::format("Fadeout Counter: {}", fadeoutCounter).c_str(), 10, 100, 16, BLACK);
                        DrawText(std::format("Background ID: {}", loadedMap.backgroundID).c_str(), 10, 115, 16, BLACK);
                        DrawText(std::format("Frame: {}", frames).c_str(), 10, 130, 16, BLACK);
                    }
                // EndDrawing();
                EndTextureMode();
            }


            BeginDrawing();
                ClearBackground(BLACK);

                // draw scaled render texture to screen
                DrawSpriteDirect(screen.texture, {0, 0}, {(float)GetScreenWidth(), (float)GetScreenHeight()});

                rlImGuiBegin();

                    // error window
                    if(errors.activeError)
                    {
                        ImGui::SetNextWindowSize({0, 0});
                        if(ImGui::Begin(errors.isFatal ? "Fatal Error" : "Nonfatal Error"))   
                        {
                            ImGui::TextColored({255, 0, 0, 255}, errors.errorMessage.c_str());
                            if(ImGui::Button("OK"))
                            {
                                if(errors.isFatal)
                                    forceCloseGame = true;
                                else
                                {
                                    errors.activeError = false;
                                    errors.overridable = true;
                                }
                            }

                            ImGui::End();
                        }
                    }

                    // debug gui
                    if(debugMode)
                    {
                        ImGui::SetNextWindowSize({0, 0});
                        ImGui::Begin("Map Debug");

                        if(ImGui::InputText("Map ID", debugBuf, IM_ARRAYSIZE(debugBuf)))
                        {
                            debugLoadMapID = debugBuf;
                        }
                        if(ImGui::Button("Load Map"))
                        {
                            LoadMap(debugLoadMapID, maps, tiles, entities, loadedMap, errors);
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        }
                        ImGui::Separator();
                        if(ImGui::Button("Refresh Map List"))
                        {
                            RegisterMapsInDir(loadedPack.rootDir.string() + loadedPack.mapDir, maps, errors);
                        }
                        if(ImGui::Button("Reload Current Map"))
                        {
                            RegisterMapsInDir(loadedPack.rootDir.string() + loadedPack.mapDir, maps, errors);
                            LoadMap(loadedMap.mapID, maps, tiles, entities, loadedMap, errors);
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        }

                        ImGui::End();

                        ImGui::SetNextWindowSize({0, 0});
                        ImGui::Begin("Pack Debug");

                        if(ImGui::InputText("Pack Name", packnameBuf, IM_ARRAYSIZE(packnameBuf)))
                        {
                            debugLoadPackName = packnameBuf;
                        }
                        if(ImGui::Button("Load Pack"))
                        {
                            LoadPackToGame(debugLoadPackName, packs, maps, sprites, backgrounds, sounds, loadedPack, errors);
                            LoadMap(loadedPack.firstMapID, maps, tiles, entities, loadedMap, errors);
                            position = {loadedMap.playerSpawn.x * 40, loadedMap.playerSpawn.y * 40};
                        }

                        ImGui::End();
                    }
                rlImGuiEnd();
            EndDrawing();
        }
        else if(inMapEditor)
        {
            Editor(inMapEditor, sprites);
        }
        else if(inMainMenu)
        {
            MainMenu(inMainMenu, forceCloseGame, sprites, backgrounds, GetSoundFromVector("select", sounds));
        }

        frames++;
    }

    CloseAudioDevice();
    CloseWindow();
    delete tiles;
    UnloadSpritesFromVector(sprites);
    UnloadSpritesFromVector(backgrounds);
    UnloadSoundsFromVector(sounds);
    rlImGuiShutdown();
    std::cout << "GAME: Shutting down\n";
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

int RandomInt(int min, int max)
{
    return (rand() % max + min);
}


// dont let the name deceive you, this is actually the game's update function
// it is a pretty huge mess but it gets the job done
void checkPlayerWorldCollsions(std::vector<Tile>& tiles, Vector2& playerpos, Vector2 playervel, bool& canJump, 
    bool& jumping, int& downVelocity, Map& currentMap, std::vector<Entity>& entities, bool& poweredUp, bool& physicsToggle) {
    Vector2 cBottom = {5, 39};
    Vector2 cTop = {5, 0};
    Vector2 cLeft = {0, 5};
    Vector2 cRight = {39, 5};

    if(playerpos.x < 0)
        playerpos.x = 41;
    if(playerpos.x > SCREENWIDTH)
        playerpos.x = SCREENWIDTH - 81;

    for(Tile tile : tiles)
    {
        if(tile.type == 1)
        {
            if(physicsToggle)
            {
                if(checkBoxCollison({cBottom.x + playerpos.x, cBottom.y + playerpos.y}, {30, 1}, tile.pos, {40, 40})) { playerpos.y = tile.pos.y - 40; canJump = true; downVelocity = 0;}
                if(checkBoxCollison({cTop.x + playerpos.x, cTop.y + playerpos.y}, {30, 1}, tile.pos, {40, 40})) { playerpos.y = tile.pos.y + 40; jumping = false; }
                if(checkBoxCollison({cLeft.x + playerpos.x, cLeft.y + playerpos.y}, {1, 30}, tile.pos, {40, 40})) { playerpos.x = tile.pos.x + 40; }
                if(checkBoxCollison({cRight.x + playerpos.x, cRight.y + playerpos.y}, {1, 30}, tile.pos, {40, 40})) { playerpos.x = tile.pos.x - 40; }
            }
        }
        else if(tile.type == 2 || tile.type == 7)
        {
            if(checkBoxCollison(playerpos, {40, 40}, tile.pos, {40, 40}))
            {
                LoadMap(currentMap.nextMapID, maps, &tiles, entities, currentMap, errors);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                poweredUp = false;
            }
        }
        else if(tile.type == 3 || tile.type == 4 || tile.type == 5 || tile.type == 6)
        {
            if(checkBoxCollison(playerpos, {40, 40}, {tile.pos.x + 10, tile.pos.y + 10}, {20, 20}))
            {
                PlaySound(GetSoundFromVector("death", sounds).sound);
                LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap, errors);
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
                    PlaySound(GetSoundFromVector("powerup", sounds).sound);
                    poweredUp = true;
                    entities.erase(std::find(entities.begin(), entities.end(), entities[i]));
                    return;
                }
            }
        }
        else if(entities[i].type == 2)
        {
            if(physicsToggle)
            {
                if(checkBoxCollison({cBottom.x + playerpos.x, cBottom.y + playerpos.y}, {30, 1}, entities[i].pos, entities[i].size)) { playerpos.y = entities[i].pos.y - 40; canJump = true; downVelocity = 0;}
                if(checkBoxCollison({cTop.x + playerpos.x, cTop.y + playerpos.y}, {30, 1}, entities[i].pos, entities[i].size)) { playerpos.y = entities[i].pos.y + 40; jumping = false; }
                if(checkBoxCollison({cLeft.x + playerpos.x, cLeft.y + playerpos.y}, {1, 30}, entities[i].pos, entities[i].size)) { playerpos.x = entities[i].pos.x + 40; }
                if(checkBoxCollison({cRight.x + playerpos.x, cRight.y + playerpos.y}, {1, 30}, entities[i].pos, entities[i].size)) { playerpos.x = entities[i].pos.x - 40; }
            }

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
                PlaySound(GetSoundFromVector("death", sounds).sound);
                LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap, errors);
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
        else if(entities[i].type == 5)
        {
            // only triggered when the map gets loaded
            if(entities[i].ticks == 2)
            {
                entities[i].target.x = 5;
                entities.push_back({7, {-40, 0}, {40, 40}, 0, {(float)RandomInt(0, 120) + 240, 1}});
                entities.push_back({7, {-40, 0}, {40, 40}, 0, {(float)RandomInt(0, 120) + 240, 2}});
            }
            entities[i].pos.y += 2 * sin((entities[i].ticks / 15));

            if(entities[i].target.x == 0)
            {
                if(fadeoutCounter < 255)
                    fadeoutCounter++;
            }

            if(fadeoutCounter >= 255)
            {
                fadeoutCounter = 0;
                inMainMenu = true;
                menuOpenSync = true;
                poweredUp = false;
                physicsToggle = true;
                LoadMap("map0", maps, &tiles, entities, currentMap, errors);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                return;
            }

            if(entities[i].target.y > 0)
                entities[i].target.y--;
        }
        else if(entities[i].type == 6)
        {
            if(entities[i].ticks == 2)
                physicsToggle = true;

            if(entities[i].target.x == 0)
            {
                if(checkBoxCollison(playerpos, {40, 40}, { entities[i].pos.x + 20, entities[i].pos.y + 20 }, entities[i].size))
                {
                    PlaySound(GetSoundFromVector("boss_hurt", sounds).sound);
                    entities[IndexOfFirstEntityOfType(5, entities)].target.x--;
                    entities[IndexOfFirstEntityOfType(5, entities)].target.y = 60;
                    physicsToggle = false;
                    entities[i].target.x = 1;
                    entities[i].target.y = 400;
                }
            }
            else if(entities[i].target.x == 1)
            {
                if(sqrt(pow(playerpos.x - 380, 2) + pow(playerpos.y - 520, 2)) > 8)
                {
                    float deltaX = 380 - playerpos.x;
                    float deltaY = 520 - playerpos.y;
                    float angle = atan2( deltaY, deltaX );
                    playerpos.x += 6 * cos( angle );
                    playerpos.y += 6 * sin( angle );
                }
                else
                {
                    entities[i].target.x = 0;
                    physicsToggle = true;
                }
            }

            if(entities[i].target.y > 0)
            {
                if(physicsToggle)
                {
                    if(CheckBoxCircleCollision({entities[i].pos.x + 40, entities[i].pos.y + 20}, 60, {playerpos.x + 20, playerpos.y + 20}, {40, 40}))
                    {
                        PlaySound(GetSoundFromVector("death", sounds).sound);
                        LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap, errors);
                        playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                        poweredUp = false;
                    }
                }
                entities[i].target.y--;
            }
        }
        else if(entities[i].type == 7)
        {
            if(entities[i].ticks == 2)
            {
                entities.push_back({8, {-200, 0}, {60, -600}, 0, {0, entities[i].target.y}});
            }

            if(entities[i].target.x == 0)
                entities[i].pos.x = -40;

            if(entities[i].target.x == 0 && entities[IndexOfFirstEntityWithTypeAndTargetY(8, entities[i].target.y, entities)].target.x == 0)
            {
                if(RandomInt(1, 3) == 2)
                {
                    int r = RandomInt(120, 240);
                    entities[i].pos.x = -40;
                    entities[i].target.x = 120 + r;
                    entities[IndexOfFirstEntityWithTypeAndTargetY(8, entities[i].target.y, entities)].target.x = 240 + r;
                    entities[IndexOfFirstEntityWithTypeAndTargetY(8, entities[i].target.y, entities)].pos.x = RandomInt(0, 740);
                }
            }

            if(entities[i].target.x <= 120 && entities[i].target.x > 0)
            {
                entities[i].pos.x = entities[IndexOfFirstEntityWithTypeAndTargetY(8, entities[i].target.y, entities)].pos.x + 10;
            }

            if(entities[i].target.x > 0)
                entities[i].target.x--;
        }
        else if(entities[i].type == 8)
        {
            if(entities[i].target.x > 0)
                entities[i].target.x--;
            
            if(entities[i].target.x == 140)
                PlaySound(GetSoundFromVector("laser", sounds).sound); 

            if(entities[i].target.x <= 140 && entities[i].target.x > 0)
            {
                if(entities[i].target.x > 120)
                {
                    entities[i].pos.y += 30;
                }
                if(entities[i].target.x < 20)
                {
                    entities[i].pos.y -= 30;
                }
            }
            else
            {
                entities[i].pos.y = -600;
            }

            if(checkBoxCollison(playerpos, {40, 40}, entities[i].pos, {60, 600}) && physicsToggle && fadeoutCounter <= 1)
            {
                PlaySound(GetSoundFromVector("death", sounds).sound);
                LoadMap(currentMap.mapID, maps, &tiles, entities, currentMap, errors);
                playerpos = {currentMap.playerSpawn.x * 40, currentMap.playerSpawn.y * 40};
                poweredUp = false;
                return;
            }
        }
    }
}