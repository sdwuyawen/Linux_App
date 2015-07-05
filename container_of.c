/*
 * =====================================================================================
 *
 *       Filename:  container_of.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/03/2015 10:50:42 AM
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

#define offsetof(TYPE, MEMBER) ((unsigned int) (&((TYPE *)0)->MEMBER))
#define container_of(ptr, type, member) ((void *)((char *)ptr - offsetof(type, member)))

typedef struct 
{
	char a;
	int b;
	char c;
	int d;
	int e;
}T_test;

T_test t_test;
int main(int argc, char *argv[])
{
	char *head;
	printf("&t_test = %08x\n", &t_test);

	head = container_of(&t_test.d, T_test, d);
	printf("head = %08x\n", head);

	return 0;
}
