#ifndef AMANI_BENCH_H
#define AMANI_BENCH_H

#include <iostream>
#include <vector>
#include <cstddef>
#include <sys/time.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "stats.h"
#include "utils.h"
#include "http.h"

#include "copool/netio.h"

netio_task http10_bench(std::vector<char>& req, stats &st, uint32_t ipaddr, uint16_t port)
{
	int sock, ret;
	char buff[8192];
	sockaddr_in addr;
	netio_task task;
	buffer buf(8192);
	http_response resp;
	struct timeval start, end;
	ssize_t nleft, nwrite, n;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(port);

	for (;;)
	{
		/* socket */
		sock = amani_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == -1)
		{
			std::cerr << "createa socket failed: " << std::strerror(errno) << std::endl;
			co_return -1;
		}

		/* connect */
		ret = co_await async_connect(sock, (sockaddr*)&addr, sizeof(addr));
		if (ret == -1)
		{
			std::cerr << "connect failed: " << std::strerror(errno) << std::endl;
			co_return -1;
		}

		::gettimeofday(&start, NULL);

		/* send data */
		for (nleft = req.size(), nwrite = 0; nleft > 0; )
		{
			n = co_await async_write(sock, req.data()+nwrite, nleft);
			if (n == -1)
			{
				std::cerr << "send request failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}

			nwrite += n;
			nleft  -= n;
		}

		st.inc_request();

		/* recv response */
		for (;;)
		{
			n = co_await async_read(sock, buff, sizeof(buff));
			if (n < 0)
			{
				std::cerr << "read response failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}
			else if (n == 0)
			{
				// connection closed by peer
				std::cout << "closed" << std::endl;
				co_return 0;
			}

			buf.push_back(buff, n);
			ret = resp.parse(buf);
			if (ret == -1)
				continue;
			else if (ret == 0)
			{
				::gettimeofday(&end, NULL);
				st.inc_response();
				st.inc_status(resp.status_code);
				st.set_rtt(utils::rtt(start, end));
				std::cout << "code: " << resp.status_code << std::endl;
				break;
			}
			else
			{
				std::cout << "Recv Invalid HTTP Response" << std::endl;
				break;
			}
		}

		buf.clear();
		close(sock);
		usleep(100);
	}
}

netio_task http11_bench(std::vector<char>& req, stats& st, uint32_t ipaddr, uint16_t port)
{
	int sock, ret;
	char buff[8192];
	sockaddr_in addr;
	netio_task task;
	buffer buf(8192);
	http_response resp;
	ssize_t nleft, nwrite, n;
	struct timeval start, end;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(port);

	/* socket */
	sock = amani_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		std::cerr << "createa socket failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	/* connect */
	ret = co_await async_connect(sock, (sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		std::cerr << "connect failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	for (;;)
	{
		::gettimeofday(&start, NULL);

		/* send data */
		for (nleft = req.size(), nwrite = 0; nleft > 0; )
		{
			n = co_await async_write(sock, req.data()+nwrite, nleft);
			if (n == -1)
			{
				std::cerr << "send request failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}

			nwrite += n;
			nleft  -= n;
		}

		st.inc_request();

		/* recv response */
		for (;;)
		{
			n = co_await async_read(sock, buff, sizeof(buff));
			if (n < 0)
			{
				std::cerr << "read response failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}
			else if (n == 0)
			{
				// connection closed by peer
				std::cout << "conn closed" << std::endl;
				co_return 0;
			}

			buf.push_back(buff, n);
			ret = resp.parse(buf);
			if (ret == -1)
				continue;
			else if (ret == 0)
			{
				::gettimeofday(&end, NULL);
				st.inc_response();
				st.inc_status(resp.status_code);
				st.set_rtt(utils::rtt(start, end));
				//debug std::cout << "code: " << resp.status_code << std::endl;
				break;
			}
			else
			{
				std::cout << "Recv Invalid HTTP Response" << std::endl;
				break;
			}
		}

		buf.clear();
		usleep(1000);
	}
}

netio_task ssl_bench(SSL_CTX* ctx, std::vector<char>& req, stats &st, uint32_t ipaddr, uint16_t port)
{
	int sock, ret;
	char errs[2048];
	char buff[8192];
	sockaddr_in addr;
	netio_task task;
	buffer buf(8192);
	http_response resp;
	ssize_t nleft, nwrite, n;
	SSL *ssl;
	struct timeval start, end;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(port);

	/* socket */
	sock = amani_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		std::cerr << "createa socket failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	/* connect */
	ret = co_await async_connect(sock, (sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		std::cerr << "connect failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	/* SSL_connect */
	ssl = SSL_new(ctx);
	if (!ssl)
	{
		std::cerr << "SSL_new failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	ret = SSL_set_fd(ssl, sock);
	if (ret != 1)
	{
		std::cerr << "SSL_set_fd failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

    for (;;)
	{
		ret = co_await async_sslconnect(ssl, sock);
		if (ret < 0)
		{
			std::cerr << "SSL_do_hand failed: " << std::strerror(errno) << std::endl;
			co_return -1;
		}

		if (ret == 1)
			break;
	}

	for (;;)
	{
		::gettimeofday(&start, NULL);

		/* send data */
		for (nleft = req.size(), nwrite = 0; nleft > 0; )
		{
			n = co_await async_sslwrite(ssl, sock, req.data()+nwrite, nleft);
			if (n <= 0)
			{
				int err = SSL_get_error(ssl, n);
				std::cerr << "send request failed: " << ERR_error_string(err, errs) << std::endl;
				co_return -1;
			}

			nwrite += n;
			nleft  -= n;
		}

		/* stats */
		st.inc_request();

		/* recv response */
		for (;;)
		{
			n = co_await async_sslread(ssl, sock, buff, sizeof(buff));
			if (n <= 0)
			{
				std::cerr << "read response failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}

			buf.push_back(buff, n);
			ret = resp.parse(buf);
			if (ret == -1)
				continue;
			else if (ret == 0)
			{
				::gettimeofday(&end, NULL);
				st.inc_response();
				st.inc_status(resp.status_code);
				st.set_rtt(utils::rtt(start, end));
				break;
			}
			else
			{
				std::cout << "Recv Invalid HTTP Response" << std::endl;
				break;
			}
		}

		buf.clear();
		usleep(1000000);
	}
}

#endif
