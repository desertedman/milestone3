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

#include "json.hpp"
#include "schedule.h"
#include <atomic>
#include <chrono>
#include <future>
#include <random>
#include <string>

using json = nlohmann::json;

#define _CRT_SECURE_NO_WARNINGS
#define CONFIG_FILE "milestone3_config.json"


// Global variable to be used for logging output
std::ofstream _outFile;

std::mutex printMutex;
std::mutex mutex;
std::atomic_int counter{0};
std::atomic<double> atomicAvg{0}, atomicMin{300.}, atomicMax{0};

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

struct Stats {
  Stats() : min{300.}, max{0.}, avg{0.}, numCalls{0} {}
  double min; // Initialize min to arbitrary large number
  double max;
  double avg;
  int numCalls;
};

struct MethodStats {
  static void updateStats(Stats &stats,
                          const std::chrono::duration<double> &seconds) {
    // Update stats
    stats.min = std::min(stats.min, seconds.count());
    stats.max = std::max(stats.max, seconds.count());
    stats.avg += seconds.count();
    stats.numCalls++;
  }

  static void calculateStats(Stats &finalStats, const Stats &stats) {
    finalStats.min = std::min(finalStats.min, stats.min);
    finalStats.max = std::max(finalStats.max, stats.max);
    finalStats.avg += stats.avg;
    finalStats.numCalls += stats.numCalls;
  }

  static void calculateAverages(Stats &stats) {
    // Safeguard against division by 0
    if (stats.numCalls != 0)
      stats.avg /= stats.numCalls;

    else {
      stats.avg = 0;
      stats.min = 0; // Set min to 0 because it is initialized as 300
    }
  }

  Stats getItemStats;
  Stats addStats;
  Stats containsStats;
  Stats removeStats;
};

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
    printMutex.lock();
    // Get the output file
    std::ofstream& outFile = getOutFile();

    std::cout << message << std::endl;  // Print to console 
    outFile << message << std::endl;  // Write to file
    printMutex.unlock();
}

int generateRandomValue(const int min, const int max) {
  std::random_device rd{};
  std::mt19937 gen(rd());
  std::uniform_int_distribution distr{min, max};

  return int{distr(gen)};
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
void getItemTest(json config, const int testSize, Stats &stats, const int shouldLock = 0) {
  auto curIterStart = std::chrono::system_clock::now();

  auto &cm = getCacheManager();
  int testKey = generateRandomValue(1, testSize);

  // if (generateRandomValue(1, 2) == 1) {
  //   // Generate new key
  //   testKey *= 1000;
  // }

  if (shouldLock)
    mutex.lock();
  auto val = cm.getItem(testKey);
  if (level == LOGGING_LEVEL::ON) {
    if (val)
      logToFileAndConsole("\n\nRetrieved key: " + std::to_string(testKey) +
                          "; with value: " + *val);

    else
      logToFileAndConsole("\n\nKey: " + std::to_string(testKey) + " not found");
  }
  if (shouldLock)
    mutex.unlock();

  auto curIterEnd = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

  MethodStats::updateStats(stats, elapsed_seconds);
}

bool addItemTest(const int testSize, Stats &stats, const int shouldLock = 0) {
  auto curIterStart = std::chrono::system_clock::now();

  auto &cm = getCacheManager();
  bool ret{};
  int testKey{generateRandomValue(testSize + 1, INT_MAX)};

  std::string value = "Test value for key: " + std::to_string(testKey);
  if (shouldLock)
    mutex.lock();
  ret = cm.add(testKey, value);
  assert(ret == true); // Ensure adds are successful
  if (level == LOGGING_LEVEL::ON)
    std::cout << "\t\tAdded: " + std::to_string(testKey) +
                     ", Value: " + *cm.getItem(testKey) + "\n";
  if (shouldLock)
    mutex.unlock();

  auto curIterEnd = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

  MethodStats::updateStats(stats, elapsed_seconds);

  return ret;
}

bool containsItemTest(const int testSize, Stats &stats, const int shouldLock = 0) {
  auto curIterStart = std::chrono::system_clock::now();

  auto &cm = getCacheManager();
  bool ret{};
  int testKey{generateRandomValue(1, testSize)};

  if (generateRandomValue(1, 2) == 1) {
    // Generate new key
    testKey *= 1000;
  }

  if (shouldLock)
    mutex.lock();
  ret = cm.contains(testKey);
  if (shouldLock)
    mutex.unlock();

  auto curIterEnd = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

  MethodStats::updateStats(stats, elapsed_seconds);

  return ret;
}

bool removeItemTest(const int testSize, Stats &stats, const int shouldLock = 0) {
  auto curIterStart = std::chrono::system_clock::now();

  auto &cm = getCacheManager();
  bool ret{};
  int testKey{generateRandomValue(1, testSize)};

  // if (generateRandomValue(1, 2) == 1) {
  //   // Generate new key
  //   testKey *= 1000;
  // }

  if (shouldLock)
    mutex.lock();
  ret = cm.remove(testKey);
  if (shouldLock)
    mutex.unlock();

  auto curIterEnd = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

  MethodStats::updateStats(stats, elapsed_seconds);

  return ret;
}

MethodStats benchmarkCacheManager(const json &config, const int threadId,
                                  const Ratio &ratios, const std::string &mode) {
  MethodStats methodStats;

  const double sleepIntervalMs =
      static_cast<double>(
          config["Milestone3"][0][mode][0]["sleepInterval"]) *
      1000.f;
  const int testSize =
      config["Milestone3"][0][mode][0]["testSize"];
  [[maybe_unused]] const int degreeOfParallelism =
      config["Milestone3"][0][mode][0]["degreeOfParallelism"];
  const int testIterations =
      config["Milestone3"][0][mode][0]["testIterations"];
  const int shouldLock = config["Milestone3"][0]["overwriteVariables"][0]["shouldLock"];

  auto schedule = Schedule::buildSchedule(ratios, testIterations);
  // std::cout << "Schedule size: " + std::to_string(schedule.size()) + "\n";

  [[maybe_unused]] double average{0}, min{300}, max{0};
  // call the specific function to time
  while (true) {
    // Make loop thread safe
    // i starts at 0
    int i = counter.fetch_add(1);
    if (i >= testIterations)
      break;

    auto curIterStart = std::chrono::system_clock::now();

    switch (schedule.at(i)) {
    case METHOD::GET_ITEM:
      getItemTest(config, testSize, methodStats.getItemStats, shouldLock);
      break;
    case METHOD::ADD_ITEM:
      addItemTest(testSize, methodStats.addStats, shouldLock);
      break;
    case METHOD::CONTAINS_ITEM:
      containsItemTest(testSize, methodStats.containsStats, shouldLock);
      break;
    case METHOD::REMOVE_ITEM:
      removeItemTest(testSize, methodStats.removeStats, shouldLock);
      break;
    default:
      break;
    }

    // need to write out the data for each timed iteration in the following
    // format:
    //
    // threadId    end time    iter#   avg     min     max
    //
    // <threadId1> <time1>         1   1.2     0.9     1.4
    // <threadId2> <time2>         2   1.1     0.7     1.2
    // ...

    // write out the current values for this iteration
    auto curIterEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = curIterEnd - curIterStart;

    std::string timeString = std::format(
        "{:%H:%M:%S}",
        std::chrono::time_point_cast<std::chrono::seconds>(curIterEnd));

    atomicMin = std::min(static_cast<double>(atomicMin), elapsed_seconds.count());
    atomicMax = std::max(static_cast<double>(atomicMax), elapsed_seconds.count());
    atomicAvg += elapsed_seconds.count();

    // Mutex lock for consistent printing
    logToFileAndConsole(std::to_string(threadId) + "\t\t" + timeString + "\t" +
                        std::to_string(i + 1) + "\t\t" +
                        std::to_string(atomicAvg / (i + 1)) + "\t" +
                        std::to_string(atomicMin) + "\t" + std::to_string(atomicMax));

    std::this_thread::sleep_for(
        std::chrono::duration<double, std::milli>{sleepIntervalMs});
  }

  return methodStats;
}

void testCacheManager(const json &config, const std::string &mode) {
  auto &cm = getCacheManager();

  [[maybe_unused]] int testSize =
      config["Milestone3"][0][mode][0]["testSize"];
  for (int key = 10; key < 20; key++) {
    std::string value = "Test value for key: " + std::to_string(key);
    cm.add(key, value);
  }

  for (int key = 0; key < 20; key++) {
    auto val = cm.getItem(key);
    std::cout << "Key: " + std::to_string(key) + ", Value: " + *val + '\n';
  }
}

void printStats(std::string method, const Stats &stats) {
  logToFileAndConsole(method + "\t\t" + std::to_string(stats.avg) + "\t" +
                      std::to_string(stats.min) + "\t" +
                      std::to_string(stats.max) + "\t" +
                      std::to_string(stats.numCalls));
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
void timeWrapper(json config, const std::string &mode) {
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

    [[maybe_unused]] int testSize = config["Milestone3"][0][mode][0]["testSize"];
    
    // Allocate the cache manager
    auto &cm = getCacheManager(); // Get cm using singleton

    // sample test load of the cache
    // (1, testSize) inclusive
    auto loadCacheStart = std::chrono::system_clock::now();
    for (auto key = 1; key <= testSize; ++key) {
      std::string value = "Test value for key: " + std::to_string(key);
      cm.add(key, value);
      // logToFileAndConsole("Added key: " + std::to_string(key) + "; value:
      // '" + value + "'");
    }
    std::chrono::duration<double> loadCacheTime =
        std::chrono::system_clock::now() - loadCacheStart;

    // write out the head line for file
    logToFileAndConsole("threadId\tend time\titer #\t\tavg\t\tmin\t\tmax\t\t");

    // after loading the cache, spawn threads and start the static ratio test as discussed in Canvas
    // use the same output format as in milestone2 (method, timeWrapper, is left in this example).  
    //     Add thread ID as the first column.

    // Dispatch threads to run benchmarks
    const size_t numThreads{config["Milestone3"][0][mode][0]["degreeOfParallelism"]};
    std::vector<std::future<MethodStats>> threads;
    threads.reserve(numThreads);
    assert(numThreads == threads.capacity()); // Ensure threads size is properly allocated

    // Initialize vector of MethodStats
    std::vector<MethodStats> methodStatsVector;
    methodStatsVector.reserve(numThreads);
    assert(numThreads == methodStatsVector.capacity()); // Ensure methodStatsVector size is properly allocated

    for (size_t i{0}; i < numThreads; i++) {
        threads.emplace_back(std::async(benchmarkCacheManager, config, i + 1, ratios, mode));
    }

    // Wait until all threads are finished
    for (auto &t : threads) {
        methodStatsVector.emplace_back(t.get());
    }

    // Clear test
    auto clearStart = std::chrono::system_clock::now();
    cm.clear();
    std::chrono::duration<double> clearTime = std::chrono::system_clock::now() - clearStart;

    // Initialize finalStats and aggregate final stats
    MethodStats finalStats;

    for (const auto &m : methodStatsVector) {
      MethodStats::calculateStats(finalStats.getItemStats, m.getItemStats);
      MethodStats::calculateStats(finalStats.addStats, m.addStats);
      MethodStats::calculateStats(finalStats.containsStats, m.containsStats);
      MethodStats::calculateStats(finalStats.removeStats, m.removeStats);
    }

    MethodStats::calculateAverages(finalStats.getItemStats);
    MethodStats::calculateAverages(finalStats.addStats);
    MethodStats::calculateAverages(finalStats.containsStats);
    MethodStats::calculateAverages(finalStats.removeStats);

    // Print aggregated stats
    logToFileAndConsole("\n\n");
    logToFileAndConsole("Method\t\tavg\t\tmin\t\tmax\t\tnumCalls");
    printStats("getItem", finalStats.getItemStats);
    printStats("addItem", finalStats.addStats);
    printStats("contain", finalStats.containsStats);
    printStats("remove", finalStats.removeStats);
    logToFileAndConsole("clear\t\t" + std::to_string(clearTime.count()));
    logToFileAndConsole("loadCache\t\t" + std::to_string(loadCacheTime.count()));
    auto totalCalls = [finalStats]() {
      return finalStats.getItemStats.numCalls + finalStats.addStats.numCalls +
             finalStats.containsStats.numCalls +
             finalStats.removeStats.numCalls;
    };
    logToFileAndConsole("Total calls: " + std::to_string(totalCalls()));

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
    timeWrapper(config, "defaultVariables");

    logToFileAndConsole("\nFinished processing benchmark.\n\n");
}

void dynamicRatio(json config) {
    logToFileAndConsole("\nProcessing dynamicRatio benchmark.\n\n");
    logToFileAndConsole("Configuration for benchmark run:\n");

    // Retrieve configured Test Type
    std::string testType = config["Milestone3"][0]["overwriteVariables"][0]["testType"];
    logToFileAndConsole("\ttestType: " + testType);

    // Retrieve configured Test Size
    int testSize = config["Milestone3"][0]["overwriteVariables"][0]["testSize"];
    logToFileAndConsole("\ttestSize: " + std::to_string(testSize));

    // Retrieve configured Test Iterations
    int testIterations = config["Milestone3"][0]["overwriteVariables"][0]["testIterations"];
    logToFileAndConsole("\ttestIterations: " + std::to_string(testIterations));

    // Retrieve configured Degree of Parallelism
    int degreeOfParallelism = config["Milestone3"][0]["overwriteVariables"][0]["degreeOfParallelism"];
    logToFileAndConsole("\tdegreeOfParallelism: " + std::to_string(degreeOfParallelism));

    // Retrieve configured Sleep Interval
    int sleepInterval = config["Milestone3"][0]["overwriteVariables"][0]["sleepInterval"];
    logToFileAndConsole("\tsleepInterval: " + std::to_string(sleepInterval));

    // call to the timing wrapper (i.e., call the actual benchmark)
    timeWrapper(config, "overwriteVariables");

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
    std::string testType = config["Milestone3"][0]["overwriteVariables"][0]["testType"];

    if (testType == "static") {
        staticRatio(config);
    }

    else if (testType == "dynamic")
        dynamicRatio(config);
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
