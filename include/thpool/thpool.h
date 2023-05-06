#ifndef AMANI_THREAD_POOL_H
#define AMANI_THREAD_POOL_H

#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>

template <typename T>
class task_queue
{
private:
    std::queue<T> m_queue;
    std::mutex    m_mutex;
public:
    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    int size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
    void enqueue(const T &t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }
    bool dequeue(T &t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;

        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
};
class thread_pool
{
private:
    class thread_worker
    {
    private:
        thread_pool *m_pool;
    public:
        thread_worker(thread_pool *pool) : m_pool(pool) {}

        void operator()()
        {
            bool dequeued;
            std::function<void()> func;
            while (!m_pool->m_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    if (m_pool->m_queue.empty())
                    {
                        m_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_pool->m_queue.dequeue(func);
                }

                if (dequeued)
                    func();
            }
        }
    };

public:
    thread_pool(const int n_threads = 4)
        : m_shutdown(false), m_threads(std::vector<std::thread>(n_threads)) {}

    // no copy
    thread_pool(const thread_pool &) = delete;
    thread_pool(thread_pool &&) = delete;
    thread_pool &operator=(const thread_pool &) = delete;
    thread_pool &operator=(thread_pool &&) = delete;

    void init(void)
    {
        for (auto& th : m_threads)
        {
            th = std::thread(thread_worker(this));
        }
    }

    void shutdown(void)
    {
        m_shutdown = true;
        m_conditional_lock.notify_all();

        for (auto &th : m_threads)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> warpper_func = [task_ptr]()
	{
		(*task_ptr)();
	};

        m_queue.enqueue(warpper_func);
        m_conditional_lock.notify_one();

        return task_ptr->get_future();
    }

private:
    bool m_shutdown;
    task_queue<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_conditional_mutex;
    std::condition_variable m_conditional_lock;
};
#endif




/* TEST
#include <iostream>
#include <chrono>
#include <thread>

#include "th.h"

int test1(void)
{
	int i;

    for (i = 1; i < 20; i++)
        std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "test1 return" << std::endl;

	return i;
}

int test2(void)
{
	int i;

    for (i = 1; i < 20; i+=2)
        std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "test2 return" << std::endl;

	return i;
}

int main(void)
{
    thread_pool pool(2);

	pool.init();

	auto t1 = pool.submit(test1);
	auto t2 = pool.submit(test2);
	
	int t1ret = t1.get();
	int t2ret = t2.get();

	std::cout << "t1 return " << t1ret << std::endl;
	std::cout << "t2 return " << t2ret << std::endl;

	pool.shutdown();

    return 0;
}
*/