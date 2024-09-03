#ifndef AMANI_COROUTINE_POOL_H
#define AMANI_COROUTINE_POOL_H

#include <cstdlib>
#include <mutex>
#include <list>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <memory>
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
		task_queues(std::vector<std::list<netio_task>>(threadnum)) {}

    /* @brief 禁用拷贝和移动 */
    netco_pool(const netco_pool &) = delete;
    netco_pool(netco_pool &&) noexcept = delete;
    netco_pool &operator=(const netco_pool &) = delete;
    netco_pool &operator=(netco_pool &&) noexcept = delete;

public:
	/* @brief 初始化协程池 */
	void init(void)
	{
		/* 0. 创建epoll线程 */
		poll = std::make_shared<epoller>();
		std::thread([this]() { this->poll_run(); });

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

	/* @brief 关闭协程池 */
	void shutdown(void)
	{
		terminated = true;
		thpool.shutdown();
	}

	/* 
	 * @brief 向协程池提交任务
	 * @param f	待执行任务的函数名
	 * @param args 待执行任务的参数
	 */
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
		task_queues[thid++ % threads].emplace_back(task_handle);
	}

	/* 
	 * @brief 对协程IO事件进行监控, 发生IO事件时修改协程状态
	 */
	void poll_run(void)
	{
		while (terminated)
		{
			if (poll->ioevent_handle() == -1)
			{
				LOG_ERROR << "poll thread exit!!!" << std::endl;
				return ;
			}
		}
	}

	/* 
	 * @brief 对协程进行调度, 销毁运行结束的协程, 处理协程IO事件
	 * @param task_que 保存该线程管理的所有协程
	 * @param poll IO多路复用对象，用于监控IO事件
	 */
	void co_run(std::list<netio_task>& task_que, std::shared_ptr<poller> poll)
	{
		while (!terminated)
		{
			if (task_que.empty())
			{
				usleep(100);
				continue;
			}

			/* 从队列中取出任务执行 */
			for (auto it = task_que.begin(); \
				((!terminated) && (it != task_que.end())); it++)
			{
				if (it->handle_.promise().run_state == CO_RUNNING)
				{
					/* 恢复协程运行 */
					it->handle_.resume();

					/* 协程恢复后再次挂起或返回，如果协程结束, 则销毁 */
					if (it->handle_.done())
					{
						/* 设置协程在结束时挂起, 因为下面还要使用
						 * 如果结束时不挂起, 则resume返回后handle就已经销毁, 后面不能再使用
						 * 设置了结束时挂起, 需要手动销毁协程: 调用 destroy()
						 */
						it->handle_.destroy();
						task_que.erase(it);
						continue;
					}

					/* 如果任务需要IO阻塞, 将IO任务交由Epoll监控
					 * 在监控过程中, 该协程不会被调度执行, 直到IO事件发生, 协程状态恢复为RUNNING
					 */
					if (it->handle_.promise().run_state == CO_IOWAIT)
					{
						/* &*it 取到元素的地址 */
						poll->ioevent_add(&(*it), it->handle_.promise().events);
					}
				}
			}
		}
	}

private:
	bool terminated;

	/* threads */
	unsigned int threads;
	thread_pool thpool;

	/* poller */
	std::shared_ptr<poller> poll;

	/* one thread one task queeue */
	std::vector<std::list<netio_task>> task_queues;
};

#endif
