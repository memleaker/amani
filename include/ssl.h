// g++ hs.cc -o hs -lssl -lcrypto

#ifndef AMANI_SSL_H
#define AMANI_SSL_H

#include <openssl/ssl.h>  
#include <openssl/err.h>

class ssl {
public:
	class openssl
	{
	public:
		static void init_ssl_env(void)
		{
    		::SSL_library_init();
    		::SSL_load_error_strings();
		}

		static SSL_CTX* new_ssl_ctx()
		{
			return ::SSL_CTX_new(TLS_client_method());
		}

		static void free_ssl_ctx(SSL_CTX *ctx)
		{
			::SSL_CTX_free(ctx);
		}
	};
};

#endif
