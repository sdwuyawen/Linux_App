/*
 * =====================================================================================
 *
 *       Filename:  time_delta.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2015 02:17:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
	printf("Hello, world!\n");
	struct timeval tvafter,tvpre;
	struct timezone tz;
	int sum = 0;      
	int i=0;
	gettimeofday (&tvpre , &tz);

	usleep(1);
#if 0
	for(i = 0; i < 100000000; i++)
	{
		sum += i;
	} 
#endif	
	gettimeofday (&tvafter , &tz);

//	printf("sum=%d , time cost:%d\n",sum, (tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000);
	printf("time0 = %ds, %dus\n",  tvpre.tv_sec, tvpre.tv_usec);
	printf("time1 = %ds, %dus\n",  tvafter.tv_sec, tvafter.tv_usec);
	return EXIT_SUCCESS;
}
