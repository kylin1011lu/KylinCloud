#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc,char*argv[])
{
	struct stat st;
	stat(argv[1],&st);

	printf("%s file size =%d\n",argv[0],st.st_size);
	printf("mode mt:%#x\n",(st.st_mode & S_IFMT));


	if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
		printf("test\n");

	return 0;
}
