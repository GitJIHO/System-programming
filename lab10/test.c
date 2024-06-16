#include <stdio.h>
#include <unistd.h>

int main(int ac, char *av[])
{
	printf("%d\n", ac);
	for(int i=0; i<ac; i++) {
		printf("args[%d] %s\n", i, av[i]);
	}
	fprintf(stderr, "err");

}
