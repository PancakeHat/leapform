#pragma once

#include <iostream>
#include <raylib.h>
#include "rlimgui/rlImGui.h"
#include "rlimgui/imgui.h"
#include "rlimgui/imgui_stdlib.h"
#include <string>

#define ERROR_NONFATAL 0
#define ERROR_FATAL 1

struct ErrorHandler {
    bool activeError;
    std::string errorMessage;
    bool isFatal;
    bool overridable;
};

void ThrowNewError(std::string message, int severity, bool overridable, ErrorHandler& errorHandler)
{
    if(errorHandler.overridable)
    {
        errorHandler.activeError = true;
        errorHandler.errorMessage = message;
        errorHandler.overridable = overridable;

        if(severity == ERROR_FATAL)
            errorHandler.isFatal = true;
        else
            errorHandler.isFatal = false;
    }
}

void ForceErrorHandlerOverride(bool overridable, ErrorHandler& errorHandler)
{
    errorHandler.overridable = overridable;
}