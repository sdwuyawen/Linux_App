/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/05/2015 10:53:20 AM
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

typedef void (*pfunc_t)(int);

pfunc_t pfunc;

void func_a(int val)
{
	printf("func_a, val = %d\n", val);
}

int main(void)
{
	pfunc = func_a;

	pfunc(2);
	
	return 0;
}
