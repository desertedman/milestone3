/**
*
* milestone3.cpp : This file contains the 'main' function. Program execution begins and ends there.
*
* 09/23/24 - Created by ChatGPT with prompt "write C++ program reads and parses the file: milestone3_config.json"
*            The file: "milestone3_config.json" is in the following format:
*
                {
                    "Milestone3": [
                        {
                            "files": [
                                {
                                    "testcaseFile": "milestone3.json",
                                    "outputFile": "generatedOutputFile.txt",
                                    "errorLogFile":"logFile.txt"
                                }
                            ],
                            "defaultVariables": [
                                {
                                    "testSize": 10,
                                    "testIterations": 10,
                                    "degreeOfParallelism": 100,
                                    "sleepInterval": 1
                                }
                            ]
                        }
                    ]
                }
            and where the testcaseFile has the following format:
                {
                    "CacheManagerBenchmark": [
                        {
                            "testCase1": [
                                {
                                    "ratioOfMethods": [
                                        {
                                            "getItem": 80,
                                            "add": 9,
                                            "contains": 2,
                                            "remove": 9,
                                            "clear": 1
                                         }
                                     ]
                                 }
                             ]
                         }
                     ]
                 }

9/15/2025 - create by Joseph Hui;

*/
#include "cache-manager.hpp"
#include "benchmark.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <thread>

#include "json.hpp"

using json = nlohmann::json;

#define _CRT_SECURE_NO_WARNINGS
#define CONFIG_FILE "milestone3_config.json"


// Global variable to be used for logging output
std::ofstream _outFile;

cache::CacheManager<int, std::string, bench::TbbBench> cacheManager;
cache::CacheManager<int, std::string, bench::TbbBench> &getCacheManager() {
  return cacheManager;
}

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
    // sample code to retrieve and print a found entry
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution distr(1, 10);
    int testKey = distr(gen);
    auto val = cm.getItem(testKey);
    logToFileAndConsole("\n\nRetrieved key: " + std::to_string(testKey) + "; with value: " + *val);

    // try one that is not in the cache
    testKey = testKey * 1000;
    val = cm.getItem(testKey);
    if (!val) {
       logToFileAndConsole("Key: " + std::to_string(testKey) + " not found (expected)!");
    } else {
       logToFileAndConsole("Retrieved key: " + std::to_string(testKey) + "; with value: " + *val);
    }
}

void benchmarkCacheManager(const json &config, const int testSize, const int threadId, const std::chrono::system_clock::time_point &start) {
  // std::cout << "threadId " + std::to_string(threadId) + " running...\n";

  // need to write out the data for each timed iteration in the following format:
  // 
  // threadId    end time    iter#   avg     min     max     
  // 
  // <threadId1> <time1>         1   1.2     0.9     1.4
  // <threadId2> <time2>         2   1.1     0.7     1.2
  // ...

  auto &cm = getCacheManager(); // Get cm using singleton

  // Declare keys; put off generation til later
  int existingKey{}, newKey{};
  bool ret{};

  // Setup random key generation
  std::random_device rd{};
  std::mt19937 gen(rd());
  std::uniform_int_distribution newDistr{
      testSize + 1, INT_MAX}; // Generate random key > testSize
  std::uniform_int_distribution existingDistr{
      1, testSize}; // Generate random key 1 - testSize

  // call the specific function to time
  for (int i = 0; i < 10; i++) {
    float average = 0.0;
    float min = 0.0;
    float max = 0.0;

    // add more functions here
    getItemTest(config, cm);

    // Check if cache contains newKey (should not!)
    do {
      // Generate newKey
      newKey = newDistr(gen); // (testSize + 1, INT_MAX)

      ret = cm.contains(newKey);
      if (ret != false) {
        std::cout << "ALERT:\t\tnewKey already added! Generating new key...";
      }
    } while (ret != false); // Validate that newKey has not already been added

    // Add newKey and value to cache
    std::string value = "Test value for key: " + std::to_string(newKey);
    ret = cm.add(newKey, value);

    // Cache is full alert
    // if (ret != true)
    //     std::cout << "ERROR: Could not add to cache!" << std::endl;

    // Verify that added key value is in the cache
    ret = cm.contains(newKey);
    // TODO: Replace with assert
    if (ret != true)
      std::cout << "\tERROR: Added newKey: returns false, expected true!"
                << std::endl;

    // Check for existingKey
    do {
      // Generate existingKey
      existingKey = existingDistr(gen); // (1, testSize)

      ret = cm.contains(existingKey);
      if (ret != true) {
        std::cout
            << "\tALERT: existingKey already removed! Generating new key..."
            << std::endl;
      }
    } while (ret !=
             true); // Validate that existingKey has not already been removed

    // Call remove on existingKey
    cm.remove(existingKey);

    // Verify that existingKey is no longer in cache
    ret = cm.contains(existingKey);
    // TODO: Replace with assert
    if (ret != false)
      std::cout << "\tERROR: Remove existingKey: returns true, expected false!"
                << std::endl;

    // write out the current values for this iteration
    auto curIterEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = curIterEnd - start;
    // std::time_t iterEndTime =
    // std::chrono::system_clock::to_time_t(curIterEnd);
    std::string timeString = "00:00:00";

    logToFileAndConsole("\t" + std::to_string(threadId) + "\t" + timeString +
                        "\t" + std::to_string(i) + "\t\t" +
                        std::to_string(average) + "\t" + std::to_string(min) +
                        "\t" + std::to_string(max));
  }
}

/**
*
* timeWrapper
*
* @brief Simple wrapper function that times a limited number of functions (hardcoded for now)
*
* @param    config              benchmark config
*
* @return   nothing, but output is sent to console and written to output file
*/
void timeWrapper(json config) {
    // set the start time
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // Retrieve file paths from the config
    std::string testcaseFilePath = config["Milestone3"][0]["files"][0]["testcaseFile"];
    std::ifstream testcaseFile(testcaseFilePath);
    if (!testcaseFile.is_open()) {
        std::cerr << "Error opening testcase file!" << std::endl;
        return;
    }

    json cases;
    testcaseFile >> cases;

    logToFileAndConsole("\nConfiguration for testCase1:\n");

    // Retrieve the method ratios
    int getItemRatio = cases["CacheManagerBenchmark"][0]["testCase1"][0]["ratioOfMethods"][0]["getItem"];
    int addRatio = cases["CacheManagerBenchmark"][0]["testCase1"][0]["ratioOfMethods"][0]["add"];
    int containsRatio = cases["CacheManagerBenchmark"][0]["testCase1"][0]["ratioOfMethods"][0]["contains"];
    int removeRatio = cases["CacheManagerBenchmark"][0]["testCase1"][0]["ratioOfMethods"][0]["remove"];
    int clearRatio = cases["CacheManagerBenchmark"][0]["testCase1"][0]["ratioOfMethods"][0]["clear"];
    logToFileAndConsole("\tratioOfMethods: ");
    logToFileAndConsole("\t\tgetItem: " + std::to_string(getItemRatio));
    logToFileAndConsole("\t\tadd: " + std::to_string(addRatio));
    logToFileAndConsole("\t\tcontains: " + std::to_string(containsRatio));
    logToFileAndConsole("\t\tremove: " + std::to_string(removeRatio));
    logToFileAndConsole("\t\tclear: " + std::to_string(clearRatio));

    // output some helpful comments to the console
    std::cout << "\nStarting computation at " << std::ctime(&start_time);

    // need to write out the data for each timed iteration in the following format:
    // 
    // threadId    end time    iter#   avg     min     max     
    // 
    // <threadId1> <time1>         1   1.2     0.9     1.4
    // <threadId2> <time2>         2   1.1     0.7     1.2
    // ...

    int testSize = config["Milestone3"][0]["defaultVariables"][0]["testSize"];
    
    // Allocate the cache manager
    auto &cm = getCacheManager(); // Get cm using singleton

    // sample test load of the cache
    for (auto key = 0; key <= testSize; ++key) {
        std::string value = "Test value for key: " + std::to_string(key);
        cm.add(key, value);
        logToFileAndConsole("Added key: " + std::to_string(key) + "; value: '" + value + "'");
    }


    // output some helpful comments to the console
    // add the load time calc and output here
    std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
    logToFileAndConsole("threadId\tend time\titer #\t\tavg\t\tmin\t\tmax\t\t");

    // after loading the cache, spawn threads and start the static ratio test as discussed in Canvas
    // use the same output format as in milestone2 (method, timeWrapper, is left in this example).  
    //     Add thread ID as the first column.

    benchmarkCacheManager(config, testSize, 0, start);
    // // Dispatch threads to run benchmarks
    // const size_t numThreads{config["Milestone3"][0]["defaultVariables"][0]["degreeOfParallelism"]};
    // std::vector<std::thread> threads;
    // threads.reserve(numThreads);
    // assert(numThreads == threads.capacity()); // Ensure threads size is properly allocated
    //
    // for (size_t i{0}; i < 1; i++) {
    //     threads.emplace_back(benchmarkCacheManager, config, testSize, i, start);
    // }
    //
    // // Wait until all threads are finished
    // for (auto &t : threads) {
    //     if (t.joinable()) {
    //         t.join();
    //     }
    // }

    logToFileAndConsole("\n\n");

    // set the end time
    auto finalEnd = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = finalEnd - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(finalEnd);

    std::cout << "Finished computation at " << std::ctime(&end_time)
        << "Elapsed time: " << elapsed_seconds.count() << "s"
        << std::endl << std::endl;

    // print out the cache manager results
    auto benchmarkResults = cm.benchmark();
    bench::printBenchmark(benchmarkResults);

    return;
}


/**
*
* staticRatio
*
* @brief run the benchmark with static method ratios
*
* @param    config          json configuration
*
* @return                   nothing, but output is sent to console and written to output file
*/
void staticRatio(json config) {
    logToFileAndConsole("\nProcessing staticRatio benchmark.\n\n");
    logToFileAndConsole("Configuration for benchmark run:\n");

    // Retrieve configured Test Type
    std::string testType = config["Milestone3"][0]["defaultVariables"][0]["testType"];
    logToFileAndConsole("\ttestType: " + testType);

    // Retrieve configured Test Size
    int testSize = config["Milestone3"][0]["defaultVariables"][0]["testSize"];
    logToFileAndConsole("\ttestSize: " + std::to_string(testSize));

    // Retrieve configured Test Iterations
    int testIterations = config["Milestone3"][0]["defaultVariables"][0]["testIterations"];
    logToFileAndConsole("\ttestIterations: " + std::to_string(testIterations));

    // Retrieve configured Degree of Parallelism
    int degreeOfParallelism = config["Milestone3"][0]["defaultVariables"][0]["degreeOfParallelism"];
    logToFileAndConsole("\tdegreeOfParallelism: " + std::to_string(degreeOfParallelism));

    // Retrieve configured Sleep Interval
    int sleepInterval = config["Milestone3"][0]["defaultVariables"][0]["sleepInterval"];
    logToFileAndConsole("\tsleepInterval: " + std::to_string(sleepInterval));

    // call to the timing wrapper (i.e., call the actual benchmark)
    timeWrapper(config);

    logToFileAndConsole("\nFinished processing benchmark.\n\n");
}


/**
*
* benchmarkWrapper
*
* @brief main wrapper method for the benchmark
*
* @param    config          json configuration
*
* @return                   nothing, but output is sent to console and written to output file
*/
void benchmarkWrapper(json config) {
    // Retrieve configured Test Type
    std::string testType = config["Milestone3"][0]["defaultVariables"][0]["testType"];

    if (testType == "static") {
        staticRatio(config);
    }
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

    // Retrieve file paths from the config
    std::string outputFilePath = config["Milestone3"][0]["files"][0]["outputFile"];
    std::string errorFilePath = config["Milestone3"][0]["files"][0]["errorLogFile"];

    // Open up the outfile and set the output file path using the setter
    //
    // Treating output file differently than input and config files because it's used in other files
    setOutFile(outputFilePath);

    // Get the output file
    std::ofstream& outFile = getOutFile();

    // Call the benchmark wrapper
    benchmarkWrapper(config);

    logToFileAndConsole("\n\nEnd of tests");

    // Close files
    configFile.close();
    outFile.close();

    return 0;
}
