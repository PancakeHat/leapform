#pragma once

#include <string>

std::string RemoveFileEnding(std::string name)
{
    std::string n = name;
    for(int i = name.length() - 1; i > 0; i--)
    {
        if(n[i] == '.')
        {
            n.pop_back();
            return n;
        }
        n.pop_back();
    }

    return "";
}

bool StringEndsIn(std::string str, std::string ending)
{
    for(int i = 0; i < ending.length(); i++)
    {
        if(ending[i] != str[str.length() - ending.length() + i])
            return false;
    }

    return true;
}

bool StringStartsWith(std::string str, std::string beginning)
{
    for(int i = 0; i < beginning.length(); i++)
    {
        if(beginning[i] != str[i])
            return false;
    }

    return true;
}