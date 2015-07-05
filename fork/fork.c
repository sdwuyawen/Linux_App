/*
 * =====================================================================================
 *
 *       Filename:  fork.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/04/2015 02:56:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, int argv[])
{
	pid_t pid;

	pid = fork();

	if(pid < 0)
	{
		printf("fork error\n");
	}
	else if(!pid)
	{
		printf("son's pid is %d\n", getpid());
	}
	else
	{
		printf("father's pid is %d\n", getpid());
	}
	
	return 0;
}
