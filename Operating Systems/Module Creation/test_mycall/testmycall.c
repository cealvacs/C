#include<stdio.h>
#include<unistd.h>
#include "testmycall.h"

int main(void)
{
	int curr_pid = getpid();
	printf("%d\n", (int)syscall(__NR_mycall, 15));
	printf("%d\n", (int)syscall(__NR_carlosalvacall, curr_pid));

}

