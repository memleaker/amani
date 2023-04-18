#ifndef AMANI_COROUTINE_POOL_H
#define AMANI_COROUTINE_POOL_H

#include <cstdlib>
#include <mutex>
#include <map>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <coroutine>
#include <functional>
#include <iostream>

#include <unistd.h>

class netio_task {
public:
    class promise_type {
    public:
        netio_task get_return_object()
        { return {netio_task(std::coroutine_handle<netio_task::promise_type>::from_promise(*this))}; }
        std::suspend_always initial_suspend() { return {}; }  // always suspend at start
        std::suspend_always  final_suspend() noexcept { return {}; }
		void return_void() {}
        void unhandled_exception() { throw; }

    public:
	int iowait;
	int value;

	int fd;
	int flags;
	int need_block;
    };

public:
    std::coroutine_handle<netio_task::promise_type> handle_;
};

class async_read {
public:
	async_read(int fd, void *buf, size_t len) : m_fd(fd), m_buf(buf), m_len(len) {}

    bool await_ready() { return false; } // always suspend

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().iowait = 1;


		// add fd to epoll
		handle.promise().fd = m_fd;
		// handle.promise().flags = POLLIN;
		handle.promise().need_block = 1;
	}

    ssize_t await_resume()
	{
		for (;;)
		{
			m_nbytes = read(m_fd, m_buf, m_len);
			if (m_nbytes == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
			}

			return m_nbytes;
		}
	}

private:
	int     m_fd;
	void *  m_buf;
	size_t  m_len;
	ssize_t m_nbytes;
};

class netco_pool
{
public:
   	template <typename F, typename... Args>
	void submit(F &&f, Args &&...args)
	{
		// 刚启动时使其挂起, 获取到返回的 handle, 便于控制
		// init_suspend()
		tasks.emplace_back(f(args...));
	}

	void run(void)
	{
		while (!tasks.empty())
		{
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				// debug std::cout << &it->handle_.promise() << std::endl;

				if (it->handle_.done())
				{
					// 结束时需要挂起, 因此需要手动销毁
					// 如果结束时不挂起, 则resume返回后handle就已经destroy(), 下面再使用不合法了
					it->handle_.destroy();
					it = tasks.erase(it);
				}
				else
				{
					it->handle_.resume();

					if (it->handle_.promise().need_block)
					{
						// ep.addfd();
					}
					it++;
				}
			}
		}
	}

private:
	std::vector<netio_task> tasks;
	std::map<int, std::coroutine_handle<netio_task::promise_type>> iowait_handles;
};


#if 0
netio_task test3()
{
	char buf[1024];

	co_await async_read(0, buf, 1024);
}
#endif

#endif