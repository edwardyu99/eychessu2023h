#ifndef THREADPOOL_H
#define THREADPOOL_H

//20230122 - chatgpt
#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>
// chatgpt c++11
class ThreadPool
{
public:
    using Task = std::function<void()>;

    explicit ThreadPool(size_t num_threads)
		{
    if (num_threads == 0)
        num_threads = std::thread::hardware_concurrency();
        //printf("std::thread::hardware_concurrency()=%d\n", num_threads );
        //return;
    start(num_threads);
		}

    ~ThreadPool()
    {
        stop();
    }

    template <class T>
    auto submit(T task) -> std::future<decltype(task())>
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));
        {
            
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace([=] {
                (*wrapper)();
            });
        }
        cv_.notify_one();
        return wrapper->get_future();
    }

    template <class T>
    auto submit(std::launch policy, T task) -> std::future<decltype(task())>
    {
        if (policy == std::launch::async)
            return submit(std::move(task));

        std::packaged_task<decltype(task()) ()> wrapper(std::move(task));
        std::future<decltype(task())> res(wrapper.get_future());
        {
            // error std::unique_lock lock{mutex_};
            std::unique_lock<std::mutex> lock(mutex_);

            if (stopped_)
                throw std::runtime_error("submit on stopped ThreadPool");

            tasks_.emplace([&]
            {
                wrapper();
            });
        }
        cv_.notify_one();
        return res;
    }

    void join()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return tasks_.empty(); });
    }

private:
    std::vector<std::thread> threads_;
    std::queue<Task> tasks_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;

    void start(size_t num_threads)
    {
        for (size_t i = 0; i < num_threads; ++i)
            threads_.emplace_back([&] {
                while (true)
                {
                    Task task;
                    {
                        // error std::unique_lock lock{mutex_};
                        std::unique_lock<std::mutex> lock(mutex_);

                        cv_.wait(lock, [&] { return stopped_ || !tasks_.empty(); });
                        if (stopped_ && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
    }

    void stop() noexcept
    {
        {
            // error std::unique_lock lock{mutex_};
            std::unique_lock<std::mutex> lock(mutex_);

            stopped_ = true;
        }
        cv_.notify_all();
        for (auto &thread : threads_)
            thread.join();
    }
};
// end of chatgpt
//extern ThreadPool thread_pool;

#endif