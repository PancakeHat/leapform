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

void UpdateMainMenu();
void StartGame();
void RenderButtons();

RenderTexture2D menuScreen;

struct Button
{
    Vector2 pos;
    Vector2 size;
    std::string text;
    void (*onClicked)();
};

std::vector<Button> buttons;

Vector2 menuMousePosition;

void MenuInit()
{
    menuScreen = LoadRenderTexture(800, 600);

    buttons.push_back(Button{{100, 100}, {200, 50}, "Play", StartGame});
}

int MainMenu(bool& menuOpen)
{
    menuMousePosition = {800 * (GetMousePosition().x / GetScreenWidth()), 600 * (GetMousePosition().y / GetScreenHeight())};

    UpdateMainMenu();

    BeginTextureMode(menuScreen);
        ClearBackground(BLUE);
        RenderButtons();
    EndTextureMode();

    BeginDrawing();
        DrawSpriteDirect(menuScreen.texture, {0, 0}, {(float)GetScreenWidth(), (float)GetScreenHeight()});
    EndDrawing();

    return 0;
}

void StartGame()
{
    std::cout << "starting game\n";
}

void UpdateMainMenu()
{
    for(Button b : buttons)
    {
        if(checkBoxCollison(menuMousePosition, {0, 0}, b.pos, b.size))
        {
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                b.onClicked();
        }
    }
}

void RenderButtons()
{
    for(Button b : buttons)
    {
        if(checkBoxCollison(menuMousePosition, {0, 0}, b.pos, b.size))
            DrawRectangle(b.pos.x, b.pos.y, b.size.x, b.size.y, {180, 180, 180, 255});
        else
            DrawRectangle(b.pos.x, b.pos.y, b.size.x, b.size.y, {128, 128, 128, 255});

        DrawText(b.text.c_str(), b.pos.x + 5, b.pos.y + 5, 15, BLACK);
    }
}