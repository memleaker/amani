
#include <signal.h>

#include "utils.h"
#include "stats.h"
#include "ssl_client.h"
#include "argument.h"
#include "http.h"
#include "thpool/thpool.h"
#include "copool/copool.h"
#include "copool/netio.h"

int main(int argc, char **argv)
{
    stats st;
	int time {0};

	/* parse argument */
    argument arg;
	arg.parse(argc, argv);

	/* signals */
    signal(SIGPIPE, SIG_IGN);

	/* ssl */
    // ssl_client sslc(1, ssl_client::openssl::init_ssl_env());

	/* build request */
	std::vector<char> buf;
	http_request req;
	if (arg.reqfile != "")
	{
		// read file, fill buf
	}
	else
	{
		req.build_request(buf);	
	}
    
	/* running */
	netco_pool pool(utils::cpu_num(4));
	pool.init();
	pool.submit(echo_server, pool);
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
