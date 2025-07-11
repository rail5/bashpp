/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads)  {
	stop = false;
	for (size_t i = 0; i < threads; i++) {
		workers.emplace_back([this] {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

					if (this->stop && this->tasks.empty()) {
						return; // Exit thread if stop is true and no tasks are left
					}

					// Get the next task
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task(); // Execute the task
			}
		});
	}
}

ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true; // Set stop to true to signal all threads to exit
	}

	condition.notify_all(); // Notify all threads to wake up and check the stop condition

	for (std::thread& worker : workers) {
		if (worker.joinable()) {
			worker.join(); // Wait for all threads to finish
		}
	}
}

void ThreadPool::enqueue(std::function<void()> task)  {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.push(std::move(task)); // Add the task to the queue
	}
	condition.notify_one(); // Notify one thread that a new task is available
}
