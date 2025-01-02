#include "raylib.h"
#include <vector>
#include "maps.hpp"
#include <iostream>
#include <cmath>
#include <format>
#include "graphics.hpp"
#include "rlimgui/rlImGui.h"
#include "rlimgui/imgui.h"
#include "rlimgui/imgui_stdlib.h"
#include <fstream>
#include <filesystem>
#include "fileio.hpp"

#pragma once

void CreateNewMap();
void PlaceTile();
void RemoveTile();
void CreateMapWindow();
void TileSelectorWindow();
void PickSpawnPoint();
void EntityWindow();
void PlaceEntity();
void RemoveEntity();
void HelpWindow();
void SaveWindow();
void LoadWindow();
void SaveMap(std::string fileName);
void InfoWindow();
void SetAllArrayFalse(bool *array, int length);
void CreatePackWindow(std::vector<Pack> &packs);
void EditPackWindow(std::vector<Pack> &packs);

bool createMapWindow = false;
bool tileSelectorWindow = false;
bool pickingSpawnPoint = false;
bool entityWindow = false;
bool helpWindow = false;
bool saveWindow = false;
bool loadWindow = false;
bool infoWindow = false;
bool createPackWindow = false;
bool editPackWindow = false;
bool packLoadingError = false;

// 0 = tiles
// 1 = entities
bool placingMode = 0;
bool mapOpen = false;

std::string buf1 = "";
std::string buf2 = "";
std::string buf3 = "";

std::string packName = "";
std::string oldPackName = "";
std::string loadPackName = "";
bool packDefaultAssets = false;
bool packDefaultMaps = false;
bool oldPackDefaultAssets = false;
bool oldPackDefaultMaps = false;
std::string packFirstMap = "";
std::filesystem::path editingPackRootDir = "";

char filenamebuf[32] = "";
std::string filename = "";

char loadfilenamebuf[32] = "";
std::string loadfilename = "";
std::string loadfilenamesubmit = "";
bool failedOpenFind = false;

bool tileSelectorChecked[7];
bool entitySelectorChecked[8];
bool pickingHoldBuffer = false;

int saveIndicatorCountdown = 0;
std::string indicatorText = "";
Color indicatorColor = DARKGREEN;

std::vector<Tile> mapTiles;
std::vector<Entity> mapEntities;
Map map = {{}, {1, 13}, {}, "", ""};

Vector2 mousePosition;
Vector2 tilePosition;

RenderTexture2D editorScreen;

void EditorInit()
{
    editorScreen = LoadRenderTexture(800, 600);

    SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked));
    tileSelectorChecked[0] = true;

    SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked));
    entitySelectorChecked[0] = true;
}

int Editor(bool& editorOpen, std::vector<Sprite> sprites, std::vector<Pack> &packs)
{
    mousePosition = {800 * (GetMousePosition().x / GetScreenWidth()), 600 * (GetMousePosition().y / GetScreenHeight())};
    Vector2 tSize = {40, 40};
    tilePosition = { float(tSize.x * ((int)(mousePosition.x / tSize.x))), float(tSize.y * ((int)(mousePosition.y / tSize.y))) };

    if(mapOpen)
    {
        if(!ImGui::GetIO().WantCaptureMouse)
        {
            if(pickingHoldBuffer)
                if(IsMouseButtonUp(MOUSE_BUTTON_LEFT)) { pickingHoldBuffer = false; }

            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !pickingHoldBuffer) 
            {
                if(!pickingSpawnPoint) 
                { 
                    if(placingMode == 0)
                        PlaceTile();
                    else
                        PlaceEntity();
                }
                else
                    PickSpawnPoint();
            }
            if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) 
            { 
                if(!pickingSpawnPoint) 
                { 
                    if(placingMode == 0)
                        RemoveTile();  
                    else
                        RemoveEntity();
                }
            }
            if(GetMouseWheelMove() != 0) 
                placingMode = !placingMode;
            if(IsKeyPressed(KEY_ONE)) 
                placingMode = 0;
            if(IsKeyPressed(KEY_TWO))
                placingMode = 1;
        }

        if(IsKeyDown(KEY_LEFT_CONTROL))
        {
            if(IsKeyPressed(KEY_S))
            {
                if(filename == "") 
                {
                    saveWindow = true;
                }
                else
                {
                    SaveMap(filename);
                }
            }

            if(IsKeyPressed(KEY_E))
                entityWindow = !entityWindow;
            
            if(IsKeyPressed(KEY_T))
                tileSelectorWindow = !tileSelectorWindow;
        }
    }

    if(IsKeyDown(KEY_LEFT_CONTROL))
        if(IsKeyPressed(KEY_O))
            loadWindow = true;
        if(IsKeyPressed(KEY_Q))
        {
            editorOpen = false;
            SetWindowTitle("window");
        }

    // BeginDrawing();
    BeginTextureMode(editorScreen);
        ClearBackground(BLACK);

        if(mapOpen)
        {
            renderTiles(mapTiles, sprites);
            RenderEntities(mapEntities, sprites);

            for(Entity e : mapEntities)
            {
                if(placingMode == 1)
                    DrawCircle(e.pos.x + 20, e.pos.y + 20, 4, MAROON);
                // DrawTriangle({ e.pos.x + 10, e.pos.y + 30 }, { e.pos.x + 30, e.pos.y + 30 }, { e.pos.x + 20, e.pos.y + 10 }, MAROON);
            }

            if(!ImGui::GetIO().WantCaptureMouse)
            {
                if(!pickingSpawnPoint)
                {
                    if(placingMode == 0)
                        DrawRectangleLines(tilePosition.x, tilePosition.y, 40, 40, WHITE);
                    else
                        DrawCircleLines(tilePosition.x + 20, tilePosition.y + 20, 20, WHITE);
                }
                else if(pickingSpawnPoint)
                    DrawCircleLines(tilePosition.x + 20, tilePosition.y + 20, 10, BLUE);
            }

            if(!pickingSpawnPoint)
                DrawCircleLines(map.playerSpawn.x * 40 + 20, map.playerSpawn.y * 40 + 20, 10, GREEN);
        }

        if(saveIndicatorCountdown > 0)
        {
            // DrawText(std::format("Saved to ./maps/{}.map", filename).c_str(), 10, 20, 15, GREEN);
            DrawOutlinedText(indicatorText.c_str(), 5, 20, 20, indicatorColor, 1, BLACK);
            saveIndicatorCountdown--;
        }
    // EndDrawing();
    EndTextureMode();

    BeginDrawing();
        DrawSpriteDirect(editorScreen.texture, {0, 0}, {(float)GetScreenWidth(), (float)GetScreenHeight()});

        rlImGuiBegin();
            if(ImGui::BeginMainMenuBar()) 
            {
                if(ImGui::BeginMenu("Editor"))
                {
                    if(ImGui::MenuItem("Help")) { helpWindow = true; }
                    if(ImGui::MenuItem("Close", "Ctrl+Q")) { editorOpen = false; SetWindowTitle("window"); }

                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Map"))
                {
                    if(ImGui::MenuItem("New")) { createMapWindow = true; }
                    if(ImGui::MenuItem("Open", "Ctrl+O")) { loadWindow = true; }
                    if(mapOpen)
                    {
                        if(ImGui::MenuItem("Save", "Ctrl+S")) { 
                            if(filename == "") 
                            {
                                saveWindow = true;
                            }
                            else
                            {
                                SaveMap(filename);
                            }
                        }
                        if(ImGui::MenuItem("Save As")) { saveWindow = true; }
                        if(ImGui::MenuItem("Edit Map Info")) { infoWindow = true; }
                    }

                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Pack"))
                {
                    if(ImGui::MenuItem("New")) { createPackWindow = true; }
                    if(ImGui::MenuItem("Edit Pack")) { editPackWindow = true; }

                    ImGui::EndMenu();
                }

                if(mapOpen)
                {
                    if(ImGui::BeginMenu("Tiles"))
                    {
                        if(ImGui::MenuItem("Open Tile Window", "Ctrl+T")) { tileSelectorWindow = !tileSelectorWindow; }
                        if(ImGui::MenuItem("Clear All Tiles")) { mapTiles.clear(); }

                        ImGui::EndMenu();
                    }

                    if(ImGui::BeginMenu("Entities"))
                    {
                        if(ImGui::MenuItem("Open Entity Window", "Ctrl+E")) { entityWindow = !entityWindow; }
                        if(ImGui::MenuItem("Choose Spawn Point")) { pickingSpawnPoint = true; }
                        if(ImGui::MenuItem("Clear All Entities")) { mapEntities.clear(); }

                        ImGui::EndMenu();
                    }
                }

                ImGui::EndMainMenuBar();
            }

            if(createMapWindow) { CreateMapWindow(); }
            if(tileSelectorWindow) { TileSelectorWindow(); }
            if(entityWindow) { EntityWindow(); }
            if(helpWindow) { HelpWindow(); }
            if(saveWindow) { SaveWindow(); }
            if(loadWindow) { LoadWindow(); }
            if(infoWindow) { InfoWindow(); }
            if(createPackWindow) { CreatePackWindow(packs); }
            if(editPackWindow) { EditPackWindow(packs); }
        rlImGuiEnd();
    EndDrawing();

    return 0;
}

uint8_t GetTileType(bool *array, int length)
{
    for(int i = 0; i < length; i++)
    {
        if(array[i] == true)
        {
            return i + 1;
        }
    }
    return 0;
}

void HelpWindow()
{
    // ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Editor Help", &helpWindow);

    ImGui::Text("Welcome to the map editor!");
    ImGui::Text("Here are a few things you should know:");
    ImGui::Text("");
    ImGui::SeparatorText("Navigating map files");
    ImGui::Text("       Map > New to start editing");
    ImGui::Text("       Map > Open to open an existing map");
    ImGui::Text("       Map > Save to save a the current map");
    ImGui::SeparatorText("Map paramaters");
    ImGui::Text("       Map ID: the ID used by the game to find the map");
    ImGui::Text("       Next Map ID: the ID of the map loaded after entering a door");
    ImGui::Text("       Background ID: the ID of the background the map should use");
    ImGui::Text("       File Name: the name of the file to save the map to (Doesn't need to include .map)");
    ImGui::SeparatorText("Navigating the editor");
    ImGui::Text("       Left click to place");
    ImGui::Text("       Right click to remove");
    ImGui::Text("       Scroll to switch between tile/entity mode");
    ImGui::SeparatorText("Placing things in the map");
    ImGui::Text("       Tiles > Open Tile Window to pick tiles");
    ImGui::Text("       Entities > Open Entity Window to pick entities");
    ImGui::Text("       Entities > Choose Spawn Point to place spawn");
    ImGui::SeparatorText("Placement Icon");
    ImGui::Text("       If the placement icon is a square, you are in Tile Mode");
    ImGui::Text("       If the placement icon is a circle, you are in Entity Mode");
    ImGui::Text("       The little red circles that appear in Entity Mode are the origin of each entity");

    ImGui::End();
}

void EditorLoadMap(std::string fileName)
{
    map = LoadMapFromFilePathShorthand(fileName);
    makeTilesFromMap(map, &mapTiles);
    mapEntities = map.entities;

    std::cout << std::format("EDITOR: Loaded map {} from ./{}.map\n", map.mapID, fileName);
    mapOpen = true;
    filename = loadfilename;
    
    SetWindowTitle(std::format("Map Editor - {}.map", fileName).c_str());

    buf1 = map.mapID.c_str();
    buf2 = map.nextMapID.c_str();
    buf3 = map.backgroundID.c_str();
}

void LoadWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Open Map", &loadWindow);

    if(ImGui::InputText("File Name", loadfilenamebuf, IM_ARRAYSIZE(loadfilenamebuf)))
    {
        loadfilename = loadfilenamebuf;
    }
    if(ImGui::Button("Open"))
    {
        std::filesystem::path mapPath = std::format("./{}.map", loadfilename);
        if(std::filesystem::exists(mapPath))
        {
            EditorLoadMap(loadfilename);
            loadWindow = false;
            failedOpenFind = false;
        }
        else
        {
            failedOpenFind = true;
            std::cout << std::format("EDITOR: Failed to find ./{}.map\n", loadfilename);
        }

        loadfilenamesubmit = loadfilename;
        
    }

    if(failedOpenFind)
        ImGui::TextColored(ImVec4{255, 0, 0, 255}, std::format("Could not find file ./{}.map", loadfilenamesubmit).c_str());

    ImGui::End();
}

void InfoWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Edit Map Info", &infoWindow);

    if(ImGui::InputText("Map ID", &buf1, 32))
    {
        map.mapID = buf1;
    }
    if(ImGui::InputText("Next Map ID", &buf2, 32))
    {
        map.nextMapID = buf2;
    }
    if(ImGui::InputText("Background ID", &buf3, 32))
    {
        map.backgroundID = buf3;
    }
    if(ImGui::Button("Apply"))
    {
        infoWindow = false;
    }

    ImGui::End();
}

void SaveMap(std::string fileName)
{
    std::ofstream file;
    file.open(std::format("./{}.map", fileName));

    file << map.mapID << "\n";
    file << map.nextMapID << "\n";
    file << map.backgroundID << "\n";
    file << map.playerSpawn.x << "\n";
    file << map.playerSpawn.y << "\n";

    MakeMapFromTiles(mapTiles, map);
    for(int i = 0; i < 300; i++)
    {
        file << std::to_string(map.tiles[i]) << ",";
    }
    file << std::to_string(map.tiles[300]) << ";";

    if(mapEntities.size() > 0)
    {
        file << "\n";
        for(int i = 0; i < mapEntities.size() - 1; i++)
        {
            file << SerializeEntity(mapEntities[i]) << "\n";
        }
        file << SerializeEntity(mapEntities[mapEntities.size() - 1]);
    }

    file.close();

    indicatorColor = DARKGREEN;
    indicatorText = std::format("Saved map to {}.map", fileName);
    saveIndicatorCountdown = 90;
    std::cout << "Saved map " << map.mapID << std::format(" to ./{}.map\n", fileName);
}

void SaveWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Save Map", &saveWindow);

    if(ImGui::InputText("File Name", filenamebuf, IM_ARRAYSIZE(filenamebuf)))
    {
        filename = filenamebuf;
    }
    if(ImGui::Button("Save"))
    {
        SaveMap(filename);
        saveWindow = false;
    }

    ImGui::End();
}

void PlaceEntity()
{
    RemoveEntityFromVectorByPos(tilePosition, mapEntities);
    if(GetTileType(entitySelectorChecked, sizeof(entitySelectorChecked)) == 5)
    {
        mapEntities.push_back({5, tilePosition, {160, 160}, 0, {5, 0}});
    }
    else
    {
        mapEntities.push_back({GetTileType(entitySelectorChecked, sizeof(entitySelectorChecked)), tilePosition, {40, 40}, 0});
    }
}

void RemoveEntity()
{
    RemoveEntityFromVectorByPos(tilePosition, mapEntities);
}

void SetAllArrayFalse(bool *array, int length)
{
    for(int i = 0; i < length; i++)
    {
        array[i] = false;
    }
}

void EntityWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Entity Selector", &entityWindow);

    if(ImGui::Checkbox("Powerup", &entitySelectorChecked[0])) { SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked)); entitySelectorChecked[0] = true; }
    if(ImGui::Checkbox("Demon", &entitySelectorChecked[2])) { SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked)); entitySelectorChecked[2] = true; }
    if(ImGui::Checkbox("Powerup Generator", &entitySelectorChecked[3])) { SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked)); entitySelectorChecked[3] = true; }
    if(ImGui::Checkbox("Boss Demon", &entitySelectorChecked[4])) { SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked)); entitySelectorChecked[4] = true; }
    if(ImGui::Checkbox("Boss Alter", &entitySelectorChecked[5])) { SetAllArrayFalse(entitySelectorChecked, sizeof(entitySelectorChecked)); entitySelectorChecked[5] = true; }

    ImGui::End();
}

void PickSpawnPoint()
{
    pickingHoldBuffer = true;
    map.playerSpawn = { tilePosition.x / 40, tilePosition.y / 40 };
    pickingSpawnPoint = false;
}

void TileSelectorWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Tile Selector", &tileSelectorWindow);

    if(ImGui::Checkbox("Block", &tileSelectorChecked[0])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[0] = true; }
    if(ImGui::Checkbox("Door", &tileSelectorChecked[1])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[1] = true; }
    if(ImGui::Checkbox("Spike", &tileSelectorChecked[2])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[2] = true; }
    ImGui::SameLine();
    if(ImGui::Checkbox("90°", &tileSelectorChecked[3])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[3] = true; }
    ImGui::SameLine();
    if(ImGui::Checkbox("180°", &tileSelectorChecked[4])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[4] = true; }
    ImGui::SameLine();
    if(ImGui::Checkbox("270°", &tileSelectorChecked[5])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[5] = true; }
    if(ImGui::Checkbox("Boss Door", &tileSelectorChecked[6])) { SetAllArrayFalse(tileSelectorChecked, sizeof(tileSelectorChecked)); tileSelectorChecked[6] = true; }

    ImGui::End();
}

void ApplyChangesToPack(std::vector<Pack> &packs)
{
    std::string packDir = "./packs/" + packName;

    if(oldPackName != packName)
    {
        std::cout << "EDITOR: Renaming pack " << oldPackName << " to " << packName << "\n";

        if(std::filesystem::exists(std::filesystem::path(editingPackRootDir.c_str())))
        {
            std::filesystem::rename(std::filesystem::path(editingPackRootDir.c_str()), std::filesystem::path("./packs/" + packName));
        }
        else
        {
            indicatorColor = RED;
            indicatorText = std::format("Couldn't find pack at {}", ("./packs/" + oldPackName + "/"));
            saveIndicatorCountdown = 90;
            return;
        }
    }

    std::ofstream file(packDir + "/.pack");

    file << packName << std::endl;

    if(!packDefaultMaps)
    {
        file << "/maps" << std::endl;
        if(oldPackDefaultMaps)
        {
            std::cout << "EDITOR: Creating maps dir for existing pack " << "\n";
            std::filesystem::create_directory(std::filesystem::path(packDir + "/maps"));
        }
    }
    else
    {
        file << "#default" << std::endl;
    }

    if(!packDefaultAssets)
    {
        file << "/assets" << std::endl;
        if(oldPackDefaultAssets)
        {
            std::cout << "EDITOR: Creating assets dir for existing pack " << "\n";
            std::filesystem::create_directory(std::filesystem::path(packDir + "/assets"));
            std::filesystem::create_directory(std::filesystem::path(packDir + "/assets/sounds"));
            std::filesystem::create_directory(std::filesystem::path(packDir + "/assets/backgrounds"));
        }
    }
    else 
    {
        file << "#default" << std::endl;
    }

    file << packFirstMap << std::endl;

    file.close();

    RegisterAllPacks("./packs", packs, EH_EXEMPT);
    
    indicatorColor = DARKGREEN;
    indicatorText = std::format("Saved pack to {}", ("./packs/" + packName + "/"));
    saveIndicatorCountdown = 90;

    std::cout << "EDITOR: Edited pack " << packName << "\n";
}

void EditPackWindow(std::vector<Pack> &packs)
{   
    Pack p;

    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Edit Pack", &editPackWindow);

    ImGui::InputText("Pack To Edit", &loadPackName, 32);
    if(ImGui::Button("Choose"))
    {
        p = GetPackByName(loadPackName, packs, packLoadingError);
        if(!packLoadingError)
        {
            oldPackName = loadPackName;
            packName = p.name;
            packFirstMap = p.firstMapID;
            if(p.assetDir == "#default") { packDefaultAssets = true; oldPackDefaultAssets = true; } else { packDefaultAssets = false; oldPackDefaultAssets = false; }
            if(p.mapDir == "#default") { packDefaultMaps = true; oldPackDefaultMaps = true; } else { packDefaultMaps = false; oldPackDefaultMaps = false; }
            editingPackRootDir = p.rootDir;
        }
    }

    if(packLoadingError)
    {
        ImGui::TextColored(ImVec4{255, 0, 0, 255}, "Couldn't find that pack");
    }

    ImGui::Separator();

    ImGui::InputText("Pack Name", &packName, 32);
    ImGui::InputText("First Map ID", &packFirstMap, 32);

    ImGui::Checkbox("Use Default Assets", &packDefaultAssets);
    ImGui::Checkbox("Use Default Maps", &packDefaultMaps);

    if(ImGui::Button("Apply"))
    {
        if(packName != "")
        {
            ApplyChangesToPack(packs);
            editPackWindow = false;
        }   
    }

    ImGui::End();
}

void CreatePack(std::vector<Pack> &packs)
{
    std::string packDir = "./packs/" + packName;

    std::filesystem::path packDirectory(packDir);
    std::filesystem::create_directory(packDir);

    if(!packDefaultAssets)
    {
        std::filesystem::create_directory(std::filesystem::path(packDir + "/assets"));
        std::filesystem::create_directory(std::filesystem::path(packDir + "/assets/sounds"));
        std::filesystem::create_directory(std::filesystem::path(packDir + "/assets/backgrounds"));
    }

    if(!packDefaultMaps)
    {
        std::filesystem::create_directory(std::filesystem::path(packDir + "/maps"));
    }

    std::ofstream file(packDir + "/.pack");

    file << packName << std::endl;

    if(!packDefaultMaps)
    {
        file << "/maps" << std::endl;
    }
    else
    {
        file << "#default" << std::endl;
    }

    if(!packDefaultAssets)
    {
        file << "/assets" << std::endl;
    }
    else 
    {
        file << "#default" << std::endl;
    }

    file << packFirstMap << std::endl;

    file.close();

    RegisterAllPacks("./packs", packs, EH_EXEMPT);
    
    indicatorColor = DARKGREEN;
    indicatorText = std::format("Created pack at {}", ("./packs/" + packName + "/"));
    saveIndicatorCountdown = 90;

    std::cout << "EDITOR: Created pack " << packName << "\n";
}

void CreatePackWindow(std::vector<Pack> &packs)
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Create Pack", &createPackWindow);

    ImGui::InputText("Pack Name", &packName, 32);
    ImGui::InputText("First Map ID", &packFirstMap, 32);

    ImGui::Checkbox("Use Default Assets", &packDefaultAssets);
    ImGui::Checkbox("Use Default Maps", &packDefaultMaps);

    if(ImGui::Button("Create"))
    {
        if(packName != "" && packFirstMap != "")
        {
            CreatePack(packs);
            createPackWindow = false;
        }
    }

    ImGui::End();
}

void CreateMapWindow()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Create Map", &createMapWindow);

    if(ImGui::InputText("Map ID", &buf1, 32))
    {
        map.mapID = buf1;
    }
    if(ImGui::InputText("Next Map ID", &buf2, 32))
    {
        map.nextMapID = buf2;
    }
    if(ImGui::InputText("Background ID", &buf3, 32))
    {
        map.backgroundID = buf3;
    }
    if(ImGui::Button("Create"))
    {
        CreateNewMap();
    }

    ImGui::End();
}

void PlaceTile()
{
    RemoveTileFromVectorByPos(tilePosition, mapTiles);
    mapTiles.push_back({GetTileType(tileSelectorChecked, sizeof(tileSelectorChecked)), tilePosition});
}

void RemoveTile()
{
    RemoveTileFromVectorByPos(tilePosition, mapTiles);
}

void CreateNewMap()
{   
    std::cout << "EDITOR: Created map " << map.mapID << "\n";
    createMapWindow = false;
    mapOpen = true;

    SetWindowTitle("Map Editor - Unsaved");

    mapTiles.clear();
    mapEntities.clear();
    map.playerSpawn = {1, 13};
    filename = "";
}