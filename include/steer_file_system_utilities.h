// =================================================================================================
//! @file steer_file_system_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public file system utility functions for the STandard Entropy 
//! Evaluation Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_FILE_SYSTEM_UTILITIES_H__
#define __STEER_FILE_SYSTEM_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Macros
// =================================================================================================

//! @def STEER_BINARY_FILE_NAME_EXTENSION
//! @brief File extension for binary data files.
#define STEER_BINARY_FILE_NAME_EXTENSION    ".bin"

//! @def STEER_JSON_FILE_NAME_EXTENSION
//! @brief File extension for JSON files.
#define STEER_JSON_FILE_NAME_EXTENSION      ".json"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    bool STEER_FileExists (const char* filePath);

    int32_t STEER_FileSize (const char* filePath,
                            size_t* fileSizeInBytes);

    bool STEER_IsReadableDevicePath (const char* pathString);

    int32_t STEER_OpenFile (const char* filePath,
                            bool forWriting,
                            bool asBinary,
                            FILE** fileReference);

    int32_t STEER_CloseFile (FILE** fileReference);
    
    int32_t STEER_ReadTextFile (const char* filePath,
                                uint8_t** textBuffer);
    
    int32_t STEER_WriteTextFile (const char* filePath,
                                 char* textBuffer);

    int32_t STEER_ScanDirectoryForFilesWithExtension (const char* directory,
                                                      const char* fileExtension,
                                                      uint32_t* fileCount,
                                                      tSTEER_FilePath** filePaths);

    int32_t STEER_GetProgramDirectory (char** programDirectoryPath);
    
    int32_t STEER_ScanProgramDirectory (const char* programDirectoryPath,
                                        uint32_t* programCount,
                                        tSTEER_FileName** programNames);

    bool STEER_ProgramAvailable (const char* programName,
                                 uint32_t programCount,
                                 tSTEER_FileName* programNames);


#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_FILE_SYSTEM_UTILITIES_H__
// =================================================================================================
