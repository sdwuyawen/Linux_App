#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


/* system函数调用/bin/sh  执行特定的shell命令，阻塞当前的进程知道shell命令执行完毕。
 * #include
 * int system(const char *command);
 * 执行system实际上是调用了fork函数(产生新进程)、exec函数(在新进程中执行新任务)、waitpid函数(等待新进程结束)。
 */

int main(int argc,char **argv)
{
    int ret;
    
    printf("当前进程的进程号为%d\n",getpid());
    ret = system("ping 202.194.201.252 -c 1 -w 1");  //调用shell命令 ls -l
    printf("ret = %d\n",ret);
    return 0;
}
