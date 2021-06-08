#include <cos_component.h>
#include <llprint.h>
#include <memmgr.h>

int
main(void)
{
	printc("Do a heap allocation\n");
	char *p = (char *)memmgr_heap_page_allocn(1);
	printc("Try to dereference the pointer\n");
	*p = '0';

	return 0;
}

void
cos_init(void)
{
}
