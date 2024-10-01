// =================================================================================================
//! @file steer_file_system_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements file system utility functions for the STandard Entropy Evaluation
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_file_system_utilities_private.h"
#include "steer_file_system_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include "whereami.h"
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <wordexp.h>

// =================================================================================================
//  STEER_PathHasFileExtension
// =================================================================================================
bool STEER_PathHasFileExtension (const char* path)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool hasFileExtension = false;

    // Check arguments
    result = STEER_CHECK_STRING(path);
    if (result == STEER_RESULT_SUCCESS)
    {
        char* ext = NULL;

        ext = strrchr(path, '.');
        hasFileExtension = (ext != NULL);
    }
    return hasFileExtension;
}

// =================================================================================================
//  STEER_IsFilePath
// =================================================================================================
int32_t STEER_IsFilePath (const char* pathString,
                          bool* isFilePath,
                          bool* exists)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(isFilePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(exists);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *isFilePath = false;
        *exists = false;

        // Make sure it's not a directory path
        if (STEER_IsDirectoryPath(pathString))
        {
            struct stat fileStat;

            *isFilePath = true;

            // Get file statistics
            memset((void*)&fileStat, 0, sizeof(struct stat));
            result = stat(pathString, &fileStat);
            if (result != STEER_RESULT_SUCCESS)
                result = STEER_CHECK_ERROR(errno);

            if (result == STEER_RESULT_SUCCESS)
                *exists = S_ISREG(fileStat.st_mode);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_IsFileEmpty
// =================================================================================================
bool STEER_IsFileEmpty (const char* filePath)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool isEmpty = false;

    // Check arguments
    result = STEER_CHECK_STRING(filePath);
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t fileSize = 0;

        // Get the file size
        result = STEER_FileSize(filePath, &fileSize);
        if (result == STEER_RESULT_SUCCESS)
            isEmpty = (fileSize == 0);
    }
    return isEmpty;
}

// =================================================================================================
//  STEER_ExpandTildeInPath
// =================================================================================================
int32_t STEER_ExpandTildeInPath (const char* pathString,
                                 char** expandedPathString)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check parameters
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(expandedPathString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        if (pathString[0] == '~')
        {
            wordexp_t p;

            result = wordexp(pathString, &p, 
                             WRDE_NOCMD | WRDE_SHOWERR | WRDE_UNDEF);
            if (result == STEER_RESULT_SUCCESS)
            {
                uint_fast32_t i = 0;

                result = STEER_DuplicateString(p.we_wordv[0], expandedPathString);
                for (i = 1; i < p.we_wordc; i++)
                {
                    result = STEER_ConcatenateString(expandedPathString, "/");
                    if (result == STEER_RESULT_SUCCESS)
                        result = STEER_ConcatenateString(expandedPathString, p.we_wordv[i]);

                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }

                // Clean up
                wordfree(&p);
            }
            else
                result = STEER_CHECK_ERROR(result);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_IsDirectoryPath
// =================================================================================================
bool STEER_IsDirectoryPath (const char* pathString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool isDirectoryPath = false;

    // Check arguments
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
    {
        struct stat fileStat;

        // Get file statistics
        memset((void*)&fileStat, 0, sizeof(struct stat));
        result = stat(pathString, &fileStat);
        if (result != STEER_RESULT_SUCCESS)
            result = STEER_CHECK_ERROR(errno);

        if (result == STEER_RESULT_SUCCESS)
            isDirectoryPath = S_ISDIR(fileStat.st_mode);
    }
    return isDirectoryPath;
}
                              
// =================================================================================================
//  STEER_DirectoryExists
// =================================================================================================
bool STEER_DirectoryExists (const char* directoryPath)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool exists = false;

    // Check arguments
    result = STEER_CHECK_STRING(directoryPath);
    if (result == STEER_RESULT_SUCCESS)
    {
        struct stat s;

        // Get statistics
        result = stat(directoryPath, &s);
        if (result == 0)
        {
            if (S_ISDIR(s.st_mode))
                exists = true;
        }
        else
        {
            if (errno == ENOENT)
                result = STEER_RESULT_SUCCESS;
            else
                result = STEER_CHECK_ERROR(errno);
        }
    }
    return exists;
}

// =================================================================================================
//  STEER_CreateDirectory
// =================================================================================================
int32_t STEER_CreateDirectory (const char* directoryPath, mode_t mode)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool exists = false;

    // Make sure directory doesn't already exist
    if (!STEER_DirectoryExists(directoryPath))
    {
        const char* path = NULL;
        const char pathSeparator = '/';
        char* tempPath = NULL;
        size_t len = strlen(directoryPath) + 1;

        result = STEER_AllocateMemory(len, (void**)&tempPath);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Setup
            path = directoryPath;

            while ((path = strchr(path, pathSeparator)) != NULL)
            {
                if ((path != directoryPath) && (*(path - 1) == pathSeparator))
                {
                    path++;
                    continue;
                }

                memset((void*)tempPath, 0, len);
                memcpy((void*)tempPath, directoryPath, path - directoryPath);
                tempPath[path - directoryPath] = 0x00;
                path++;

                if (mkdir(tempPath, mode) != 0)
                {
                    if (errno != EEXIST)
                    {
                        result = STEER_CHECK_ERROR(errno);
                        break;
                    }
                }
            }

            // Clean up
            STEER_FreeMemory((void**)&tempPath);

            if (result == STEER_RESULT_SUCCESS)
            {
                if (mkdir(directoryPath, mode) != 0)
                {
                    if (errno != EEXIST)
                        result = STEER_CHECK_ERROR(errno);
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_DirectoryHasTrailingSlash
// =================================================================================================
bool STEER_DirectoryHasTrailingSlash (const char* directoryPath)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool hasSlash = false;

    // Check arguments
    result = STEER_CHECK_STRING(directoryPath);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Check for trailing slash
        size_t len = strlen(directoryPath);

        hasSlash = (directoryPath[len - 1] == '/') ? true : false;
    }
    return hasSlash;
}

// =================================================================================================
//  STEER_TrimFileExtensionFromPath
// =================================================================================================
int32_t STEER_TrimFileExtensionFromPath (const char* pathString,
                                         const char* fileExtension,
                                         char** trimmedPath)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(fileExtension);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(trimmedPath);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *trimmedPath = NULL;

        // Does the path have a file extension?
        if (STEER_PathHasFileExtension(pathString))
        {
            // Duplicate path string for work
            result = STEER_AllocateMemory(strlen(pathString) + 1, (void**)trimmedPath);
            if (result == STEER_RESULT_SUCCESS)
            {
                char* extensionPtr = NULL;

                strcpy(*trimmedPath, pathString);
                extensionPtr = strrchr(*trimmedPath, '.');
                *extensionPtr = 0x00;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ValidateFileString
// =================================================================================================
int32_t STEER_ValidateFileString (const char* fileString,
                                  char** filePath)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(fileString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(filePath);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        struct stat fileStat;

        // Setup
        *filePath = NULL;

        // Get file statistics
        memset((void*)&fileStat, 0, sizeof(struct stat));
        result = stat(fileString, &fileStat);
        if (result != STEER_RESULT_SUCCESS)
            result = STEER_CHECK_ERROR(errno);
        
        if (result == STEER_RESULT_SUCCESS)
        {
            // Is this a file?
            result = STEER_CHECK_CONDITION(S_ISREG(fileStat.st_mode), STEER_RESULT_NOT_A_FILE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Yes; check file size and make sure it's not empty
                result = STEER_CHECK_CONDITION((fileStat.st_size > 0), STEER_RESULT_EMPTY_FILE);
            }
            else
            {
                // Is this a readable device?
                result = STEER_CHECK_CONDITION(S_ISCHR(fileStat.st_mode), STEER_RESULT_NOT_A_FILE); // TODO: Need distinct result code here
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_DuplicateString(fileString, filePath);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ValidatePathString
// =================================================================================================
int32_t STEER_ValidatePathString (const char* pathString,
                                  bool createMissingDirectoriesInPath,
                                  char** path)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(path);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *path = NULL;

        // Is this a file path?
        if (STEER_PathHasFileExtension(pathString))
        {
            bool exists = false;
            char* dirPath = NULL;

            // This appears to be a file path
            // Duplicate the path
            result = STEER_DuplicateString(pathString, &dirPath);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the directory path
                dirPath = dirname(dirPath);

                // If directory path doesn't exist, check to see whether we should create it
                if (!STEER_DirectoryExists(dirPath) && (createMissingDirectoriesInPath == true))
                    result = STEER_CreateDirectory(dirPath, 0777);

                free((void*)dirPath);
                dirPath = NULL;
            }
            else    // Assume this is a directory path
            {
                // If directory path doesn't exist, check to see whether we should create it
                if (!STEER_DirectoryExists(pathString) && (createMissingDirectoriesInPath == true))
                    result = STEER_CreateDirectory(pathString, 0777);
            }
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(pathString, path);
    }
    return result;
}

// =================================================================================================
//  STEER_FileExists
// =================================================================================================
bool STEER_FileExists (const char* filePath)
{
    bool exists = false;

    // Check arguments
    int32_t result = STEER_CHECK_STRING(filePath);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Check for existence
        result = access(filePath, F_OK);
        result = STEER_CHECK_ERROR(result);
        if (result == 0)
            exists = true;
    }
    return exists;
}

// =================================================================================================
//  STEER_FileSize
// =================================================================================================
int32_t STEER_FileSize (const char* filePath,
                        size_t* fileSizeInBytes)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(fileSizeInBytes);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *fileSizeInBytes = 0;

        // Does the file exist?
        if (STEER_FileExists(filePath))
        {
            // Open it for binary read
            FILE* fp = fopen(filePath, "rb");	
            result = STEER_CHECK_CONDITION((fp != NULL), errno);	    
            if (result == STEER_RESULT_SUCCESS)
            {
                // Seek to EOF
                result = STEER_CHECK_ERROR(fseek(fp, 0, SEEK_END));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get file size
                    *fileSizeInBytes = (size_t)ftell(fp);
                }
                
                // Clean up
                (void)fclose(fp);
            }
        }
        else    // File doesn't exist
            result = ENOENT;
    }
    return result;
}

// =================================================================================================
//  STEER_IsReadableDevicePath
// =================================================================================================
bool STEER_IsReadableDevicePath (const char* pathString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool isReadableDevicePath = false;

    // Check arguments
    result = STEER_CHECK_STRING(pathString);
    if (result == STEER_RESULT_SUCCESS)
    {
        struct stat fileStat;

        // Get file statistics
        memset((void*)&fileStat, 0, sizeof(struct stat));
        result = stat(pathString, &fileStat);
        if (result != STEER_RESULT_SUCCESS)
            result = STEER_CHECK_ERROR(errno);

        if (result == STEER_RESULT_SUCCESS)
            isReadableDevicePath = S_ISCHR(fileStat.st_mode);
    }
    return isReadableDevicePath;
}
                            
// =================================================================================================
//  STEER_OpenFile
// =================================================================================================
int32_t STEER_OpenFile (const char* filePath,
                        bool forWriting,
                        bool asBinary,
                        FILE** fileReference)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(fileReference);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        FILE* fp = NULL;

        // Setup
        *fileReference = NULL;

        // Open the file
        fp = fopen(filePath, forWriting ? (asBinary ? "wb" : "w") : (asBinary ? "rb" : "r"));
        result = STEER_CHECK_CONDITION((fp != NULL), errno);
        if (result == STEER_RESULT_SUCCESS)
            *fileReference = fp;
    }   // cppcheck-suppress resourceLeak
    return result;
}

// =================================================================================================
//  STEER_CloseFile
// =================================================================================================
int32_t STEER_CloseFile (FILE** fileReference)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(fileReference);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*fileReference != NULL)
        {
            // Close the file
            (void)fclose(*fileReference);
            *fileReference = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_WriteTextFile
// =================================================================================================
int32_t STEER_WriteTextFile (const char* filePath,
                            char* textBuffer)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(textBuffer);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(textBuffer);
    if (result == STEER_RESULT_SUCCESS)
    {
        FILE* fp = NULL;

        // Open the file
        result = STEER_OpenFile(filePath, true, false, &fp);
        if (result == STEER_RESULT_SUCCESS)
        {
            result = fprintf(fp, textBuffer);

            if (result < 0)
            {
                result = errno;
                STEER_CHECK_ERROR(errno);
            } 
            else if (result == 0)
                result = STEER_RESULT_FAILURE;
            else
                result = STEER_RESULT_SUCCESS;
            // Clean up
            (void)STEER_CloseFile(&fp);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ReadTextFile
// =================================================================================================
int32_t STEER_ReadTextFile (const char* filePath,
                            uint8_t** textBuffer)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(textBuffer);
    if (result == STEER_RESULT_SUCCESS)
    {
        FILE* fp = NULL;

        // Setup
        *textBuffer = NULL;

        // Open the file
        result = STEER_OpenFile(filePath, false, false, &fp);
        if (result == STEER_RESULT_SUCCESS)
        {
            struct stat fileStat;

            // Get the file size
            memset((void*)&fileStat, 0, sizeof(struct stat));
            result = stat(filePath, &fileStat);
            if (result != STEER_RESULT_SUCCESS)
            {
                result = errno;
                STEER_CHECK_ERROR(errno);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_CHECK_CONDITION((fileStat.st_size > 0), STEER_RESULT_EMPTY_FILE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    uint8_t* buf = NULL;
                    size_t bytesRead = 0;

                    // Allocate space
                    result = STEER_AllocateMemory(fileStat.st_size + 1, (void**)&buf);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        bytesRead = fread(buf, sizeof(uint8_t), fileStat.st_size, fp);
                        result = STEER_CHECK_CONDITION((bytesRead == fileStat.st_size), errno);
                        if (result == STEER_RESULT_SUCCESS)
                            *textBuffer = buf;
                        else
                            STEER_FreeMemory((void**)&buf);
                    }
                }
            }

            // Clean up
            (void)STEER_CloseFile(&fp);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ScanDirectoryForFilesWithExtension
// =================================================================================================
int32_t STEER_ScanDirectoryForFilesWithExtension (const char* directory,
                                                  const char* fileExtension,
                                                  uint32_t* fileCount,
                                                  tSTEER_FilePath** filePaths)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint32_t count = 0;
    tSTEER_FilePath* list = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(directory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(fileExtension);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(fileCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(filePaths);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        DIR* fileDir = NULL;
        struct dirent* dir = NULL;

        // Setup
        *fileCount = 0;
        *filePaths = NULL;

        // Open the directory
        fileDir = opendir(directory);
        result = STEER_CHECK_POINTER(fileDir);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Loop through directory contents
            do
            {
                // Read a directory
                dir = readdir(fileDir);
                if (dir != NULL)
                {
                    // Skip the ".", "..", and ".DS_Store" directories
                    if ((strcmp(dir->d_name, ".") != 0) &&
                        (strcmp(dir->d_name, "..") != 0) &&
                        (strcmp(dir->d_name, ".DS_Store") != 0))
                    {
                        // Does this entry have the specified file extension?
                        const char* ext = strrchr(dir->d_name, '.');
                        if ((ext != NULL) && (strcmp(ext, fileExtension) == 0))
                        {
                            // Bump count
                            count++;

                            // If this is the first item found, allocate space for list
                            if (list == NULL)
                                result = STEER_AllocateMemory(sizeof(tSTEER_FilePath),
                                                              (void**)&list);
                            
                            else    // Grow the list by one item
                            {
                                result = STEER_ReallocateMemory(sizeof(tSTEER_FilePath) * (count - 1),
                                                                sizeof(tSTEER_FilePath) * count,
                                                                (void**)&list);
                                if (result != STEER_RESULT_SUCCESS)
                                {
                                    // It didn't
                                    STEER_FreeMemory((void**)&list);
                                    count = 0;
                                    break;
                                }
                            }

                            // Check status
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Copy item to list
                                memset((void*)&(list[count - 1]), 0, sizeof(tSTEER_FilePath));
                                result = STEER_CHECK_CONDITION(strlen(directory) + strlen("/") + (strlen(dir->d_name) + 1 < STEER_PATH_MAX_LENGTH), 
                                    STEER_RESULT_BUFFER_TOO_SMALL);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    size_t len = strlen(directory);
                                    strcpy((char*)&(list[count - 1]), directory);
                                    if (directory[len - 1] != '/')
                                        strcat((char*)&(list[count - 1]), "/");
                                    strcat((char*)&(list[count - 1]), dir->d_name);
                                }
                                else
                                {
                                    STEER_FreeMemory((void**)&list);
                                    count = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
            } 
            while (dir != NULL);   

            // Clean up
            closedir(fileDir);
            fileDir = NULL;     
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        *fileCount = count;
        *filePaths = list;
    }
    return result;
}

// =================================================================================================
//  STEER_GetProgramDirectory
// =================================================================================================
int32_t STEER_GetProgramDirectory (char** programDirectoryPath)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(programDirectoryPath);
    if (result == STEER_RESULT_SUCCESS)
    {
        int32_t len = 0;
        char* programPath = NULL;

        // Setup
        *programDirectoryPath = NULL;
        
        len = wai_getExecutablePath(NULL, 0, NULL);
        result = STEER_CHECK_CONDITION((len > 0), STEER_RESULT_EMPTY_STRING);
        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_AllocateMemory(sizeof(char) * (len + 1), 
                                          (void**)&programPath);
            if (result == STEER_RESULT_SUCCESS)
            {
                int32_t dirNameLen = 0;

                (void) wai_getExecutablePath(programPath,
                                             len,
                                             &dirNameLen);
                result = STEER_CHECK_CONDITION((strlen(programPath) == len), 
                                               STEER_RESULT_BUFFER_LENGTH_MISMATCH);
                if (result == STEER_RESULT_SUCCESS)
                {
                    char* dirPath = programPath;

                    dirPath = dirname(dirPath);
                    result = STEER_AllocateMemory(dirNameLen + 1,
                                                  (void**)programDirectoryPath);
                    if (result == STEER_RESULT_SUCCESS)
                        strcpy(*programDirectoryPath, dirPath);
                }

                // Clean up
                STEER_FreeMemory((void**)&programPath);
            }
        }
    }
    return result;
}
    
// =================================================================================================
//  STEER_ScanProgramDirectory
// =================================================================================================
int32_t STEER_ScanProgramDirectory (const char* programDirectory,
                                    uint32_t* programCount,
                                    tSTEER_FileName** programNames)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint32_t count = 0;
    tSTEER_FileName* list = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(programDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(programCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(programNames);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        DIR* progDir = NULL;
        struct dirent* dir = NULL;

        // Setup
        *programCount = 0;
        *programNames = NULL;

        // Open the program directory
        progDir = opendir(programDirectory);
        result = STEER_CHECK_POINTER(progDir);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Loop through directory contents
            do
            {
                // Read a directory
                dir = readdir(progDir);
                if (dir != NULL)
                {
                    // Skip the ".", "..", and ".DS_Store" directories and
                    // files with file extensions
                    if ((strcmp(dir->d_name, ".") != 0) &&
                        (strcmp(dir->d_name, "..") != 0) &&
                        (strcmp(dir->d_name, ".DS_Store") != 0) &&
                        (strrchr(dir->d_name, '.') == NULL))
                    {
                        // Bump count
                        count++;

                        // If this is the first item found, allocate space for list
                        if (list == NULL)
                            result = STEER_AllocateMemory(sizeof(tSTEER_FileName),
                                                          (void**)&list);
                        
                        else    // Grow the list by one item
                        {
                            result = STEER_ReallocateMemory(sizeof(tSTEER_FileName) * (count - 1),
                                                            sizeof(tSTEER_FileName) * count,
                                                            (void**)&list);
                            if (result != STEER_RESULT_SUCCESS)
                            {
                                // It didn't
                                STEER_FreeMemory((void**)&list);
                                count = 0;
                                break;
                            }
                        }

                        // Check status
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Copy item to list
                            memset((void*)&(list[count - 1]), 0, sizeof(tSTEER_FileName));
                            result = STEER_CHECK_CONDITION((strlen(basename(dir->d_name)) < STEER_FILE_NAME_MAX_LENGTH), 
                                STEER_RESULT_BUFFER_TOO_SMALL);
                            if (result == STEER_RESULT_SUCCESS)
                                strcpy((char*)&(list[count - 1]), basename(dir->d_name));
                            else
                            {
                                STEER_FreeMemory((void**)&list);
                                count = 0;
                                break;
                            }
                        }
                    }
                }
            } 
            while (dir != NULL);   

            // Clean up
            closedir(progDir);
            progDir = NULL;     
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        *programCount = count;
        *programNames = list;
    }
    return result;
}

// =================================================================================================
//  STEER_ProgramAvailable
// =================================================================================================
bool STEER_ProgramAvailable (const char* programName,
                             uint32_t programCount,
                             tSTEER_FileName* programNames)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool available = false;

    // Check arguments
    result = STEER_CHECK_STRING(programName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((programCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(programNames);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint32_t i = 0;

        for (i = 0; i < programCount; i++)
        {
            if (strcmp(programNames[i], programName) == 0)
            {
                available = true;
                break;
            }
        }
    }
    return available;
}

// =================================================================================================
