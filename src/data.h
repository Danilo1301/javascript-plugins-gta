#pragma once

#include "mod/iaml.h"
#include <string>

inline std::string GetScriptsFolder()
{
    std::string dataRootPath = aml->GetAndroidDataRootPath();
    std::string scriptsFolder = dataRootPath + "/scripts/";

    return scriptsFolder;
}