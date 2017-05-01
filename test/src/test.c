#include <stdio.h>

int main()
{
	int i=0;
	
	int ref = 0;
	flush(&ref);

	printf("flush %d\n", ref);
	return 0;
}

void flush(int *ref)
{
	*ref = 10;
}
