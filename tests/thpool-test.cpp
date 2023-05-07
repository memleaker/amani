#include <iostream>
#include <chrono>
#include <thread>

#include "thpool/thpool.h"

int test1(void)
{
	int i;

    for (i = 1; i < 20; i++)
        std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "test1 return" << std::endl;

	return i;
}

int test2(void)
{
	int i;

    for (i = 1; i < 20; i+=2)
        std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "test2 return" << std::endl;

	return i;
}

int main(void)
{
    thread_pool pool(2);

	pool.init();

	auto t1 = pool.submit(test1);
	auto t2 = pool.submit(test2);
	
	int t1ret = t1.get();
	int t2ret = t2.get();

	std::cout << "t1 return " << t1ret << std::endl;
	std::cout << "t2 return " << t2ret << std::endl;

	pool.shutdown();

    return 0;
}