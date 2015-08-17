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

int main(int argc, char *argv[])
{
	int i;
	int count = 0;

	for(i = 0; i < 4; i++)
	{
		fork();
		printf("-\n");
	}
	
	return 0;
}
