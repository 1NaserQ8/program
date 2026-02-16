#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {
	pid_t pid = fork();

	if(pid>0){
		printf("fork faild");
	}else
		if(pid == 0){
			printf("PID: %d\n",getpid());
		}

  return 0;
}
