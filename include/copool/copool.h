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
		terminated(true), threads(threadnum), thpool(threadnum) {}

    /* @brief 禁用拷贝和移动 */
    netco_pool(const netco_pool &) = delete;
    netco_pool(netco_pool &&) noexcept = delete;
    netco_pool &operator=(const netco_pool &) = delete;
    netco_pool &operator=(netco_pool &&) noexcept = delete;

public:
	/* @brief 初始化协程池 */
	void init(void)
	{
		/* 0. 标记协程池运行状态 */
		terminated = false;

		/* 1. 创建epoll线程 */
		poll = std::make_shared<epoller>();

		/* 2. 启动线程池, 等待任务 */
		thpool.init();

		/* 3. 向线程池的工作线程提交工作任务
		      一个任务为监控IO事件线程，其它为调度用户协程的进程
		 */
		thpool.submit([this] { this->poll_run(); });
		for (unsigned int i = 1; i < threads; i++)
		{
			thpool.submit([this] {this->co_run();});
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
		/* 
		 1. submit 时, 直接运行协程, 由于协程设置启动时挂起
		    即可在这里取到协程的handle
		 2. 取到handle, 将返回的netio_task存储起来, 方便对协程进行控制(恢复)
		*/
		netio_task task_handle = f(args...);
		task_que.enqueue(task_handle);
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
     *        这里使用了一个所有线程公用的任务队列和各自线程独有的任务列表
	 *        避免多线程使用同一个任务列表时，遍历任务调度时要加锁的情况
	 * @param task_que 保存用户submit的协程任务
	 * @param poll IO多路复用对象，用于监控IO事件
	 */
	void co_run(void)
	{
		netio_task t;
		std::list<netio_task> task_list;

		while (!terminated)
		{
			/* 0. 从任务队列中取一个任务放到任务列表中 */
			if (task_que.dequeue(t))
			{
				task_list.emplace_back(t);
			}

			/* 1. 等待任务列表不为空 */
			if (task_list.empty())
			{
				usleep(100);
				continue;
			}

			/* 2. 从任务列表中取出任务执行 */
			for (auto it = task_list.begin(); \
				((!terminated) && (it != task_list.end()));)
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
						task_list.erase(it++);  /* 传递给erase一个副本, 自身自增 */
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

				/* 为实现遍历中删除节点, 不在for语句中写自增 */
				it++;
			}
		}
	}

private:
	bool terminated;

	/* 线程池 */
	unsigned int threads;
	thread_pool thpool;

	/* poller */
	std::shared_ptr<poller> poll;

	/* 公用任务队列, 用于submit任务, 以及线程取任务 */
	task_queue<netio_task> task_que;
};

#endif
