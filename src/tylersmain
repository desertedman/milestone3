#include "cache-manager.hpp"
#include "benchmark.hpp"

#include <iostream>
#include <string>

#define TEST_KEY 1
#define TEST_VAL 1
#define TEST_SZ 4096

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



int main(int argc, char **argv) {
	cache::CacheManager<int, int> cm;
	setOutFile("outputFile.txt");


	cm.add(TEST_KEY, TEST_VAL);
	auto opt = cm.getItem(TEST_KEY);
	bool found = false;
	if (opt) {
		std::cout << "v: " << *opt << "\n";
		found = true;
	} else {
		std::cout << "v: FAILED\n";
	}

	if (found != cm.contains(TEST_KEY)) {
		std::cout << "contains: FAILED\n";
	} else {
		std::cout << "contains: SUCCESS\n";
	}

	cm.clear();
	if (cm.contains(TEST_KEY)) {
		std::cout << "clear: FAILED\n";
	} else {
		std::cout << "clear: SUCCESS\n";
	}

	cache::CacheManager<int, std::string, bench::TbbBench> cm2;
	for (auto i = 0; i < TEST_SZ; ++i) {
		std::string s(1, static_cast<char>(i % 256));
		cm2.add(i, s);
		auto opt2 = cm2.getItem(i);
		bool found2 = cm2.contains(i);
	}
	cm2.clear();
	auto bm = cm2.benchmark();
	bench::printBenchmark(bm);

	return 0;
}
