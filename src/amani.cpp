
#include <signal.h>

#include "utils.h"
#include "stats.h"
#include "ssl_client.h"
#include "thpool/thpool.h"
#include "copool/copool.h"

int main(int argc, char **argv)
{
    stats st;
    ssl_client sslc(1, ssl_client::openssl::init_ssl_env());

    signal(SIGPIPE, SIG_IGN);
    
    thread_pool thpool{static_cast<int>(utils::cpu_num(4))};

    thpool.init();

    // auto costat = thpool.submit(print_stats, st); 每秒打印一次打印stats

    // costat.get();

    thpool.shutdown();

    return 0;
}