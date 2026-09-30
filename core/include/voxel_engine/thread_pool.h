#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace voxeng {

using Task = std::function<void()>;

class ThreadPool {
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop = false;

    void workerLoop();

public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void submit(Task t);
};

}
