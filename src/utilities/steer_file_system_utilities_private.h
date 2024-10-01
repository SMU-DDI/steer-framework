// =================================================================================================
//! @file steer_file_system_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private file system utility functions for the STandard Entropy 
//! Evaluation Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-09
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_FILE_SYSTEM_UTILITIES_PRIVATE_H__
#define __STEER_FILE_SYSTEM_UTILITIES_PRIVATE_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    bool STEER_PathHasFileExtension (const char* path);

    int32_t STEER_IsFilePath (const char* pathString,
                              bool* isFilePath,
                              bool* exists);

    bool STEER_IsFileEmpty (const char* filePath);

    int32_t STEER_ExpandTildeInPath (const char* pathString,
                                     char** expandedPathString);
                                     
    bool STEER_IsDirectoryPath (const char* pathString);
                              
    bool STEER_DirectoryExists (const char* directoryPath);

    int32_t STEER_CreateDirectory (const char* directoryPath,
                                   mode_t mode);

    bool STEER_DirectoryHasTrailingSlash (const char* directoryPath);

    int32_t STEER_TrimFileExtensionFromPath (const char* pathString,
                                             const char* fileExtension,
                                             char** trimmedPath);

    int32_t STEER_ValidateFileString (const char* fileString,
                                      char** filePath);

    int32_t STEER_ValidatePathString (const char* pathString,
                                      bool createMissingDirectoriesInPath,
                                      char** path);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_FILE_SYSTEM_UTILITIES_PRIVATE_H__
// =================================================================================================
