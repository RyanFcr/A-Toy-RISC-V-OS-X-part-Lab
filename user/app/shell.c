#include "syscall.h"
#include "stdio.h"
#include "string.h"

void runcmd(char* cmd) {
	// printf("welcome to runcmd\n");
	exec(cmd);
	fprintf(2, "exec %s failed\n", cmd);
	exit(0);
}

int getcmd(char *buf, int nbuf) {
	fprintf(2, "$ ");
	memset(buf, 0, nbuf);
	// printf("getcmd\n");
	gets(buf, nbuf);
	// printf("getcmd finish\n");
	printf("%s\n", buf);
	if (buf[0] == 0)
		return -1;
	return 0;
}

int main(void) {
	static char buf[100];
	int fd;
	// printf("welcome to our shell\n");
	while (getcmd(buf, sizeof(buf)) >= 0)
	{
		fprintf(2, "%s\n", buf);
		
		if (strcmp(buf, "hello") == 0) {
			int pid2 = fork();

			printf("Get pid %d\n", pid2);

			if(pid2 == 0)
				runcmd(buf);//子进程exec，替换为新的可执行文件
			else if (pid2 > 0) {
				wait();
			}
				
			else
				panic("panic fork");
		}
		else {
			printf("Invalid command %s\n", buf);
		}
		exit(0);
	}

	exit(0);
}