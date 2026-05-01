/**
 * @file benchmark.hpp
 *
 * @author agge3, kpowkitty
 * @version 1.0
 * @since 2025-11-04
 *
 * Benchmarking utilities for CacheManager.
 */

#pragma once

#include "macros.hpp"
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

#define NDEBUG 1

namespace bench {

/**
 * Metrics produced by the cache manager.
 */
struct Benchmark {
	size_t hits = 0; /* amount of successful hits on the cache */
	size_t misses = 0; /* amount of failed hits on the cache */
	size_t evictions = 0; /* amount of evictions in the cache */
	float hit_ratio = 0; /* percentage of hits on the cache vs misses */
	float calc_hit_ratio() const {
		const float total = static_cast<float>(hits) + misses;
		return total == 0.0f ? 0.0f : static_cast<float>(hits) / total;
	};
};

/**
 * Prints the current benchmark metrics to std::out.
 */
inline void printBenchmark(const Benchmark &bm) {
	std::cout << "hits:\t\t" << bm.hits << "\n"
			  << "misses:\t\t" << bm.misses << "\n"
			  << "evictions:\t" << bm.evictions << "\n"
			  << "hit ratio:\t" << bm.hit_ratio << "\n";
}

/**
 * Writes benchmark to JSON file. Appends to it.
 */
inline void writeBenchmark(const Benchmark &bm) {
	std::string filename = "benchmark.jsonl";

	int run_number = 1;
	if (std::filesystem::exists(filename)) {
		std::ifstream infile(filename);
		std::string line;
		while (std::getline(infile, line)) {
			if (!line.empty()) {
				run_number++;
			}
		}
		infile.close();
	}

	nlohmann::json bench;

	bench["run"] = run_number;
	bench["hits"] = bm.hits;
	bench["misses"] = bm.misses;
	bench["evictions"] = bm.evictions;
	bench["hit_ratio"] = bm.hit_ratio;

	std::ofstream file(filename, std::ios::app);
	if (file.is_open()) {
		file << bench.dump() << "\n";
		file.close();
	}
}

/**
 * No-op bench for benchless runs.
 */
struct NoneBench {
	static inline void hit() {}
	static inline void miss() {}
	static inline void eviction() {}
	static Benchmark aggregate() { return {}; }
};

/**
 * Threaded benchmark.
 */
struct ThreadBench {
	/* mutex for adding to the registry */
	static inline std::mutex registry_mutex;
	
	/* current threads running */
	static inline std::vector<Benchmark *> registry;
	
	/* benchmark per thread */
	static inline thread_local Benchmark local_bench;

	/**
	 * Registers thread to the registry.
	 */
	static inline void register_thread() {
		[[maybe_unused]] static thread_local bool registered = [] {
			std::lock_guard<std::mutex> g(registry_mutex);
			registry.push_back(&local_bench);
			return true;
		}();
	}

	/**
	 * Logs if we had a hit.
	 */
	static inline void hit() {
		register_thread();
		++local_bench.hits;
	}

	/**
	 * Logs if we had a miss.
	 */
	static inline void miss() {
		register_thread();
		++local_bench.misses;
	}

	/**
	 * Logs if we had an eviction.
	 */
	static inline void eviction() {
		register_thread();
		++local_bench.evictions;
	}

	/**
	 * Accumulates all metrics for the given thread.
	 *
	 * @return The combined benchmark metric.
	 */
	static inline Benchmark aggregate() {
		std::lock_guard<std::mutex> g(registry_mutex);
		Benchmark bm{};
		for (auto &p : registry) {
			bm.hits += p->hits;
			bm.misses += p->misses;
			bm.evictions += p->evictions;
		}
		bm.hit_ratio = bm.calc_hit_ratio();
		return bm;
	}
};

/**
 * Tbb based benchmark.
 */
struct TbbBench {
	/* container for local benchmark metrics */
	static inline tbb::enumerable_thread_specific<Benchmark> ets;

	/* logs if we had a hit */
	static inline void hit() { ++ets.local().hits; }
	/* logs if we had a miss */
	static inline void miss() { ++ets.local().misses; }
	/* logs if we had an eviction */
	static inline void eviction() { ++ets.local().evictions; }

	/**
	 * Accumulates all benchmark metrics of the current thread.
	 */
	static Benchmark aggregate() {
		Benchmark bm{};
		for (auto &t : ets) {
			bm.hits += t.hits;
			bm.misses += t.misses;
			bm.evictions += t.evictions;
		}
		bm.hit_ratio = bm.calc_hit_ratio();
		return bm;
	}
};
} // namespace bench
