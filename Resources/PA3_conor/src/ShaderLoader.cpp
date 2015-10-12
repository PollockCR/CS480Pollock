#include "ShaderLoader.hpp"
#include <iostream>
#include <fstream>
#include <string.h>

char* ShaderLoader::LoadShader(const char* FilePath)
{
    char* Loader = new char[1000];
    char* temp = new char[1000];
    int StringSize = 1000;
    std::ifstream LoaderFile(FilePath);

    // Clear char array
    for (int LoaderIndex = 0; LoaderIndex < 1000; LoaderIndex++)
    {
        Loader[LoaderIndex] = '\0';
        temp[LoaderIndex] = '\0';
    }

    // Input loader and concatenate to Loader char array
    do
    {
        LoaderFile.getline(temp, StringSize);
        strcat(Loader, temp);
        Loader[strlen(Loader)] = '\n';
    }
    while (LoaderFile.good());

    return Loader;
}
