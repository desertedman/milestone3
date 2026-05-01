/**
*
* milestone2.cpp : This file contains the 'main' function. Program execution begins and ends there.
*
* 02/21/26 - Created by ChatGPT with prompt "write C++ program reads and parses the file: milestone2_config.json"
*            The file: "milestone2_config.json" is in the following format:
*
                {
                    "Milestone2": [
                        {
                            "files": [
                                {
                                    "testcaseFile": "milestone2.json",
                                    "outputFile": "generatedOutputFile.txt",
                                    "errorLogFile":"logFile.txt"
                                }
                            ],
                            "defaultVariables": [
                                {
                                    "numberOfIterations": 10000,
                                    "readSize": 100
                                }
                            ]
                        }
                    ]
                }
            and where the testcaseFile has the following format:
                {
                    "FileReadBenchmark": [
                        {
                            "testCase1": [
                                {
                                    "readTest": {"inputFileName": "top-output.txt", "numberOfIterations": 100}
                                }
                                ...
                            ]
                        }
                    ]
                }

2/15/2026 - create by Joseph Hui;

*/
#include "cache-manager.hpp"
#include "benchmark.hpp"

#include <climits>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <format>

#include "json.hpp"

using json = nlohmann::json;

#define _CRT_SECURE_NO_WARNINGS
#define CONFIG_FILE "milestone2_config.json"

// Global variable to be used for logging output
std::ofstream _outFile;

/**
*
* getOutFile
*
* @brief function to return pointer to outFile
*
* @param        none
*
* @return       pointer to output file
*/
std::ofstream& getOutFile() {
    return _outFile;
}


/**
*
* setOutFile
*
* @brief function to set path and open output file
*
* @param filePath       the path to output file
*
* @return               nothing
*/
void setOutFile(const std::string& filePath) {
    // Close the current file if it's already open
    if (_outFile.is_open()) {
        _outFile.close();
    }

    // Open the new file
    _outFile.open(filePath);
    if (!_outFile.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
}

/**
 * logToFileAndConsole
 *
 * @brief Logs a message to both the console and the output file.
 *
 * This helper function prints a message to the console and writes the same
 * message to the output file.
 *
 * @param message The message to log
 */
void logToFileAndConsole(std::string message) {
    // Get the output file
    std::ofstream& outFile = getOutFile();

    std::cout << message << std::endl;  // Print to console 
    outFile << message << std::endl;  // Write to file
}

/**
*
* getItemTest
*
* @brief calls CacheManager.getItem()
*
* @param    config              benchmark config
*
* @return   nothing, but output is sent to console and written to output file
*/
void getItemTest(json config, cache::CacheManager<int, std::string, bench::TbbBench> &cm) {
    int testSize = config["Milestone2"][0]["defaultVariables"][0]["testSize"];

    // generate a randome key to retrieve
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution distr(1, testSize);
    int testKey = distr(gen);

    // get the value from the Cache Manager for testKey
    // (this is the actual call for the test)
    auto val = cm.getItem(testKey);

    // if (val) {
    //    logToFileAndConsole("Retrieved key: " + std::to_string(testKey) + "; with value: '" + *val + "'");
    // } else {
    //    logToFileAndConsole("Key: " + std::to_string(testKey) + " not found (NOT EXPECTED!!!!");
    // }

    // try one that is not in the cache
    testKey = testKey * 1000;
    val = cm.getItem(testKey);

    // if (val) {
    //    logToFileAndConsole("Retrieved key: " + std::to_string(testKey) + "; with value (NOT EXPECTED!!!): " + *val);
    // } else {
    //    logToFileAndConsole("Key: " + std::to_string(testKey) + " not found (expected)!");
    // }
}

/**
*
* timeWrapper 
* 
* @brief Simple wrapper function that times a limited number of functions
*
* @param    config              benchmark config
*
* @return   nothing, but output is sent to console and written to output file
*/
void timeWrapper(json config) {
    // set the start time
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // output some helpful comments to the console
    std::cout << "\nStarting computation at " << std::ctime(&start_time);

    // need to write out the data for each timed iteration in the following format:
    // 
    // iter#    end time    elapsed time     
    // 
    // 1        <time1>     10
    // 2        <time2>     9
    // 3        <time3>     11
    // ...
    //
    // aggregated statistics:
    //
    // avg      min         max
    //
    // 10       9           11

    int testSize = config["Milestone2"][0]["defaultVariables"][0]["testSize"];
    int testIterations = config["Milestone2"][0]["defaultVariables"][0]["testIterations"];
    
    // Allocate the cache manager
    cache::CacheManager<int, std::string, bench::TbbBench> cm;

    auto cacheLoadStart = std::chrono::system_clock::now();
    // sample test load of the cache
    for (auto key = 1; key <= testSize + 1; ++key) {
        std::string value = "Test value for key: " + std::to_string(key);
        cm.add(key, value);
        // logToFileAndConsole("Added key: " + std::to_string(key) + "; value: '" + value + "'");
    }
    auto cacheLoadEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> cacheLoadTime = cacheLoadEnd - cacheLoadStart;

    // write out the head line for output file
    logToFileAndConsole("\niter#\t\tend time\telapsed time");

    // Setup random key generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution newDistr(testSize + 1, INT_MAX);  // Generate random key > testSize
    std::uniform_int_distribution existingDistr(1, testSize);       // Generate random key 1 - testSize

    // Time statistics
    double average = 0.f;
    double min = 20.f;  // Set to some arbitrary high value, so that we can take min accurately
    double max = -20.f;

    // call the specific function to time
    for (int i = 1; i < testIterations + 1; i++) {
        // Declare keys; put off generation til later
        int existingKey, newKey;
        bool ret;

        // Set start time
        auto curIterStart = std::chrono::system_clock::now();

        // add more functions here
        getItemTest(config, cm);





        // Check if cache contains newKey (should not!)
        do {
            // Generate newKey
            newKey = newDistr(gen);  // (testSize + 1, INT_MAX)

            ret = cm.contains(newKey);
            if (ret != false) {
                std::cout << "ALERT:\t\tnewKey already added! Generating new key...";
            }
            //
            // else {
            //     logToFileAndConsole("SUCCESS:\t\tnewKey not found! Adding key " + std::to_string(newKey) + " ");
            // }
        } while (ret != false);  // Validate that newKey has not already been added





        // Add newKey and value to cache
        std::string value = "Test value for key: " + std::to_string(newKey);
        ret = cm.add(newKey, value);

        // Cache is full alert
        // if (ret != true)
        //     std::cout << "ERROR: Could not add to cache!" << std::endl;

        // Verify that added key value is in the cache
        ret = cm.contains(newKey);
        if (ret != true)
            std::cout << "\tERROR: Added newKey: returns false, expected true!" << std::endl;





        // Check for existingKey
        do {
            // Generate existingKey
            existingKey = existingDistr(gen);  // (1, testSize)

            ret = cm.contains(existingKey);
            if (ret != true) {
                std::cout << "\tALERT: existingKey already removed! Generating new key..." << std::endl;
            }
            //
            // else {
            //     logToFileAndConsole("SUCCESS:\t\texistingKey found! Removing key " + std::to_string(existingKey) + " ");
            // }
        } while (ret != true);  // Validate that existingKey has not already been removed

        // Call remove on existingKey
        cm.remove(existingKey);

        // Verify that existingKey is no longer in cache
        ret = cm.contains(existingKey);
        if (ret != false)
            std::cout << "\tERROR: Remove existingKey: returns true, expected false!" << std::endl;





        // write out the current values for this iteration
        auto curIterEnd = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

        // Update time stats
        min = std::min(min, elapsed_seconds.count());
        max = std::max(max, elapsed_seconds.count());
        average += elapsed_seconds.count();

        // Convert end timestamp to formatted string
        auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(curIterEnd);
        std::string timeString = std::format("{:%H:%M:%S}", now_sec);

        logToFileAndConsole(std::to_string(i) + "\t\t" + timeString + "\t" + std::to_string(elapsed_seconds.count()));
    }

    // Time clear()
    auto clearTimeStart = std::chrono::system_clock::now();
    cm.clear();
    auto clearTimeEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> clearTime = clearTimeEnd - clearTimeStart;

    // set the end time
    auto finalEnd = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = finalEnd - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(finalEnd);

    std::cout << "\nFinished computation at " << std::ctime(&end_time)
        << "\nNumber of Iterations: " << testIterations
        << "\nElapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    // output the aggregated statistics
    average /= testIterations;
    logToFileAndConsole("\naggregated statistics:");
    logToFileAndConsole("avg\t\t\tmin\t\t\tmax");
    logToFileAndConsole(std::to_string(average) + "\t\t" + std::to_string(min) + "\t\t" + std::to_string(max));
    logToFileAndConsole("\nTime to load cache:\t" + std::to_string(cacheLoadTime.count()));
    logToFileAndConsole("Time to clear:\t\t" + std::to_string(clearTime.count()));

    // print out the cache manager results
    std::cout << "\n\nCache Manager statistics:\n";
    auto benchmarkResults = cm.benchmark();
    bench::printBenchmark(benchmarkResults);

    return;
}


/**
*
* runBenchmark
*
* @brief run the benchmark
*
* @param    config          json configuration
*
* @return                   nothing, but output is sent to console and written to output file
*/
void runBenchmark(json config) {
    // Retrieve configured test cache size and iterations
    int testSize = config["Milestone2"][0]["defaultVariables"][0]["testSize"];
    int testIterations = config["Milestone2"][0]["defaultVariables"][0]["testIterations"];

    logToFileAndConsole("\nProcessing benchmark.");
    logToFileAndConsole("\n\nConfiguration for benchmark run:");
    logToFileAndConsole("\n\ttestSize: " + std::to_string(testSize));
    logToFileAndConsole("\ttestIterations: " + std::to_string(testIterations));

    // call to the timing wrapper (i.e., the actual test)
    timeWrapper(config);

    logToFileAndConsole("\nFinished processing benchmark.");
}

/**
*
* main
*
* main function which does the following:
*   read config file for input file, output file, error file, hash table size and FIFO size
*   create a hash table
*   for each of the test case
*       process test cases - display results to console and write to output file
*       print out the hash table
*       clear out hash table
*
* @param    none
*
* @return   nothing, but output is written to console and files
*/

int main() {
    // Load the config file
    std::ifstream configFile(CONFIG_FILE);
    if (!configFile.is_open()) {
        std::cerr << "Error opening config file!" << std::endl;
        return 1;
    }

    json config;
    configFile >> config;
    configFile.close();

    // Retrieve file paths from the config
    std::string outputFilePath = config["Milestone2"][0]["files"][0]["outputFile"];
    std::string errorFilePath = config["Milestone2"][0]["files"][0]["errorLogFile"];

    // Open up the outfile and set the output file path using the setter
    //
    // Treating output file differently than input and config files
    setOutFile(outputFilePath);
    std::ofstream& outFile = getOutFile();

    // Call the benchmark
    runBenchmark(config);

    logToFileAndConsole("\n\nEnd of tests.");

    // Close output file
    outFile.close();

    return 0;
}
