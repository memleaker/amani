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

#include "thpool/thpool.h"
#include "epoller.h"

#define MAX_EVENTS 1024

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
		int fd;
		int need_block;
		uint32_t flags;
    };

public:
    std::coroutine_handle<netio_task::promise_type> handle_;
};

class netco_pool
{
public:
	netco_pool(unsigned int threadnum) : 
		terminated(true), threads(threadnum), thpool(threadnum), \
		eps(std::vector<epoller>(threadnum)), \
		task_queues(std::vector<task_queue<netio_task>>(threadnum)) {}

	void init()
	{
		thpool.init();

		for (auto &ep : eps)
		{
			ep.init();
		}
	}

   	template <typename F, typename... Args>
	void submit(F &&f, Args &&...args)
	{
		static unsigned int thid = 0;

		// 刚启动时即使其挂起, 获取到返回的 handle, 便于控制
		task_queues[thid++ % threads].enqueue(f(args...));
	}

	void run()
	{
		/* running */
		terminated = false;

		// submit task co_run
		for (unsigned int i = 0; i < threads; i++)
		{
			thpool.submit([i, this] {co_run(task_queues[i], eps[i]);});
		}
	}

	void shutdown()
	{
		terminated = true;

		thpool.shutdown();

		// destroy epoll
	}

	void co_run(task_queue<netio_task>& task_que, epoller& ep)
	{
		int events, i;
		netio_task task;
		epoll_event evs[MAX_EVENTS];

		std::cout << "co run" << std::endl;

		while (!terminated)
		{
			/* check io */
			events = ep.wait(evs, MAX_EVENTS);
			for (i = 0; i < events; i++)
			{
				ep.del_fd(evs[i].data.fd);
				// resume schedule
				task_que.enqueue(iowait_tasks[evs[i].data.fd]);
			}

			/* get task */
			if (!task_que.dequeue(task))
			{
				usleep(10000);
				continue;
			}

			/* handle task */
			if (task.handle_.done())
			{
				// 结束时需要挂起, 因此需要手动销毁
				// 如果结束时不挂起, 则resume返回后handle就已经destroy(), 下面再使用不合法了
				task.handle_.destroy();
			}
			else
			{
				// resume
				task.handle_.resume();

				if (task.handle_.promise().need_block)
				{
					// add fd to epoll
					std::cout << "add " << task.handle_.promise().fd << "to epoll" << std::endl;
					ep.add_fd(task.handle_.promise().fd, task.handle_.promise().flags);

					iowait_tasks[task.handle_.promise().fd] = task;
				}
				else
				{
					// requeue task to continue schedle
					task_que.enqueue(task);
				}
			}
		}
	}

private:
	bool terminated;

	unsigned int threads;
	thread_pool thpool;
	std::vector<epoller> eps;
	std::vector<task_queue<netio_task>> task_queues;
	std::map<int, netio_task> iowait_tasks;
};

#endif
