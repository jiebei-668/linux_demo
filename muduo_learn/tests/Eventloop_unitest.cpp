#include "../Eventloop.h"
#include "../Channel.h"

int main(int argc, char* argv[])
{
	Eventloop el;
	Channel cnl(&el, 0);
	return 0;
}
