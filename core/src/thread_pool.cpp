#include "voxel_engine/thread_pool.h"

namespace voxeng {

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; i++) {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_all();
    for (auto& worker : m_workers) {
        worker.join();
    }
}

void ThreadPool::submit(Task t) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(t));
    }
    m_cv.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return m_stop || !m_tasks.empty(); });

            if (m_stop && m_tasks.empty()) {
                return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
    }
}

} // namespace voxeng
