#include <cos_component.h>
#include <llprint.h>
#include <filesystem.h>

int 
main(void)
{
	printc("Hello world!\n");

	fs_fopen("/test.txt", "r+w");

	while (1) ;
}
