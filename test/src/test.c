#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc,char*argv[])
{
<<<<<<< HEAD
	int i=0;
	
	int ref = 0;
	flush(&ref);
=======
	struct stat st;
	stat(argv[1],&st);

	printf("%s file size =%d\n",argv[0],st.st_size);
	printf("mode mt:%#x\n",(st.st_mode & S_IFMT));


	if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
		printf("test\n");
>>>>>>> d62790ced9bbb792f13fb3fa9ecff698abed3c24

	printf("flush %d\n", ref);
	return 0;
}

void flush(int *ref)
{
	*ref = 10;
}
