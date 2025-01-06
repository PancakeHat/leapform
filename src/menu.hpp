#include "raylib.h"
#include <vector>
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

struct Button
{
    Vector2 pos;
    Vector2 size;
    int fontSize;
    std::string text;
    void (*onClicked)();
};

void UpdateButtons(std::vector<Button> &buttons, GameSound selectionSound);
void StartGame();
void RenderButtons(std::vector<Button> buttons);
void QuitGame();
void OpenModMenu();
void ModMenu();
void PauseMenu(bool& paused, bool& mainMenu, GameSound clickSound);
void Resume();
void ReturnToMenu();
void OpenMapEditor();
void ToggleTimer();

Font uiFont;
Font titleFont;

// this is one of the weirdes solutions ive ever had to come upt with but i think itll work
bool menuOpenSync = true;
bool forceQuitSync = false;
bool pauseMenuSync = false;
bool mapEditorSync = false;
bool speedrunTimerSync = false;

bool modMenu = false;
std::string modPackName = "";
char nameBuf[48] = "";
bool menuLoadPack = false;

bool showFinalTime = false;
int finalTime;

Color titleTextColor = {91, 111, 226, 255};
Color titleTextOutlineColor = {95, 207, 227, 255};

Color buttonForeground = {55, 72, 75, 255};
Color buttonBackground = {18, 39, 43, 255};
Color buttonBackgroundHighlight = {53, 109, 119, 255};

RenderTexture2D menuScreen;

std::vector<Button> buttons;
std::vector<Button> pauseButtons;
std::vector<std::string> packNames;

Vector2 menuMousePosition;

void MenuInit(bool menuOpen, std::vector<Pack>& packs)
{
    uiFont = LoadFontEx("./assets/fonts/retro.ttf", 2048, 0, 0);
    titleFont = LoadFontEx("./assets/fonts/wonder.ttf", 900, 0, 0);

    std::cout << "MENU: Starting menu\n";
    menuOpenSync = menuOpen;
    menuScreen = LoadRenderTexture(800, 600);

    buttons.push_back(Button{{20, 120}, {230, 50}, 40, "Play", StartGame});
    buttons.push_back(Button{{20, 180}, {230, 50}, 40, "Mods", OpenModMenu});
    buttons.push_back(Button{{20, 240}, {230, 50}, 40, "Editor", OpenMapEditor});
    buttons.push_back(Button{{20, 300}, {230, 50}, 40, "Timer Off", ToggleTimer});
    buttons.push_back(Button{{20, 360}, {230, 50}, 40, "Quit", QuitGame});

    pauseButtons.push_back(Button{{20, 85}, {180, 40}, 30, "Resume", Resume});
    pauseButtons.push_back(Button{{20, 135}, {180, 40}, 30, "Main Menu", ReturnToMenu});

    packNames.clear();

    for(Pack p : packs)
    {
        packNames.push_back(p.name);
    }
}

int MainMenu(bool& menuOpen, bool& forceQuit, std::vector<Sprite>& sprites, std::vector<Sprite>& backgrounds, GameSound clickSound, bool& inMapEditor, bool& speedrunTimer)
{
    menuMousePosition = {800 * (GetMousePosition().x / GetScreenWidth()), 600 * (GetMousePosition().y / GetScreenHeight())};

    menuOpen = menuOpenSync;
    forceQuit = forceQuitSync;
    inMapEditor = mapEditorSync;
    speedrunTimer = speedrunTimerSync;
    UpdateButtons(buttons, clickSound);

    if (IsKeyPressed(KEY_M) && !ImGui::GetIO().WantCaptureMouse) { inMapEditor = true; mapEditorSync = true; SetWindowTitle("Map Editor"); }


    BeginTextureMode(menuScreen);
        ClearBackground(BLACK);
        DrawSpriteFromVectorAlpha("cave", {0, 0}, {800, 600}, backgrounds, 178);
        DrawOutlinedTextEx("LEAPFORM", 25, 20, 80, titleTextColor, 5, titleTextOutlineColor, titleFont, 5);
        RenderButtons(buttons);
        DrawTextEx(uiFont, "A PancakeHat Game", {535, 580}, 20, 4, {153, 175, 179, 255});

        if(showFinalTime)
        {
            std::string hdths = std::to_string((int)((float)finalTime / 0.6) % 10);

            DrawRectangle(660, 20, 140, 45, BLACK);
            DrawTextEx(uiFont, std::format("{}:{}:{}{}", (int)((finalTime / 60) / 60), (int)((finalTime / 60) % 60), (int)((finalTime / 6) % 10), hdths).c_str(), {665, 20}, 30, 2, BLUE);
            DrawTextEx(uiFont, std::format("{}", finalTime).c_str(), {665, 45}, 20, 2, BLUE);
        }
    EndTextureMode();

    BeginDrawing();
        DrawSpriteDirect(menuScreen.texture, {0, 0}, {(float)GetScreenWidth(), (float)GetScreenHeight()});
        rlImGuiBegin();
            if(modMenu)
            {
                ModMenu();
            }
        rlImGuiEnd();
    EndDrawing();

    return 0;
}

void Resume()
{
    pauseMenuSync = false;
}

void ReturnToMenu()
{
    menuOpenSync = true;
    pauseMenuSync = false;
}

void PauseMenu(bool& paused, bool& mainMenu, GameSound clickSound)
{
    menuMousePosition = {800 * (GetMousePosition().x / GetScreenWidth()), 600 * (GetMousePosition().y / GetScreenHeight())};

    paused = pauseMenuSync;
    mainMenu = menuOpenSync;

    UpdateButtons(pauseButtons, clickSound);
    DrawRectangle(0, 0, 800, 600, {0, 0, 0, 80});
    DrawOutlinedTextEx("Paused", 24, 20, 50, titleTextColor, 4, titleTextOutlineColor, titleFont, 4);
    RenderButtons(pauseButtons);
}

void OpenMapEditor()
{
    SetWindowTitle("Map Editor");
    mapEditorSync = true;
}

void ModMenu()
{
    ImGui::SetNextWindowSize({0, 0});
    if(ImGui::Begin("Mod Menu", &modMenu))
    {
        if(ImGui::InputText("Pack Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
        {
            modPackName = nameBuf;
        }
        if(ImGui::Button("Load Pack"))
        {
            std::cout << std::format("MENU: Loading mod {} from menu\n", modPackName);
            menuLoadPack = true;
            menuOpenSync = false;
        }
        ImGui::Separator();
        ImGui::Text("Packs:");
        for(std::string n : packNames)
        {
            ImGui::TextColored(ImVec4{0.6, 0.6, 0.6, 255}, n.c_str());
        }
        ImGui::End();
    }
}

void OpenModMenu()
{
    modMenu = true;
}

void StartGame()
{
    menuLoadPack = false;
    menuOpenSync = false;
}

void ToggleTimer()
{
    std::cout << "MENU: Toggling timer\n";
    speedrunTimerSync = !speedrunTimerSync;
}

void QuitGame()
{
    forceQuitSync = true;
}

void UpdateButtons(std::vector<Button> &buttons, GameSound selectionSound)
{
    for(int i = 0; i < buttons.size(); i++)
    {
        if(!ImGui::GetIO().WantCaptureMouse)
        {
            if(checkBoxCollison(menuMousePosition, {0, 0}, buttons[i].pos, buttons[i].size))
            {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    PlaySound(selectionSound.sound);
                    std::cout << std::format("MENU: Button {} clicked\n", buttons[i].text);
                    buttons[i].onClicked();

                    // if timer button
                    if(buttons[i].pos.y == 300)
                    {
                        if(speedrunTimerSync)
                            buttons[i].text = "Timer On";
                        else
                            buttons[i].text = "Timer Off";
                    }
                }
            }
        }
    }
}

void RenderButtons(std::vector<Button> buttons)
{
    for(Button b : buttons)
    {
        if(checkBoxCollison(menuMousePosition, {0, 0}, b.pos, b.size) && !ImGui::GetIO().WantCaptureMouse)
            DrawRectangle(b.pos.x - 3, b.pos.y - 3, b.size.x + 6, b.size.y + 6, buttonBackgroundHighlight);
        else
            DrawRectangle(b.pos.x - 3, b.pos.y - 3, b.size.x + 6, b.size.y + 6, buttonBackground);

        DrawRectangle(b.pos.x, b.pos.y, b.size.x, b.size.y, buttonForeground);
        DrawTextEx(uiFont, b.text.c_str(), {b.pos.x + b.fontSize / 10, b.pos.y + b.fontSize / 10}, b.fontSize, b.fontSize / 10, {153, 175, 179, 255});
    }
}