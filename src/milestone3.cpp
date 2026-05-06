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
#include "schedule.h"

using json = nlohmann::json;

#define _CRT_SECURE_NO_WARNINGS
#define CONFIG_FILE "milestone3_config.json"


// Global variable to be used for logging output
std::ofstream _outFile;

cache::CacheManager<int, std::string, bench::TbbBench> cacheManager;
// Singleton to get cacheManager
cache::CacheManager<int, std::string, bench::TbbBench> &getCacheManager() {
  return cacheManager;
}

enum class LOGGING_LEVEL {
  ON,
  OFF,
};

constexpr LOGGING_LEVEL level = LOGGING_LEVEL::OFF;

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
void getItemTest(json config, cache::CacheManager<int, std::string, bench::TbbBench> &cm, const LOGGING_LEVEL level) {
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

void getItemTest2(const json config, cache::CacheManager<int, std::string, bench::TbbBench> &cm, const int testSize) {
  // Setup random key generation
  std::random_device rd{};
  std::mt19937 gen(rd());
  std::uniform_int_distribution newDistr{
      testSize + 1, INT_MAX}; // Generate random key > testSize
  std::uniform_int_distribution existingDistr{
      1, testSize}; // Generate random key 1 - testSize

  int newKey{newDistr(gen)};
  int existingKey{existingDistr(gen)}; // (1, testSize)

  // Try key that is in cache
  auto val{cm.getItem(existingKey)};
  logToFileAndConsole("key: " + std::to_string(existingKey) + ", value: " + *val);

  // Try key not in cache
  val = cm.getItem(newKey);
  logToFileAndConsole("key: " + std::to_string(newKey) + ", value: " + *val);
}

int generateRandomValue(const int min, const int max) {
  std::random_device rd{};
  std::mt19937 gen(rd());
  std::uniform_int_distribution distr{min, max};

  return int{distr(gen)};
}

bool addItemTest(const int key, const int min, const int max) {
  auto &cm = getCacheManager();
  int newKey{};
  bool ret{};

  // Check if cache contains newKey (should not!)
  do {
    // Generate newKey
    newKey = key;

    ret = cm.contains(newKey);
    if (ret != false && level == LOGGING_LEVEL::ON) {
      std::cout << "ALERT:\t\tnewKey already added! Generating new key...\n";
    }
  } while (ret != false); // Validate that newKey has not already been added

  // Add newKey and value to cache
  std::string value = "Test value for key: " + std::to_string(newKey);
  ret = cm.add(newKey, value);
  if (level == LOGGING_LEVEL::ON)
    std::cout << "\t\tAdded: " + std::to_string(newKey) +
                     ", Value: " + *cm.getItem(newKey) + "\n";

  return ret;
}

void benchmarkCacheManager(const json &config, const int testSize, const int threadId, const std::chrono::system_clock::time_point &start, const Ratio &ratios) {
  // auto &cm = getCacheManager(); // Get cm using singleton
  auto schedule = Schedule::buildSchedule(ratios);

  // call the specific function to time
  for (int i = 0; i < 10; i++) {
    float average{0.f}, min{0.f}, max{0.f};

    switch (schedule[i]) {
    case METHOD::GET_ITEM:
      break;
    case METHOD::ADD_ITEM:
      break;
    case METHOD::CONTAINS_ITEM:
      break;
    case METHOD::REMOVE_ITEM:
      break;
    }

    // TODO: Insert sleep here...

    // write out the current values for this iteration
    auto curIterEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = curIterEnd - start;
    // std::time_t iterEndTime =
    // std::chrono::system_clock::to_time_t(curIterEnd);
    std::string timeString = "00:00:00";

    logToFileAndConsole(std::to_string(threadId) + "\t\t" + timeString +
                        "\t" + std::to_string(i) + "\t\t" +
                        std::to_string(average) + "\t" + std::to_string(min) +
                        "\t" + std::to_string(max));
  }
}

void testCacheManager(const json &config) {
  auto &cm = getCacheManager();

  [[maybe_unused]] int testSize =
      config["Milestone3"][0]["defaultVariables"][0]["testSize"];
  for (int key = 10; key < 20; key++) {
    std::string value = "Test value for key: " + std::to_string(key);
    cm.add(key, value);
  }

  for (int key = 0; key < 20; key++) {
    auto val = cm.getItem(key);
    std::cout << "Key: " + std::to_string(key) + ", Value: " + *val + '\n';
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
    const Ratio ratios{
        .getItemRatio{cases["CacheManagerBenchmark"][0]["testCase1"][0]
                           ["ratioOfMethods"][0]["getItem"]},
        .addRatio{cases["CacheManagerBenchmark"][0]["testCase1"][0]
                       ["ratioOfMethods"][0]["add"]},
        .containsRatio{cases["CacheManagerBenchmark"][0]["testCase1"][0]
                            ["ratioOfMethods"][0]["contains"]},
        .removeRatio{cases["CacheManagerBenchmark"][0]["testCase1"][0]
                          ["ratioOfMethods"][0]["remove"]},
        .clearRatio{cases["CacheManagerBenchmark"][0]["testCase1"][0]
                         ["ratioOfMethods"][0]["clear"]}};
    logToFileAndConsole("\tratioOfMethods: ");
    logToFileAndConsole("\t\tgetItem: " + std::to_string(ratios.getItemRatio));
    logToFileAndConsole("\t\tadd: " + std::to_string(ratios.addRatio));
    logToFileAndConsole("\t\tcontains: " + std::to_string(ratios.containsRatio));
    logToFileAndConsole("\t\tremove: " + std::to_string(ratios.removeRatio));
    logToFileAndConsole("\t\tclear: " + std::to_string(ratios.clearRatio));

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
        // logToFileAndConsole("Added key: " + std::to_string(key) + "; value: '" + value + "'");
    }

    // testCacheManager(config);

    // output some helpful comments to the console
    // add the load time calc and output here
    std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
    logToFileAndConsole("threadId\tend time\titer #\t\tavg\t\tmin\t\tmax\t\t");

    // after loading the cache, spawn threads and start the static ratio test as discussed in Canvas
    // use the same output format as in milestone2 (method, timeWrapper, is left in this example).  
    //     Add thread ID as the first column.

    // Dispatch threads to run benchmarks
    const size_t numThreads{config["Milestone3"][0]["defaultVariables"][0]["degreeOfParallelism"]};
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    assert(numThreads == threads.capacity()); // Ensure threads size is properly allocated

    for (size_t i{0}; i < 1; i++) {
        threads.emplace_back(benchmarkCacheManager, config, testSize, i, start, ratios);
    }

    // Wait until all threads are finished
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

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
