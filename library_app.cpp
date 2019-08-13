#include "pch.h"
#include "Tester.h"
#include <iostream>

int main()
{
	const char* urls[] = { "http://127.0.0.1:8081","http://127.0.0.1:8080", "http://127.0.0.1:8082" };

	std::cout << "\n\nTEST run started ...\n\n" << std::endl;


	{
		auto tester = test::Tester(urls, 3, "Library");

		tester.runInParallel(8);
	}
	std::cout << "\n\nTEST run finished ...\n\n" << std::endl;

}
