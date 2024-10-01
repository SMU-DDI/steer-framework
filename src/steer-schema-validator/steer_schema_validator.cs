// =================================================================================================
//! @file steer_schema_validator.cs
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a simple .NET JSON schema validator for STEER schemas.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-04-22
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
// =================================================================================================
//  Main
// =================================================================================================

using System;
using System.IO;
using System.Net.Http;
using System.Text.Json;
using Json.Schema;

// STEER JSON schema URLs
string testInfoSchemaUrl = "https://www.steer-framework.dev/schemas/test-info/0.1.0/test_info_schema.json";
string testParametersInfoSchemaUrl = "https://www.steer-framework.dev/schemas/parameters-info/0.1.0/parameters_info_schema.json";
string testParameterSetSchemaUrl = "https://www.steer-framework.dev/schemas/parameter-set/0.1.0/parameter_set_schema.json";
string testReportSchemaUrl = "https://www.steer-framework.dev/schemas/report/0.1.0/report_schema.json";
string testScheduleSchemaUrl = "https://www.steer-framework.dev/schemas/schedule/0.1.0/schedule_schema.json";
string testExpectedResultsSchemaUrl = "https://www.steer-framework.dev/schemas/expected-results/0.1.0/expected_results_schema.json";

// Temp files for STEER JSON schema
string testInfoSchemaFile = "/tmp/test_info_schema.json";
string testParametersInfoSchemaFile = "/tmp/parameters_info_schema.json";
string testParameterSetSchemaFile = "/tmp/parameter_set_schema.json";
string testReportSchemaFile = "/tmp/report_schema.json";
string testScheduleSchemaFile = "/tmp/schedule_schema.json";
string testExpectedResultsSchemaFile = "/tmp/expected_results_schema.json";

// Set up HTTP client for downloading
var httpClient = new HttpClient();

// Download schemas
await Helpers.HttpDownloadUrlToFile(httpClient, testInfoSchemaUrl, testInfoSchemaFile);
await Helpers.HttpDownloadUrlToFile(httpClient, testParametersInfoSchemaUrl, testParametersInfoSchemaFile);
await Helpers.HttpDownloadUrlToFile(httpClient, testParameterSetSchemaUrl, testParameterSetSchemaFile);
await Helpers.HttpDownloadUrlToFile(httpClient, testReportSchemaUrl, testReportSchemaFile);
await Helpers.HttpDownloadUrlToFile(httpClient, testScheduleSchemaUrl, testScheduleSchemaFile);
await Helpers.HttpDownloadUrlToFile(httpClient, testExpectedResultsSchemaUrl, testExpectedResultsSchemaFile);

// Process arguments
var cmd = "";
var schemaIsValid = true;
for (int i = 0; i < args.Length; i++)
{
    cmd = args[i];
    switch (cmd)
    {
        case "-h":
        case "--help":
            // Display help
            Helpers.PrintHelp();
            break;

        case "-e":
        case "--expected-results-json":
            // Validate expected results JSON
            var testExpectedResultsSchema = JsonSchema.FromFile(testExpectedResultsSchemaFile);
            if (i < (args.Length - 1))
            {
                var testExpectedResultsFile = File.OpenRead(args[i + 1]);
                Task<bool> testExpectedResultsValid = Helpers.ValidateJsonFile(testExpectedResultsFile, testExpectedResultsSchema);
                if (testExpectedResultsValid.Result == true)
                    Console.WriteLine("Expected results JSON is valid.");
                else
                {
                    Console.WriteLine("Expected results JSON is invalid.");
                    schemaIsValid = false;
                }
                testExpectedResultsFile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testExpectedResultsStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testExpectedResultsStr += tempStr;
                }
                var testExpectedResultsValid = Helpers.ValidateJsonString(testExpectedResultsStr, testExpectedResultsSchema);
                if (testExpectedResultsValid == true)
                    Console.WriteLine("Expected results JSON is valid.");
                else
                {
                    Console.WriteLine("Expected results JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        case "-i":
        case "--test-info-json":
            // Validate test info JSON
            var testInfoSchema = JsonSchema.FromFile(testInfoSchemaFile);
            if (i < (args.Length - 1))
            {
                var testInfoFile = File.OpenRead(args[i + 1]);
                Task<bool> testInfoValid = Helpers.ValidateJsonFile(testInfoFile, testInfoSchema);
                if (testInfoValid.Result == true)
                    Console.WriteLine("Test info JSON is valid.");
                else
                {
                    Console.WriteLine("Test info JSON is invalid.");
                    schemaIsValid = false;
                }
                testInfoFile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testInfoStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testInfoStr += tempStr;
                }
                var testInfoValid = Helpers.ValidateJsonString(testInfoStr, testInfoSchema);
                if (testInfoValid == true)
                    Console.WriteLine("Test info JSON is valid.");
                else
                {
                    Console.WriteLine("Test info JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        case "-p":
        case "--parameters-info-json":
            // Validate parameters info JSON
            var testParametersInfoSchema = JsonSchema.FromFile(testParametersInfoSchemaFile);
            if (i < (args.Length - 1))
            {
                var testParametersInfofile = File.OpenRead(args[i + 1]);
                Task<bool> testParametersInfoValid = 
                    Helpers.ValidateJsonFile(testParametersInfofile, testParametersInfoSchema);
                if (testParametersInfoValid.Result == true)
                    Console.WriteLine("Parameters info JSON is valid.");
                else
                {
                    Console.WriteLine("Parameters info JSON is invalid.");
                    schemaIsValid = false;
                }
                testParametersInfofile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testParametersInfoStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testParametersInfoStr += tempStr;
                }
                var testParametersInfoValid = 
                    Helpers.ValidateJsonString(testParametersInfoStr, testParametersInfoSchema);
                if (testParametersInfoValid == true)
                    Console.WriteLine("Parameters info JSON is valid.");
                else
                {
                    Console.WriteLine("Parameters info JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        case "-P":
        case "--parameter-set-json":
            // Validate parameter set JSON
            var testParameterSetSchema = JsonSchema.FromFile(testParameterSetSchemaFile);
            if (i < (args.Length - 1))
            {
                var testParametersSetFile = File.OpenRead(args[i + 1]);
                Task<bool> testParameterSetValid = 
                    Helpers.ValidateJsonFile(testParametersSetFile, testParameterSetSchema);
                if (testParameterSetValid.Result == true)
                    Console.WriteLine("Parameter set JSON is valid.");
                else
                {
                    Console.WriteLine("Parameter set JSON is invalid.");
                    schemaIsValid = false;
                }
                testParametersSetFile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testParameterSetStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testParameterSetStr += tempStr;
                }
                var testParameterSetValid = 
                    Helpers.ValidateJsonString(testParameterSetStr, testParameterSetSchema);
                if (testParameterSetValid == true)
                    Console.WriteLine("Parameter set JSON is valid.");
                else
                {
                    Console.WriteLine("Parameter set JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        case "-r":
        case "--report-json":
            // Validate report JSON
            var testReportSchema = JsonSchema.FromFile(testReportSchemaFile);
            if (i < (args.Length - 1))
            {
                var testReportFile = File.OpenRead(args[i + 1]);
                Task<bool> testReportValid = 
                    Helpers.ValidateJsonFile(testReportFile, testReportSchema);
                if (testReportValid.Result == true)
                    Console.WriteLine("Report JSON is valid.");
                else
                {
                    Console.WriteLine("Report JSON is invalid.");
                    schemaIsValid = false;
                }
                testReportFile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testReportStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testReportStr += tempStr;
                }
                var testReportValid = 
                    Helpers.ValidateJsonString(testReportStr, testReportSchema);
                if (testReportValid == true)
                    Console.WriteLine("Report JSON is valid.");
                else
                {
                    Console.WriteLine("Report JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        case "-s":
        case "--schedule-json":
            // Validate schedule JSON
            var testScheduleSchema = JsonSchema.FromFile(testScheduleSchemaFile);
            if (i < (args.Length - 1))
            {
                var testScheduleFile = File.OpenRead(args[i + 1]);
                Task<bool> testScheduleValid = 
                    Helpers.ValidateJsonFile(testScheduleFile, testScheduleSchema);
                if (testScheduleValid.Result == true)
                    Console.WriteLine("Schedule JSON is valid.");
                else
                {
                    Console.WriteLine("Schedule JSON is invalid.");
                    schemaIsValid = false;
                }
                testScheduleFile.Close();
            }
            else if (i == (args.Length - 1))
            {
                var tempStr = "";
                var testScheduleStr = "";
                while ((tempStr = Console.ReadLine()) != null)
                {
                    testScheduleStr += tempStr;
                }
                var testScheduleValid = 
                    Helpers.ValidateJsonString(testScheduleStr, testScheduleSchema);
                if (testScheduleValid == true)
                    Console.WriteLine("Schedule JSON is valid.");
                else
                {
                    Console.WriteLine("Schedule JSON is invalid.");
                    schemaIsValid = false;
                }
            }
            break;

        default:   
            break;
    }
}

if (schemaIsValid)
    Environment.Exit(0);
else
    Environment.Exit(-1);

// =================================================================================================
//  Helper functions
// =================================================================================================
public static class Helpers
{
    public static void PrintHelp()
    {
        Console.WriteLine("\nUsage: steer_schema_validator <arguments>\n");
        Console.WriteLine("\tAvailable command line arguments are:");
        Console.WriteLine("\t-e, --expected-results-json <path>\tValidates the supplied JSON file against the expected results schema.");
        Console.WriteLine("\t-h, --help\t\t\t\tPrints this usage notice.");
        Console.WriteLine("\t-i, --test-info-json <path>\t\tValidates the supplied JSON file against the test info schema.");
        Console.WriteLine("\t-p, --parameters-info-json <path>\tValidates the supplied JSON file against the parameters info schema.");
        Console.WriteLine("\t-P, --parameter-set-json <path>\t\tValidates the supplied JSON file against the parameter set schema.");
        Console.WriteLine("\t-r, --report-json <path>\t\tValidates the supplied JSON file against the report schema.");
        Console.WriteLine("\t-s, --schedule-json <path>\t\tValidates the supplied JSON file against the schedule schema.\n");
    }
    public static async Task HttpDownloadUrlToFile (HttpClient client, string url, string file)
    {
        try
        {
            if (File.Exists(file))
            {
                File.Delete(file);
            }
            using (var stream = await client.GetStreamAsync(url))
            {
                using (var fileStream = new FileStream(file, FileMode.CreateNew))
                {
                    await stream.CopyToAsync(fileStream);
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("Exception caught: {0}.", ex.Message);
        }
    }

    public static async Task<bool> ValidateJsonFile (System.IO.FileStream jsonToValidate, 
                                                     Json.Schema.JsonSchema jsonSchema)
    {
        bool valid = false;

        try {
            var json = await JsonDocument.ParseAsync(jsonToValidate);
            var result = jsonSchema.Validate(json.RootElement,
                                             new ValidationOptions {
                                                RequireFormatValidation = true,
                                                ValidateAs = Draft.Draft202012,
                                                OutputFormat = OutputFormat.Verbose
                                             });
            valid = result.IsValid;
        }
        catch (Exception ex)
        {
            Console.WriteLine("Exception caught: {0}.", ex.Message);
        }
        return valid;
    }

    public static bool ValidateJsonString (string jsonToValidate, 
                                           Json.Schema.JsonSchema jsonSchema)
    {
        bool valid = false;

        try {
            var json = JsonDocument.Parse(jsonToValidate);
            var result = jsonSchema.Validate(json.RootElement,
                                             new ValidationOptions {
                                                RequireFormatValidation = true,
                                                ValidateAs = Draft.Draft202012,
                                                OutputFormat = OutputFormat.Verbose
                                             });
            valid = result.IsValid;
        }
        catch (Exception ex)
        {
            Console.WriteLine("Exception caught: {0}.", ex.Message);
        }
        return valid;
    }
}

// =================================================================================================
