#include "argument.h"

static struct option long_options[] = {
    { "url",     no_argument, NULL, 'i' },
    { "reqfile", no_argument, NULL, 'r' },
    { "http10",  no_argument, NULL, 0 },
    { "get",     no_argument, NULL, 1 },
    { "post",    no_argument, NULL, 2 },
    { "https",   no_argument, NULL, 3 },
	{ "header-key", required_argument, NULL, 4 },
	{ "header-val", required_argument, NULL, 5 },
};

void argument::parse(int argc, char **argv)
{
    int opt, opidx;

    while (-1 != (opt = getopt_long(argc, argv, "i:r:c:d:h", long_options, &opidx)))
    {
        switch (opt)
	{
	case 'c':
		clients = std::atoi(optarg);
		if (clients < 0) {
			std::cerr << "client need greater than zero" << std::endl;
			std::exit(1);
		}
		break;
	case 'd':
		time = std::atoi(optarg);
		if (clients < 0) {
			std::cerr << "time need greater than zero" << std::endl;
			std::exit(1);
		}
		
		break;
	case 'r':
		reqfile = optarg;
		break;
	case 'i':
		urlstr = optarg;
		break;
	case 'h':
		break;
	case 0:
		http_version = HTTP10;
		break;
	case 1:
		meth = GET;
		break;
	case 2:
		break;
		meth = POST;
	case 3:
		https = true;
		break;
	default:
		break;
	}
    }

	/* check */
	if (urlstr == "") {
		std::cout << "url is must option" << std::endl;
		std::exit(1);
	}

	if ((http_version == HTTP10) && https) {
		std::cout << "--http10 cannot used with --https" << std::endl;
		std::exit(1);
	}

	/* parse url */
	urlinfo.parse(urlstr);
}
