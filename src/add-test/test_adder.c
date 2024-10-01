// =================================================================================================
//! @file test_adder.c
//! @author Alex Magyari (alexander@anametric.com)
//! @brief This file implements a program to add a new test framework to the STEER suite.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-05-19
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_file_system_utilities.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_schedule_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <ctype.h>
// =================================================================================================
//  Private constants
// =================================================================================================

//  Command line options
#define HELP_CMD                  "help"
#define BUILD_OPTIONS_FOLDER_CMD  "build-folder-path"
#define TEST_NAME_CMD             "test-name"
#define TEST_FOLDER_CMD           "test-folder"

//  Command line options
#define HELP_OPTION                         0
#define BUILD_OPTIONS_FOLDER_OPTION         2
#define TEST_NAME_OPTION                    3
#define TEST_FOLDER_OPTION                  4

//  Short command line strings
#define HELP_SHORT_CMD                    'h'
#define BUILD_OPTIONS_FOLDER_SHORT_CMD    's'
#define TEST_NAME_SHORT_CMD               't'
#define TEST_FOLDER_SHORT_CMD             'f'
#define UNKNOWN_SHORT_CMD                 '?'

// Build configuration file list
static const char* TEST_FILE_NAME               = "/build_files/test_names.txt";
static const char* TEST_FOLDERS_FILE_NAME       = "/build_files/test_folder_names.txt";
static const char* TEST_DATAS_FILE_NAME         = "/build_files/test_data_names.txt";
static const char* VALIDATION_FOLDER_NAME       = "/test/validation/nist-sts/"; 
static const char* MAKEFILE_TEMPLATE_FILE_NAME  = "/build_files/template/makefile";
static const char* C_TEMPLATE_FILE_NAME         = "/build_files/template/template.c";
static const char* C_MT_TEMPLATE_FILE_NAME      = "/build_files/template/template_mt.c";
static const char* C_ST_TEMPLATE_FILE_NAME      = "/build_files/template/template_st.c";
static const char* VALIDATION_FILE_NAME         = "/src/run-validations/validation_checks.json";
static const char* TEST_SCHEDULE_FILE_NAME      = "/src/test-scheduler/validation_test_schedule.json";
static const char* VALIDATION_MEASURED_TEMPLATE = "\t\t\t\"measured\": \"./results/validation/nist-sts/testfolder/test_report_testname.json\",\n";
static const char* VALIDATION_EXPECTED_TEMPLATE = "\t\t\t\"expected\": \"./test/validation/nist-sts/testfolder/expected_results_testname.json\"\n";
static const char* TEST_SCHEDULE_PREFIX       = 
        "\t\t\t\{\n"
        "\t\t\t\t\"program name\": \"nist_sts_testnameus\",\n"
        "\t\t\t\t\"profiles\": [";
static const char* TEST_SCHEDULE_DATA_TEMPLATE = 
        "\t\t\t\t\t{\n"
        "\t\t\t\t\t\t\"profile id\": \"dataname\",\n"
        "\t\t\t\t\t\t\"input\": \"./data/validation/nist-sts/dataname.bin\",\n"
        "\t\t\t\t\t\t\"parameters\": \"./test/validation/nist-sts/testfolder/parameters_dataname.json\",\n"
        "\t\t\t\t\t\t\"report\": \"./results/validation/nist-sts/testfolder/test_report_dataname.json\"\n"
        "\t\t\t\t\t}";
static const char* TEST_SCHEDULE_SUFFIX = 
        "\t\t\t\t]\n"
        "\t\t\t}";

//  Options array
static const char kShortOptions[] = { HELP_SHORT_CMD,
                                      BUILD_OPTIONS_FOLDER_SHORT_CMD, ':',
                                      TEST_NAME_SHORT_CMD, ':',
                                      TEST_FOLDER_SHORT_CMD, ':',
                                      0x00 };


struct FileArray {
    int length;
    char ** strings;
};

// =================================================================================================
//  Private prototypes
// =================================================================================================
static void PrintCmdLineHelp (const char* programName);

int32_t LoadBuildFiles(const char * path, char ** sourceDirectory, char ** testNames, 
                       char ** testFolders, char ** testDatas);

int32_t BuildTestFolder(char * buildFolder, char * sourceDirectory, char * testname,
                        struct FileArray * createdFolders);

int32_t BuildFile(char * fileTextBuf, char * testname, char ** modifiedFileBuf);

int32_t BuildValidationCheck(const char * sourceDirectory, const char * expectedTemplate,
                             const char * measuredTemplate, const char * testFolder,
                             const char * dataNames, struct FileArray * modifiedFiles);

int32_t BuildValidationFolder(const char * sourceDirectory, const char * testName,
                              const char * testFolder, const char * dataNames,
                              struct FileArray * createdFiles,
                              struct FileArray * createdFolders);

int32_t BuildTestSchedule(const char * sourceDirectory, const char * testName,
                          const char * testFolder, const char * dataNames,
                          struct FileArray * modifiedFiles);

int32_t WriteBuildFiles(const char * path, const char * testName, 
                        const char * testNames, const char * testFolders, 
                        struct FileArray * modifiedFiles); 

int32_t AppendToStringArray(const char * newString, struct FileArray * arr);

int32_t FreeStringArray(struct FileArray * arr);

int32_t InitializeStringArray(struct FileArray * arr);

void PrintStringArray(const char * prefix, struct FileArray * arr);

// =================================================================================================
//  PrintStringArray
// =================================================================================================
void PrintStringArray(const char * prefix, struct FileArray * arr)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(arr);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(prefix);

    if (result == STEER_RESULT_SUCCESS)
    {
        fprintf(stdout, "\t%s\n", prefix);
        for (int i = 0; i < arr->length; i ++)
            fprintf(stdout,"\t\t%s\n", arr->strings[i]); 
    }
}
// =================================================================================================
//  FreeStringArray
// =================================================================================================
int32_t InitializeStringArray(struct FileArray * arr)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(arr);
    if (result == STEER_RESULT_SUCCESS)
    {
        arr->length = 0;
        arr->strings = NULL;
    }

    return result;
}
// =================================================================================================
//  FreeStringArray
// =================================================================================================
int32_t FreeStringArray(struct FileArray * arr)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(arr);
    
    if (result == STEER_RESULT_SUCCESS)
    {
        for (int i = 0; i < arr->length; i++)
            STEER_FreeMemory((void **) &(arr->strings[i]));
        STEER_FreeMemory((void **) &(arr->strings));
    }
    
    return result;
}
// =================================================================================================
//  ApendToStringArray
// =================================================================================================
int32_t AppendToStringArray(const char * newString, 
                            struct FileArray * arr)
{

    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_STRING(newString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(arr);
    
    if (result == STEER_RESULT_SUCCESS)
    {
        int num = arr->length;
        
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReallocateMemory(num * sizeof(char **), 
                            (num + 1) * sizeof(char **), (void **)&(arr->strings));
        if (result == STEER_RESULT_SUCCESS)
            arr->strings[num] = NULL;
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(newString, &(arr->strings[num]));
        if (result == STEER_RESULT_SUCCESS)
            arr->length = arr->length + 1;
    }
    return result;
}

// =================================================================================================
//  PrintCmdLineHelp
// =================================================================================================
void PrintCmdLineHelp (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s <path>\tPath to the steer parent directory.\n", 
           BUILD_OPTIONS_FOLDER_SHORT_CMD, 
           BUILD_OPTIONS_FOLDER_CMD);
    printf("\t-%c, --%s <test-name>\tName of the new test.\n", 
           TEST_NAME_SHORT_CMD, 
           TEST_NAME_CMD);
    printf("\t-%c, --%s <test-folder>\tFolder to place the test in. Defaults to test name.\n", 
           TEST_FOLDER_SHORT_CMD, 
           TEST_FOLDER_CMD);
    printf("\t-%c, --%s\t\t\tPrints this usage notice.\n", 
           HELP_SHORT_CMD, 
           HELP_CMD);
    return;
}


// =================================================================================================
//  BuildTestSchedule
// =================================================================================================
int32_t BuildTestSchedule(const char * sourceDirectory,
                          const char * testName,
                          const char * testFolder,
                          const char * dataNames,
                          struct FileArray * modifiedFiles)
{

    int32_t result = STEER_RESULT_SUCCESS;
    
    result = STEER_CHECK_STRING(sourceDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(dataNames);

    if (result == STEER_RESULT_SUCCESS)
    {
        const char * line = NULL;
        char * fileName = NULL;
        char * fileBuf = NULL;
        char * strBuf1 = NULL;
        char * strBuf2 = NULL;
        char * fileSuffix = NULL;

        result = STEER_DuplicateString(sourceDirectory, &fileName);
        // Read test schedule into buffer
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&fileName, TEST_SCHEDULE_FILE_NAME);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReadTextFile(fileName, (uint8_t **) &fileBuf);

        if (fileBuf != NULL)
        {
            int originalLength = strlen(fileBuf);
            
            // Remove everything after last json object
            char * ptr = strrchr(fileBuf, '\"');
            ptr = strchr(ptr, ']');
            ptr = strchr(ptr, '}') + 1;
            char temp = *ptr;
            int newLen = (void *)ptr - (void *)fileBuf + 3;

            STEER_DuplicateString(ptr, &fileSuffix);

            STEER_ReallocateMemory(originalLength + 1, newLen, (void **) &fileBuf);
            ptr = fileBuf + newLen - 3;
            *(ptr++) = ',';
            *(ptr++) = temp;
            *ptr = '\0';
        } else {
            result = STEER_RESULT_FAILURE;
        }

        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&fileBuf, TEST_SCHEDULE_PREFIX);
        
        if (result == STEER_RESULT_SUCCESS)
            STEER_ReplaceSubstring(testName, " ", "_", &strBuf2);
        if (result == STEER_RESULT_SUCCESS)
            STEER_ReplaceSubstring(fileBuf, "testnameus", strBuf2, &strBuf1);
        if (result == STEER_RESULT_SUCCESS)
        {
            STEER_FreeMemory((void **) &strBuf2);
            STEER_FreeMemory((void **) &fileBuf);
        }
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(strBuf1, &fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            STEER_FreeMemory((void **) &strBuf1);

        if (result == STEER_RESULT_SUCCESS)
        {
            line = dataNames;
            int useComma = 0;
            while (line)
            {
                // Copy each line into its own string and add a null term
                char * lineBuf = NULL;

                char * nextLine = strchr(line, '\n');
                int lineLen = nextLine ? (nextLine - line) : strlen(line);
                result = STEER_AllocateMemory(lineLen + 1,(void **)&lineBuf);
                
                if ((result == STEER_RESULT_SUCCESS) && (nextLine > line))
                {
                    memcpy(lineBuf, line, lineLen);
                    lineBuf[lineLen] = '\0';

                    // Finished formatting line into individual string
                    // Now build new JSON params
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        if (useComma)
                            result = STEER_ConcatenateString(&fileBuf, ",\n");
                        else
                            result = STEER_ConcatenateString(&fileBuf, "\n");
                    }
                    useComma = 1;
                    STEER_DuplicateString(TEST_SCHEDULE_DATA_TEMPLATE, &strBuf1);

                    if (result == STEER_RESULT_SUCCESS)
                        STEER_ReplaceSubstring(strBuf1, "dataname", lineBuf, &strBuf2);
                    if (result == STEER_RESULT_SUCCESS)
                        STEER_FreeMemory((void **) &strBuf1);
                    if (result == STEER_RESULT_SUCCESS)
                        STEER_ConcatenateString(&fileBuf, strBuf2);
                    STEER_FreeMemory((void **) &strBuf1);
                    STEER_FreeMemory((void **) &strBuf2);
                }

                STEER_FreeMemory((void **) &lineBuf);
                line = nextLine ? (nextLine + 1) : NULL;
            }

            if (result == STEER_RESULT_SUCCESS)
                STEER_ConcatenateString(&fileBuf, "\n");
            if (result == STEER_RESULT_SUCCESS)
                STEER_ReplaceSubstring(fileBuf, "testfolder", testFolder, &strBuf2);
            if (result == STEER_RESULT_SUCCESS)
                STEER_FreeMemory((void **) &fileBuf);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_DuplicateString(strBuf2, &fileBuf);
            if (result == STEER_RESULT_SUCCESS)
                STEER_ConcatenateString(&fileBuf, TEST_SCHEDULE_SUFFIX);
            if (result == STEER_RESULT_SUCCESS)
                STEER_ConcatenateString(&fileBuf, fileSuffix);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_WriteTextFile(fileName, fileBuf);
            // printf("Updated file: %s\n", fileName);
            if (result == STEER_RESULT_SUCCESS)
                result = AppendToStringArray(fileName, modifiedFiles);
            STEER_FreeMemory((void **) &strBuf1);
            STEER_FreeMemory((void **) &strBuf2);
            STEER_FreeMemory((void **) &fileBuf);
            STEER_FreeMemory((void **) &fileSuffix);
            STEER_FreeMemory((void **) &fileName);
   
        }
    }
    return result;
}
// =================================================================================================
//  BuildValidationFolder
// =================================================================================================
int32_t BuildValidationFolder(const char * sourceDirectory,
                              const char * testName,
                              const char * testFolder,
                              const char * dataNames,
                              struct FileArray * createdFiles,
                              struct FileArray * createdFolders)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    result = STEER_CHECK_STRING(sourceDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testFolder);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(dataNames);

    if (result == STEER_RESULT_SUCCESS)
    {
        const char * line = NULL;
        char * testFolderDir = NULL;
        char * templateDir = NULL;
        char * modifiedFileBuf = NULL;
        char * strBuf = NULL;
        
        result = STEER_DuplicateString(sourceDirectory, &testFolderDir);
        // Make new folder
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&testFolderDir, VALIDATION_FOLDER_NAME);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&testFolderDir, testFolder);
        if (!STEER_DirectoryExists(testFolderDir))
        {
            result = STEER_CreateDirectory(testFolderDir, 0777);
            if (result == STEER_RESULT_SUCCESS)
                //fprintf(stdout, "Created folder %s\n", testFolderDir);
                result = AppendToStringArray(testFolderDir, createdFolders);
        }
        else
        {
            fprintf(stdout, "Folder %s already exists\n", testFolderDir);
            result = STEER_RESULT_FAILURE;
        }
        

        line = dataNames;
        // Copy each expected_results_* and parameters_* for each template json file
        while (line)
        {
            // Copy each line into its own string and add a null term
            int lineLen;
            char * lineBuf = NULL;
            char * nextLine = NULL;
            char * fileBuf = NULL;

            nextLine = strchr(line, '\n');
            lineLen = nextLine ? (nextLine - line) : strlen(line);
            result = STEER_AllocateMemory(lineLen + 1,(void **)&lineBuf);
            
            if ((result == STEER_RESULT_SUCCESS) && (nextLine > line))
            {
                memcpy(lineBuf, line, lineLen);
                lineBuf[lineLen] = '\0';

                // Load test data expected results
                result = STEER_DuplicateString(sourceDirectory, &templateDir);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, "/build_files/template/validation/");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, "expected_results_");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, lineBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, ".json");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ReadTextFile(templateDir, (uint8_t **) &fileBuf);
                if (result == STEER_RESULT_NOT_A_FILE)
                {
                    // Just skip the file if it doesn't exist. Either the user deleted it from the template
                    // folder, or added a new test data to the data set but did not add the template file.
                    STEER_FreeMemory((void **) &fileBuf);
                    STEER_FreeMemory((void **) &lineBuf);
                    continue;
                }
                // Write the file
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_DuplicateString(testFolderDir, &strBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, "/expected_results_");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, lineBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, ".json");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_WriteTextFile(strBuf, fileBuf);
                if (result == STEER_RESULT_SUCCESS)
                    AppendToStringArray(strBuf, createdFiles);
                    //fprintf(stdout, "Created file %s\n", strBuf);

                STEER_FreeMemory((void **) &fileBuf);
                STEER_FreeMemory((void **) &strBuf);
                STEER_FreeMemory((void **) &templateDir);

                // Load test data parameters
                result = STEER_DuplicateString(sourceDirectory, &templateDir);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, "/build_files/template/validation/");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, "parameters_");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, lineBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&templateDir, ".json");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ReadTextFile(templateDir, (uint8_t **) &fileBuf);
                if (result == STEER_RESULT_NOT_A_FILE)
                {
                    // Just skip the file if it doesn't exist. Either the user deleted it from the template
                    // folder, or added a new test data to the data set but did not add the template file.
                    STEER_FreeMemory((void **) &fileBuf);
                    STEER_FreeMemory((void **) &lineBuf);
                    continue;
                }
                // Modify the file contents
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ReplaceSubstring(fileBuf, "__testnamefull__", testName, &modifiedFileBuf);
                // Write the file
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_DuplicateString(testFolderDir, &strBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, "/parameters_");
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, lineBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ConcatenateString(&strBuf, ".json"); 
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_WriteTextFile(strBuf, modifiedFileBuf);
                if (result == STEER_RESULT_SUCCESS)
                    result = AppendToStringArray(strBuf, createdFiles);
                    // fprintf(stdout, "Created file %s\n", strBuf);

                STEER_FreeMemory((void **) &strBuf);
                STEER_FreeMemory((void **) &modifiedFileBuf);
                STEER_FreeMemory((void **) &fileBuf);
                STEER_FreeMemory((void **) &lineBuf);
            }

            line = nextLine ? (nextLine + 1) : NULL;
    
            STEER_FreeMemory((void **) &templateDir);
            STEER_FreeMemory((void **) &modifiedFileBuf);
            STEER_FreeMemory((void **) &strBuf);
            STEER_FreeMemory((void **) &lineBuf);
        }

        STEER_FreeMemory((void **) &testFolderDir);
    }
    return result;
}   

// =================================================================================================
//  BuildValidationString
// =================================================================================================
int32_t BuildValidationCheck(const char * sourceDirectory,
                             const char * expectedTemplate,
                             const char * measuredTemplate,
                             const char * testFolder,
                             const char * dataNames,
                             struct FileArray * modifiedFiles)
{
    int32_t result = STEER_RESULT_SUCCESS;
    const char * line = NULL;
    char * fileName = NULL;
    char * strBuf1 = NULL;
    char * strBuf2 = NULL;
    char * fileBuf = NULL;

    result = STEER_CHECK_STRING(sourceDirectory); 
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(expectedTemplate);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(measuredTemplate);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testFolder);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(dataNames);
    
    // Load the validation checks file
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_DuplicateString(sourceDirectory, &fileName);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&fileName, VALIDATION_FILE_NAME);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReadTextFile(fileName, (uint8_t **) &fileBuf);
    }    
    
    
    if (result == STEER_RESULT_SUCCESS)
    {
        int originalLength = strlen(fileBuf);
            
        // Remove last three lines
        for (int i = 0; i < 4; i = i + 1)
        {
            strBuf1 = strrchr(fileBuf, '\n');
            *strBuf1 = '\0';
            originalLength = strlen(fileBuf);
        }
        
        STEER_ReallocateMemory(originalLength, strlen(fileBuf) + 1, (void **) &fileBuf);
        STEER_ConcatenateString(&fileBuf, "\n\t\t}");


        line = dataNames;
        strBuf1 = NULL;

        while (line)
        {
            // Copy each line into its own string and add a null term
            char * lineBuf = NULL;

            char * nextLine = strchr(line, '\n');
            int lineLen = nextLine ? (nextLine - line) : strlen(line);
        
            result = STEER_AllocateMemory(lineLen + 1,(void **)&lineBuf);
            
            if ((result == STEER_RESULT_SUCCESS) && (nextLine > line))
            {
                memcpy(lineBuf, line, lineLen);
                lineBuf[lineLen] = '\0';

                // Finished formatting line into individual string
                // Now build new JSON params
                STEER_ConcatenateString(&fileBuf, ",\n\t\t{\n");

                if (result == STEER_RESULT_SUCCESS)
                    STEER_ReplaceSubstring(measuredTemplate, "testfolder", testFolder, &strBuf1);
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ReplaceSubstring(strBuf1, "testname", lineBuf, &strBuf2);
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ConcatenateString(&fileBuf, strBuf2);
                
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ReplaceSubstring(expectedTemplate, "testfolder", testFolder, &strBuf1);
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ReplaceSubstring(strBuf1, "testname", lineBuf, &strBuf2);
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ConcatenateString(&fileBuf, strBuf2);
                if (result == STEER_RESULT_SUCCESS)
                    STEER_ConcatenateString(&fileBuf, "\t\t}");
                STEER_FreeMemory((void **) &strBuf1);
                STEER_FreeMemory((void **) &strBuf2);
            }
            
            STEER_FreeMemory((void **) &lineBuf);
            line = nextLine ? (nextLine + 1) : NULL;
        }
        STEER_ConcatenateString(&fileBuf, "\n\t]\n}\n");
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_WriteTextFile(fileName, fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = AppendToStringArray(fileName, modifiedFiles);
        //printf("Updated file: %s\n", fileName);
        STEER_FreeMemory((void **) &fileBuf);
        STEER_FreeMemory((void **) &fileName);
    }
    return result;
}
// =================================================================================================
//  BuildFile
// =================================================================================================
int32_t BuildFile(char * fileTextBuf, char * testname, char ** modifiedFileBuf)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char * strBuf = NULL;
    char * fileBuf = NULL;

    result = STEER_CHECK_STRING(fileTextBuf);
    
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(modifiedFileBuf);

    if (result == STEER_RESULT_SUCCESS)
    {            
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(testname, &strBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(fileTextBuf, "__testnamefull__", strBuf, modifiedFileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(*modifiedFileBuf, &fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_SwapCharacters(testname, ' ', '_', &strBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(fileBuf, "__testnameus__", strBuf, modifiedFileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(*modifiedFileBuf, &fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(testname, " ", "", &strBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(fileBuf, "__testname__", strBuf, modifiedFileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(*modifiedFileBuf, &fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConvertStringToCamelCase(testname, &strBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(fileBuf, "__testnameskipcap__", strBuf, modifiedFileBuf);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(*modifiedFileBuf, &fileBuf);
        if (result == STEER_RESULT_SUCCESS)
            strBuf[0] = toupper(strBuf[0]);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReplaceSubstring(fileBuf, "__testnamecaps__", strBuf, modifiedFileBuf);


        STEER_FreeMemory((void **) &fileBuf);
        STEER_FreeMemory((void **) &strBuf);

    }
    
    return result;

}
    
// =================================================================================================
//  BuildTestFolder
// =================================================================================================
int32_t BuildTestFolder(char * buildFolder, 
                        char * sourceDirectory, 
                        char * testname, 
                        struct FileArray * createdFolders)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char * filePath = NULL;
    char *strBuf = NULL;
    char *fileBuf = NULL;
    char *modifiedFileBuf = NULL;
    
    if (!STEER_DirectoryExists(buildFolder))
    {
        result = STEER_CreateDirectory(buildFolder, 0777);
        if (result == STEER_RESULT_SUCCESS)
            // fprintf(stdout, "Created folder %s\n", buildFolder);
            AppendToStringArray(buildFolder, createdFolders);
    }
    else
    {
        fprintf(stdout, "Folder %s already exists\n", buildFolder);
        result = STEER_RESULT_FAILURE;
    }

    // Load makefile template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(sourceDirectory, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, MAKEFILE_TEMPLATE_FILE_NAME);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ReadTextFile(filePath, (uint8_t **)&fileBuf);

    // Modify template        
    if (result == STEER_RESULT_SUCCESS)
        result = BuildFile(fileBuf, testname, &modifiedFileBuf);
    
    // Save makefile template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(buildFolder, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "/makefile");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_WriteTextFile(filePath, modifiedFileBuf);

    STEER_FreeMemory((void **) &filePath);
    STEER_FreeMemory((void **) &fileBuf);
    STEER_FreeMemory((void **) &modifiedFileBuf);
    
    // Load base c template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(sourceDirectory, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, C_TEMPLATE_FILE_NAME);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ReadTextFile(filePath, (uint8_t **)&fileBuf);

    // Modify c template        
    result = BuildFile(fileBuf, testname, &modifiedFileBuf);

    STEER_FreeMemory((void **) &filePath);
    
    // Save c template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(buildFolder, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "/");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_SwapCharacters(testname, ' ', '_', &strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, ".c");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_WriteTextFile(filePath, modifiedFileBuf);
    
    STEER_FreeMemory((void **) &filePath);
    STEER_FreeMemory((void **) &fileBuf);
    STEER_FreeMemory((void **) &modifiedFileBuf);
    // Load c mt template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(sourceDirectory, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, C_MT_TEMPLATE_FILE_NAME);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ReadTextFile(filePath, (uint8_t **)&fileBuf);
    
    // Modify c mt template        
    result = BuildFile(fileBuf, testname, &modifiedFileBuf);

    STEER_FreeMemory((void **) &filePath);
    
    // Save c mt template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(buildFolder, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "/");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_SwapCharacters(testname, ' ', '_', &strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "_mt.c");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_WriteTextFile(filePath, modifiedFileBuf);

    STEER_FreeMemory((void **) &modifiedFileBuf);
    STEER_FreeMemory((void **) &fileBuf);
    STEER_FreeMemory((void **) &filePath);
    
    // Load c st template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(sourceDirectory, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, C_ST_TEMPLATE_FILE_NAME);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ReadTextFile(filePath, (uint8_t **)&fileBuf);
    
    // Modify c template        
    result = BuildFile(fileBuf, testname, &modifiedFileBuf);

    STEER_FreeMemory((void **) &filePath);
    
    // Save c template
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_DuplicateString(buildFolder, &filePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "/");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_SwapCharacters(testname, ' ', '_', &strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, strBuf);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_ConcatenateString(&filePath, "_st.c");
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_WriteTextFile(filePath, modifiedFileBuf);
    
    STEER_FreeMemory((void **) &modifiedFileBuf);
    STEER_FreeMemory((void **) &fileBuf);
    STEER_FreeMemory((void **) &strBuf);
    STEER_FreeMemory((void **) &filePath);
    return result;

}

// =================================================================================================
//  WriteBuildFiles
// =================================================================================================
int32_t WriteBuildFiles(const char * path,
                        const char * testName,
                        const char * testNames,
                        const char * testFolders,
                        struct FileArray * modifiedFiles) 
{    
    int32_t result = STEER_RESULT_SUCCESS;    
    char * extendedPath = NULL;
    char * strBuf1       = NULL;
    char * strBuf2       = NULL;

    if (strlen(path) > 0)
    {
        result = STEER_ExpandTildeInPath(path, &extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            if (extendedPath == NULL)
                result = STEER_DuplicateString(path, &extendedPath);
                
        // Write test file
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&extendedPath, TEST_FILE_NAME);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(testNames, &strBuf1);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&strBuf1, testName);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&strBuf1, "\n");
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_WriteTextFile(extendedPath, strBuf1);
        if (result == STEER_RESULT_SUCCESS)
            // printf("Modified file: %s\n", extendedPath);
            result = AppendToStringArray(extendedPath, modifiedFiles);
        if (result == STEER_RESULT_SUCCESS)
        {
            STEER_FreeMemory((void **)&extendedPath);
            STEER_FreeMemory((void **)&strBuf1);
        }
        // Write test folders. The 'test_folders' option is a camel case
        // alternative to the test folder name
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(path, &extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&extendedPath, TEST_FOLDERS_FILE_NAME);                
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(testFolders, &strBuf1);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConvertStringToCamelCase(testName, &strBuf2);
        if (result == STEER_RESULT_SUCCESS)
            *strBuf2 = toupper(*strBuf2);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&strBuf1, strBuf2);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&strBuf1, "\n");
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_WriteTextFile(extendedPath, strBuf1);
        if (result == STEER_RESULT_SUCCESS)
            //printf("Modified file: %s\n", extendedPath);
            AppendToStringArray(extendedPath, modifiedFiles);
        
        STEER_FreeMemory((void**)&extendedPath);
        STEER_FreeMemory((void**)&strBuf1);
        STEER_FreeMemory((void**)&strBuf2);
  
    }
    return result;
}
// =================================================================================================
//  LoadBuildFiles
// =================================================================================================
int32_t LoadBuildFiles(const char * path,
                       char ** sourceDirectory,
                       char ** testNames,
                       char ** testFolders,
                       char ** testDatas) 
{    
    int32_t result = STEER_RESULT_SUCCESS;    
    char * extendedPath = NULL;

    if (strlen(path) > 0)
    {
        result = STEER_ExpandTildeInPath(path, &extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            if (extendedPath == NULL)
                result = STEER_DuplicateString(path, &extendedPath);
                
        // Load test files
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(extendedPath, sourceDirectory);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&extendedPath, TEST_FILE_NAME);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReadTextFile(extendedPath, (uint8_t **)testNames);
        if (result == STEER_RESULT_SUCCESS)
            STEER_FreeMemory((void**)&extendedPath);
 
        // Load test folders
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(path, &extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&extendedPath, TEST_FOLDERS_FILE_NAME);                
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReadTextFile(extendedPath, (uint8_t **)testFolders);                    
        
        // Load test datas   
        if (result == STEER_RESULT_SUCCESS)
            STEER_FreeMemory((void**)&extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(path, &extendedPath);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ConcatenateString(&extendedPath, TEST_DATAS_FILE_NAME);                
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_ReadTextFile(extendedPath, (uint8_t **)testDatas);                    

        STEER_FreeMemory((void**)&extendedPath);
  
    }
    if (result != STEER_RESULT_SUCCESS)
    {
        STEER_FreeMemory((void**)testNames);
        STEER_FreeMemory((void**)testFolders);
        STEER_FreeMemory((void**)testDatas);
    }
    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check for no arguments 
    if (argc == 1)
    {
        PrintCmdLineHelp(argv[0]);
        printf("\n");
    }
    else
    {

        char* testFolders = NULL;
        char* testNames = NULL;
        char* testDatas = NULL;
        char* newTestName = NULL;
        char* newTestFolder = NULL;
        char* sourceDirectory = NULL;
        char* inputDir               = NULL;
        int32_t cmdLineOption = 0;
        int32_t optionIndex = 0;
        bool done = false;
        
        // Define command line options
        static struct option longOptions[] =
        {
            { HELP_CMD,                   no_argument,        0, 0 }, // Prints help
            { BUILD_OPTIONS_FOLDER_CMD,   required_argument,  0, 0 }, // Path to a file containing JSON test schedule
            { TEST_NAME_CMD,              required_argument,  0, 0 }, // Name of the new test
            { TEST_FOLDER_CMD,            optional_argument,  0, 0 }, // Name of the folder for the new test
            { 0,                          0,                  0, 0 }
        };

        // Parse command line arguments
        while ((cmdLineOption != -1) && (result == STEER_RESULT_SUCCESS))
        {
            cmdLineOption = getopt_long(argc,
                                        (char *const *)argv,
                                        kShortOptions,
                                        longOptions,
                                        &optionIndex);
            
            // Case on command line option
            switch (cmdLineOption)
            {
                case 0: // Long option found

                    // Help
                    if (optionIndex == HELP_OPTION)
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }

                    // Build Options Folder
                    else if (optionIndex == BUILD_OPTIONS_FOLDER_OPTION)
                    {
                        STEER_DuplicateString(optarg, &inputDir);
                    }

                    // Test name
                    else if (optionIndex == TEST_NAME_OPTION)
                    {
                        result = STEER_DuplicateString(optarg, &newTestName);
                    }

                    // Test folder name
                    else if (optionIndex == TEST_FOLDER_OPTION)
                    {
                        result = STEER_DuplicateString(optarg, &newTestFolder);
                    }

                    // Unknown
                    else
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }
                    break;

                // Help
                case HELP_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                // Test schedule file path
                case BUILD_OPTIONS_FOLDER_SHORT_CMD:
                {
                    STEER_DuplicateString(optarg, &inputDir);
                    break;
                }

                // Test name JSON
                case TEST_NAME_SHORT_CMD:
                {
                    result = STEER_DuplicateString(optarg, &newTestName);
                    break;
                }

                // Test folder JSON
                case TEST_FOLDER_SHORT_CMD:
                {
                    result = STEER_DuplicateString(optarg, &newTestFolder);
                    break;
                }
                
                // Unknown
                case UNKNOWN_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                default:
                    break;
            }
        }

        // Continue?
        if ((result == STEER_RESULT_SUCCESS) && !done)
        {
            // String array test
            struct FileArray newFolders;
            struct FileArray newFiles;
            struct FileArray modifiedFiles;

            InitializeStringArray(&newFolders);
            InitializeStringArray(&newFiles);
            InitializeStringArray(&modifiedFiles);
            
            char * testFolderDir = NULL;
            char * strBuf = NULL;
            if ((newTestName == NULL) || (inputDir == NULL))
                return STEER_RESULT_FAILURE;

            // Load build files
            result = LoadBuildFiles(inputDir, &sourceDirectory, &testNames, &testFolders, &testDatas);
            if (result != STEER_RESULT_SUCCESS)
            {
                printf("Error loading build folder. Please verify the folder path, and that the files" 
                       "'%s' and '%s' are in the steer directory specified with -s.\n", 
                        TEST_FOLDERS_FILE_NAME, TEST_FILE_NAME);
            }

            // Build test folder name
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_SwapCharacters(newTestName, ' ', '-', &newTestFolder);
           
            // Add new test folder
            result = STEER_DuplicateString(sourceDirectory, &testFolderDir);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConcatenateString(&testFolderDir, "/src/nist-sts/");
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConcatenateString(&testFolderDir, newTestFolder);
            if (result == STEER_RESULT_SUCCESS)
                result = BuildTestFolder(testFolderDir, inputDir, newTestName, &newFolders);

            // Build the validation string
            if (result == STEER_RESULT_SUCCESS)
                result = BuildValidationCheck(sourceDirectory, VALIDATION_EXPECTED_TEMPLATE,
                                              VALIDATION_MEASURED_TEMPLATE, newTestFolder, 
                                              testDatas, &modifiedFiles);
           
            // Build validation folder
            if (result == STEER_RESULT_SUCCESS)
               result = BuildValidationFolder(sourceDirectory, newTestName, newTestFolder, testDatas,
                                                &newFiles, &newFolders);

            // Update the test schedule
            if (result == STEER_RESULT_SUCCESS)
                result = BuildTestSchedule(sourceDirectory, newTestName, newTestFolder, testDatas, &modifiedFiles);

            // Write build files
            if (result == STEER_RESULT_SUCCESS)
                result = WriteBuildFiles(sourceDirectory, newTestName, testNames, testFolders, &modifiedFiles);

            // Output results
            if (result == STEER_RESULT_SUCCESS)
            {
                fprintf(stdout, "\n");
                PrintStringArray("New folders created:", &newFolders);
                PrintStringArray("New files created:", &newFiles);
                PrintStringArray("Modified files:", &modifiedFiles);
                fprintf(stdout, "\n");
            }

            FreeStringArray(&newFolders);
            FreeStringArray(&newFiles);
            FreeStringArray(&modifiedFiles);
            STEER_FreeMemory((void**)&testFolderDir);
            STEER_FreeMemory((void**)&strBuf);
    
        }
        STEER_FreeMemory((void**)&testNames);
        STEER_FreeMemory((void**)&testFolders);
        STEER_FreeMemory((void**)&testDatas);
        STEER_FreeMemory((void**)&newTestName);
        STEER_FreeMemory((void**)&newTestFolder);
        STEER_FreeMemory((void**)&sourceDirectory);
        STEER_FreeMemory((void**)&inputDir);
    }
    return result;
}

// =================================================================================================
