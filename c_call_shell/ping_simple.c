#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


/* system函数调用/bin/sh  执行特定的shell命令，阻塞当前的进程知道shell命令执行完毕。
 * #include
 * int system(const char *command);
 * 执行system实际上是调用了fork函数(产生新进程)、exec函数(在新进程中执行新任务)、waitpid函数(等待新进程结束)。
 */

/* 函数说明 system()会调用fork()产生子进程，由子进程来调用/bin/sh-c string来执行参数string字符串所代表的命令，此命令执行完后随即返回原调用的进程。
 * 在调用system()期间SIGCHLD 信号会被暂时搁置，SIGINT和SIGQUIT 信号则会被忽略。
 * 返回值 如果system()在调用/bin/sh时失败则返回127，其他失败原因返回-1。若参数string为空指针(NULL)，则返回非零值。
 * 如果system()调用成功则最后会返回执行shell命令后的返回值，但是此返回值也有可能为system()调用/bin/sh失败所返回的127，因此最好能再检查errno 来确认执行成功。
 * 附加说明 在编写具有SUID/SGID权限的程序时请勿使用system()，system()会继承环境变量，通过环境变量可能会造成系统安全的问题。 
 */

int main(int argc,char **argv)
{
    int ret;
    
    printf("当前进程的进程号为%d\n",getpid());
    ret = system("ping 202.194.201.252 -c 1 -w 1");  //调用shell命令 ls -l
    printf("ret = %d\n",ret);
    return 0;
}
