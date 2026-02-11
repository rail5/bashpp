/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>

/**
 * @class ThreadPool
 * @brief A thread pool implementation that manages a pool of worker threads to execute tasks concurrently.
 *
 * By default, the pool is initialized with a number of available worker threads
 * equal to the number of hardware threads available on the system. Tasks can be enqueued
 * to be executed by the worker threads, which will run them concurrently.
 * 
 */
class ThreadPool {
	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	public:
		ThreadPool(size_t threads = std::thread::hardware_concurrency());
		~ThreadPool();

		/**
		 * @brief Enqueue a new task to be executed by the thread pool.
		 * 
		 * @param task A function pointer to be excecuted by a worker thread.
		 */
		void enqueue(std::function<void()> task);
		void cleanup();
		size_t getThreadCount() const;
};
