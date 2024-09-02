#ifndef AMANI_COROUTINE_POOL_H
#define AMANI_COROUTINE_POOL_H

#include <cstdlib>
#include <mutex>
#include <set>
#include <map>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <coroutine>
#include <functional>
#include <iostream>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>

#include "epoller.h"
#include "thpool/thpool.h"
#include "netio_task.h"

class netco_pool
{
public:
	netco_pool(unsigned int threadnum) : 
		terminated(false), threads(threadnum), thpool(threadnum), \
		task_queues(std::vector<task_queue<netio_task>>(threadnum)) {}

	void init()
	{
		/* 0. 创建epoll线程 */
		poll = new epoller();
		std::thread([this](){
			this->poll_run();
		});

		/* 1. 启动线程池, 等待任务 */
		thpool.init();

		/* 2. 向线程池的工作线程提交工作任务
		      任务为调度用户的协程
		 */
		for (unsigned int i = 0; i < threads; i++)
		{
			thpool.submit([i, this] {co_run(task_queues[i], poll);});
		}
	}

   	template <typename F, typename... Args>
	void submit(F &&f, Args &&...args)
	{
		static unsigned int thid = 0;

		/* 
		 1. submit 时, 直接运行协程, 由于协程设置启动时挂起
		    即可在这里取到协程的handle
		 2. 取到handle, 将返回的netio_task存储起来, 方便对协程进行控制(恢复)
		 3. 通过thid, 将协程均匀的放在多个线程中
		*/
		netio_task task_handle = f(args...);
		task_queues[thid++ % threads].enqueue(task_handle);
	}

	void shutdown(void)
	{
		terminated = true;
		thpool.shutdown();
	}

	void poll_run(void)
	{
		while (terminated)
		{
			if (poll->ioevent_handle() == -1) {
				LOG_ERROR << "poll thread exit!!!" << std::endl;
				return ;
			}
		}
	}

	void co_run(task_queue<netio_task>& task_que, poller* poll)
	{
		netio_task co_task;

		while (!terminated)
		{
			/* 从队列中取出任务 */
			if (!task_que.dequeue(co_task))
			{
				usleep(100);
				continue;
			}

			if (co_task.handle_.promise().run_state == CO_RUNNING)
			{
				/* 恢复协程运行 */
				co_task.handle_.resume();

				/* 协程恢复后再次挂起或返回，如果协程结束, 则销毁 */
				if (co_task.handle_.done())
				{
					// 结束时需要挂起, 因此需要手动销毁
					// 如果结束时不挂起, 则resume返回后handle就已经destroy(), 下面再使用不合法了
					co_task.handle_.destroy();

					// 不需要删除epoll中的fd, 当close(fd)时，会自动从epoll中移除
					continue;
				}

				/* 如果任务需要IO阻塞 */
				if (co_task.handle_.promise().run_state == CO_IOWAIT)
				{
					poll->ioevent_add(&co_task, co_task.handle_.promise().events);
				}
			}
		}
	}

private:
	bool terminated;

	unsigned int threads;
	thread_pool thpool;

	/* epoller */
	poller *poll;

	/* one thread one task queeue */
	std::vector<task_queue<netio_task>> task_queues;
};

#endif
