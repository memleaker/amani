#ifndef AMANI_SSL_CLIENT_H
#define AMANI_SSL_CLIENT_H

#include <cstdio>  
#include <cstdlib>  
#include <cstring>  

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

class ssl_client
{
public:
	ssl_client() = delete;
	ssl_client(int sock, SSL_CTX *ctx) : sock(sock), ssl(nullptr), ctx(ctx) {}
	~ssl_client()
	{
		if (ssl)
		{
    		SSL_shutdown(ssl);
    		SSL_free(ssl);
		}

		if (sock != -1)
    		close(sock);
	}

	// 如果两次调用 connect ， 第一次失败, 怎么处理
	int connect(sockaddr_in *addr)
	{
		int ret;
		sockaddr_in server;

    	ssl = ::SSL_new(ctx);
		if (!ssl)
			return -1;

		ret = ::connect(sock, (struct sockaddr *)addr, sizeof(*addr));
		if (ret == -1)
			goto out;

    	ret = ::SSL_set_fd(ssl, sock);
		if (ret == -1)
			goto out;

    	ret = ::SSL_connect(ssl);
		if (ret == -1)
			goto out;

		return 0;

	out:
		SSL_free(ssl);
		return -1;
	}

	int read()
	{
		// ::SSL_read();
	}

	int write()
	{
		// ::SSL_write();
	}

	class openssl
	{
	public:
		static SSL_CTX* init_ssl_env(void)
		{
    		::SSL_library_init();
    		::SSL_load_error_strings();
			return ::SSL_CTX_new(TLS_client_method());
		}

		static void free_ssl_env(SSL_CTX *ctx)
		{
			::SSL_CTX_free(ctx);
		}
	};
	
private:
	int sock;
	SSL *ssl;
	SSL_CTX *ctx;
};

#endif