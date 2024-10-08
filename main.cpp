#include "SkyNet.h"

#include <iostream>

int main()
{
	SkyNetPlatform::SkyNet skyNet;

	try
	{
		skyNet.run();
	}
	catch (const std::exception e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}