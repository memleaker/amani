#ifndef AMANI_BENCH_H
#define AMANI_BENCH_H

#include <iostream>
#include <vector>
#include <cstddef>

#include "http.h"
#include "copool/netio.h"

netio_task http10_bench(std::vector<char>& req, uint32_t ipaddr, uint16_t port)
{
	int sock, ret;
	char buf[8192];
	sockaddr_in addr;
	netio_task task;
	http_response resp;
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

		/* recv response */
		for (;;)
		{
			n = co_await async_read(sock, buf, sizeof(buf));
			if (n < 0)
			{
				std::cerr << "read response failed: " << std::strerror(errno) << std::endl;
				// 统计信息
				co_return -1;
			}
			else if (n == 0)
			{
				// connection closed by peer
				// 统计信息
				co_return 0;
			}

			// if (resp.parse(buf, n))
			// 	break;
		}

		/* 统计信息 */
		// std::cout << "code: " << resp.status_code << std::endl;

		close(sock);

		sleep(1);
	}
}

#endif