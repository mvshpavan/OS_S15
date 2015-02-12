#include <stdlib.h>
#include<stdio.h>
uint64_t *get_rsp();

void _start(void) {
	uint64_t count,i;
	char* argv[0];
	char* envp[0];
	int res;
	uint64_t *rsp;
	__asm__
	(
		"mov %%rsp,%0\n\t"
		:"=r" (rsp)
	);
//	printf("1) %x\n",*rsp);
	printf("2) %x\n",*(rsp+5));
	rsp=rsp+5;	//rsp+1 implies after 8 bits we have argc
	printf("3) %x\n",*rsp);
	count=*rsp;
	i=1;
	while(i<count){
		printf("argc %s\n",*(rsp+i));
		i++;
	}
	res = main(*rsp, argv, envp);
	exit(res);
}


