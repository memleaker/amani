#include "argument.h"

static struct option long_options[] = {
    { "url",     no_argument, NULL, 'i' },
    { "http10",  no_argument, NULL, 0 },
    { "get",     no_argument, NULL, 1 },
    { "post",    no_argument, NULL, 2 },
	{ "help",    no_argument, NULL, 'h'},
};

static void usage(void)
{
	std::cout << "Usage: amani [arguments]\n" <<
				 "Arguments:\n" <<
				 "\t-i [url] or --url [url]\n" <<
				 "\t-c [clients]\n" <<
				 "\t-d [time]\n" <<
				 "\t--http10\n" << 
				 "\t--get\n" <<
				 "\t--post" << std::endl; 
}

void argument::parse(int argc, char **argv)
{
    int opt, opidx;

	/* 1. Parse Arguments */
    while (-1 != (opt = getopt_long(argc, argv, "i:c:d:h", long_options, &opidx)))
    {
        switch (opt)
	{
	case 'c':
		/* Clients Number. */
		clients = std::atoi(optarg);
		if (clients < 0) {
			std::cerr << "client need greater than zero" << std::endl;
			std::exit(1);
		}
		break;
	case 'd':
		/* Test Time. */
		time = std::atoi(optarg);
		if (clients < 0) {
			std::cerr << "time need greater than zero" << std::endl;
			std::exit(1);
		}
		
		break;
	case 'i':
		/* URL */
		urlstr = optarg;
		break;
	case 0:
		http_version = HTTP10;
		break;
	case 1:
		meth = GET;
		break;
	case 2:
		meth = POST;
		break;
	case '?':
	case 'h':
	default:
		usage();
		std::exit(1);
	}
    }

	/* 2. Check Must Option. */
	if (urlstr == "") {
		std::cout << "url is must option" << std::endl;
		std::exit(1);
	}

	/* 3. Parse URL. */
	urlinfo.parse(urlstr);
}
