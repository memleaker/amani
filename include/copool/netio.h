#ifndef AMANI_NETIO_H
#define AMANI_NETIO_H

#include <cstddef>
#include <cstdint>
#include <cerrno>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "copool.h"

int amani_socket(int domain, int type, int protocol)
{
	return socket(domain, type | SOCK_NONBLOCK, protocol);
}

int amani_listen(int fd, uint32_t ipaddr, uint16_t port)
{
	int ret;
	sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	ret = bind(fd, (const sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
		return ret;

	ret = listen(fd, 128);
	if (ret == -1)
		return ret;

	return 0;
}

class async_accept {
public:
	async_accept(int fd, sockaddr* addr, socklen_t *addrlen) : 
		m_fd(fd), m_connfd(-1), m_need_suspend(false), m_addr(addr), m_addrlen(addrlen) {}

    bool await_ready()
	{
		for (;;)
		{
			m_connfd = accept4(m_fd, m_addr, m_addrlen, SOCK_NONBLOCK);		
			if (m_connfd == -1)
			{
				if (errno == EAGAIN)
				{
					m_need_suspend = true;
					return false;  // suspend
				}

				if (errno == EINTR)
					continue;
			}

			return true; // dont't suspend
		}
	}

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().fd = m_fd;
		handle.promise().flags = EPOLLIN;
		handle.promise().need_block = true;
	}

    ssize_t await_resume()
	{
		if (!m_need_suspend)
			return m_connfd;

		for (;;)
		{
			m_connfd = accept4(m_fd, m_addr, m_addrlen, SOCK_NONBLOCK);	
			if (m_connfd == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
			}

			return m_connfd;
		}
	}

private:
	int     	m_fd;
	int         m_connfd;
	bool        m_need_suspend;
	sockaddr   *m_addr;
	socklen_t  *m_addrlen;
};


class async_read {
public:
	async_read(int fd, void *buf, size_t len) : 
			m_fd(fd), m_buf(buf), m_len(len), m_need_suspend(false) {}

    bool await_ready()
	{
		for (;;)
		{
			m_nbytes = read(m_fd, m_buf, m_len);
			if (m_nbytes == -1)
			{
				if (errno == EAGAIN)
				{
					m_need_suspend = true;
					return false;
				}
				
				if (errno == EINTR)
					continue;
			}

			return true;
		}
	}

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().fd = m_fd;
		handle.promise().flags = EPOLLIN;
		handle.promise().need_block = true;
	}

    ssize_t await_resume()
	{
		if (!m_need_suspend)
			return m_nbytes;

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
	bool    m_need_suspend;
};

class async_write {
public:
	async_write(int fd, void *buf, size_t len) : 
				m_fd(fd), m_buf(buf), m_len(len), m_need_suspend(false) {}

    bool await_ready()
	{
		for (;;)
		{
			m_nbytes = write(m_fd, m_buf, m_len);
			if (m_nbytes == -1)
			{
				if (errno == EAGAIN)
				{
					m_need_suspend = true;
					return false;
				}

				if (errno == EINTR)
					continue;
			}

			return true;
		}
	}

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().fd = m_fd;
		handle.promise().flags = EPOLLOUT;
		handle.promise().need_block = true;
	}

    ssize_t await_resume()
	{
		if (!m_need_suspend)
			return m_nbytes;

		for (;;)
		{
			m_nbytes = write(m_fd, m_buf, m_len);
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
	bool    m_need_suspend;
};

#endif