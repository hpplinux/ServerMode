#include "Server.h"

int main(int argc,char *argv[])
{
	if (argc != 3) {
		LOG_INFO("the parameter number is wrong");
		return -1;
	}

	Server server(argv[1],atoi(argv[2]));
	server.Initialize();
	server.Run();
	return 0;
}
