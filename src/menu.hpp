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

void UpdateButtons(std::vector<Button> buttons, GameSound selectionSound);
void StartGame();
void RenderButtons(std::vector<Button> buttons);
void QuitGame();
void OpenModMenu();
void ModMenu();
void PauseMenu(bool& paused, bool& mainMenu, GameSound clickSound);
void Resume();
void ReturnToMenu();

Font uiFont;

// this is one of the weirdes solutions ive ever had to come upt with but i think itll work
bool menuOpenSync = true;
bool forceQuitSync = false;
bool pauseMenuSync = false;

bool modMenu = false;
std::string modPackName = "";
char nameBuf[48] = "";
bool menuLoadPack = false;

RenderTexture2D menuScreen;

std::vector<Button> buttons;
std::vector<Button> pauseButtons;
std::vector<std::string> packNames;

Vector2 menuMousePosition;

void MenuInit(bool menuOpen, std::vector<Pack>& packs)
{
    uiFont = LoadFontEx("./assets/fonts/retro.ttf", 100, 0, 0);

    std::cout << "MENU: Starting menu\n";
    menuOpenSync = menuOpen;
    menuScreen = LoadRenderTexture(800, 600);

    buttons.push_back(Button{{20, 120}, {200, 50}, 40, "Play", StartGame});
    buttons.push_back(Button{{20, 180}, {200, 50}, 40, "Mods", OpenModMenu});
    buttons.push_back(Button{{20, 240}, {200, 50}, 40, "Quit", QuitGame});

    pauseButtons.push_back(Button{{20, 80}, {180, 40}, 30, "Resume", Resume});
    pauseButtons.push_back(Button{{20, 130}, {180, 40}, 30, "Main Menu", ReturnToMenu});

    packNames.clear();

    for(Pack p : packs)
    {
        packNames.push_back(p.name);
    }
}

int MainMenu(bool& menuOpen, bool& forceQuit, std::vector<Sprite>& sprites, std::vector<Sprite>& backgrounds, GameSound clickSound)
{
    menuMousePosition = {800 * (GetMousePosition().x / GetScreenWidth()), 600 * (GetMousePosition().y / GetScreenHeight())};

    menuOpen = menuOpenSync;
    forceQuit = forceQuitSync;
    UpdateButtons(buttons, clickSound);

    BeginTextureMode(menuScreen);
        ClearBackground(BLACK);
        DrawSpriteFromVectorAlpha("cave", {0, 0}, {800, 600}, backgrounds, 178);
        DrawTextEx(uiFont, "Game", {20, 20}, 80, 8, BLACK);
        RenderButtons(buttons);
        DrawTextEx(uiFont, "A PancakeHat Game", {520, 580}, 20, 4, BLACK);
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
    DrawTextEx(uiFont, "Paused", {20, 20}, 50, 8, BLACK);
    RenderButtons(pauseButtons);
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

void QuitGame()
{
    forceQuitSync = true;
}

void UpdateButtons(std::vector<Button> buttons, GameSound selectionSound)
{
    for(Button b : buttons)
    {
        if(!ImGui::GetIO().WantCaptureMouse)
        {
            if(checkBoxCollison(menuMousePosition, {0, 0}, b.pos, b.size))
            {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    PlaySound(selectionSound.sound);
                    std::cout << std::format("MENU: Button {} clicked\n", b.text);
                    b.onClicked();
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
            DrawRectangle(b.pos.x, b.pos.y, b.size.x, b.size.y, {170, 170, 170, 255});
        else
            DrawRectangle(b.pos.x, b.pos.y, b.size.x, b.size.y, {128, 128, 128, 255});

        // DrawText(b.text.c_str(), b.pos.x + 5, b.pos.y + 5, b.fontSize, BLACK);
        DrawTextEx(uiFont, b.text.c_str(), {b.pos.x + b.fontSize / 10, b.pos.y + b.fontSize / 10}, b.fontSize, b.fontSize / 10, BLACK);
    }
}