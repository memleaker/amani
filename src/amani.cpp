
#include <signal.h>
#include <sys/time.h>

#include "utils.h"
#include "stats.h"
#include "argument.h"
#include "http.h"
#include "bench.h"
#include "thpool/thpool.h"
#include "copool/copool.h"
#include "copool/netio.h"

#ifdef HTTPS_SUPPORT
#include "ssl.h"
#endif

int main(int argc, char **argv)
{
    stats st;

	/* 1. Parse argument */
    argument arg;
	arg.parse(argc, argv);

	/* 2. Signals */
    signal(SIGPIPE, SIG_IGN);

#ifdef HTTPS_SUPPORT
	SSL_CTX *ctx;
	ssl::openssl::init_ssl_env();
	ctx = ssl::openssl::new_ssl_ctx();
#endif

	/* 3. Build HTTP request */
	std::vector<char> buf;
	http_request req;
	req.set_version(arg.http_version);
	req.set_method(arg.meth);
	req.set_uri(arg.urlinfo.uri);
	req.build_request(buf);

	/* 4. Create Thread Pool And Coroutine Pool */
	netco_pool pool(utils::cpu_num(4));
	pool.init();

	/* One client one coroutine
	 * use pool.submit() to create coroutine
	 */
	for (int i = 0; i < arg.clients; i++)
	{
		if (arg.http_version == HTTP11) {
			if (arg.urlinfo.proto == "https") {
#ifdef HTTPS_SUPPORT
	 			pool.submit(ssl_bench, ctx, buf, st, inet_addr(arg.urlinfo.ipaddr.c_str()), arg.urlinfo.port);
#else
				std::cerr << "Error: https is not support" << std::endl;
				std::exit(1);
#endif
			} else {
				pool.submit(http11_bench, buf, st, inet_addr(arg.urlinfo.ipaddr.c_str()), arg.urlinfo.port);
			}
		} else if (arg.http_version == HTTP10) {
			if (arg.urlinfo.proto == "https") {
				std::cerr << "Error: http10 is not support https" << std::endl;
				std::exit(1);
			}
		 	pool.submit(http10_bench, buf, st, inet_addr(arg.urlinfo.ipaddr.c_str()), arg.urlinfo.port);
		} else {
			std::cerr << "FATAL: internal error" << std::endl;
			std::exit(1);
		}
	}

	/* 5. Start Running */
	pool.run();

	/* 6. Print stat Util end of time */
	st.print_status(arg.time, arg.urlstr, arg.urlinfo.ipaddr);

	/* 7. stop */
	pool.shutdown();
    return 0;
}
