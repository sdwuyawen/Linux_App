/*
 * =====================================================================================
 *
 *       Filename:  time.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2015 10:00:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <sys/time.h>

int main(void)
{
	struct timeval tv;
	int ret;

	ret = gettimeofday(&tv, NULL);
	if (ret) {
		perror("gettimeofday");
	} else {
		printf("seconds=%ld useconds=%ld\n",
				(long)tv.tv_sec, (long)tv.tv_usec);
	}

	return 0;
}
