
#include <signal.h>

#include "utils.h"
#include "stats.h"
#include "argument.h"
#include "http.h"
#include "bench.h"
#include "ssl.h"
#include "thpool/thpool.h"
#include "copool/copool.h"
#include "copool/netio.h"

int main(int argc, char **argv)
{
    stats st;
	int time {0};
	SSL_CTX *ctx;

	/* parse argument */
    argument arg;
	arg.parse(argc, argv);

	/* signals */
    signal(SIGPIPE, SIG_IGN);

	/* ssl */
	ssl::openssl::init_ssl_env();
	ctx = ssl::openssl::new_ssl_ctx();

	/* build request */
	std::vector<char> buf;
	http_request req;
	if (arg.reqfile != "")
	{
		// read file, fill buf
	}
	else
	{
		req.set_version(HTTP11);
		req.build_request(buf);
	}

	/* running */
	netco_pool pool(utils::cpu_num(4));
	pool.init();
	pool.submit(ssl_bench, ctx, buf, inet_addr("220.181.38.149"), 443);
	pool.run();

	/* stat */
    for (time = 0; time < arg.time; time++)
	{
    	// print_stats(st); // 每秒打印一次打印stats
		sleep(1);
	}

	/* stop */
	pool.shutdown();

    return 0;
}
