
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

	/* parse argument */
    argument arg;
	arg.parse(argc, argv);

	/* signals */
    signal(SIGPIPE, SIG_IGN);

#ifdef HTTPS_SUPPORT
	/* ssl */
	SSL_CTX *ctx;
	ssl::openssl::init_ssl_env();
	ctx = ssl::openssl::new_ssl_ctx();
#endif

	/* build request */
	std::vector<char> buf;
	http_request req;
	req.set_version(arg.http_version);
	req.set_method(arg.meth);
	req.set_uri(arg.urlinfo.uri);
	req.build_request(buf);

	/* running */
	netco_pool pool(utils::cpu_num(4));
	pool.init();
	for (int i = 0; i < arg.clients; i++)
	{
		if (arg.http_version == HTTP11) {
#ifdef HTTPS_SUPPORT
			if (arg.urlinfo.proto == "https") {
	 			pool.submit(ssl_bench, ctx, buf, st, inet_addr(arg.urlinfo.ipaddr.c_str()), arg.urlinfo.port);
			} else {
#endif
				pool.submit(http11_bench, buf, st, inet_addr(arg.urlinfo.ipaddr.c_str()), arg.urlinfo.port);
#ifdef HTTPS_SUPPORT
			}
#endif
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

	/* running */
	pool.run();

	/* stat */
	st.print_status(arg.time, arg.urlstr, arg.urlinfo.ipaddr);

	/* stop */
	pool.shutdown();

    return 0;
}
