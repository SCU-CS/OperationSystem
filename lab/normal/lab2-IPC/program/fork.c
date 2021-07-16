#include <string.h>
#include <stdio.h>
#include <errno.h>

/*
子进程输出根目录下的文件信息，相当于执行 "ls -l /"
*/

int main(int argc,char *argv[])	// argc是参数个数，*argv[]是指向字符串的指针数组
{
  int pid;						//保存进程ID
  char *prog_argv[4];

  prog_argv[0]="/bin/ls";
  prog_argv[1]="-l";
  prog_argv[2]="/";
  prog_argv[3]=NULL; 			

  if ((pid=fork())<0) 			// 创建进程pid=fork()
  {
    perror("Fork failed");		//输出错误信息
    exit(errno);
  }

  if (!pid) 					// 进程Id等于0,即子进程执行的代码块
  {
    execvp(prog_argv[0],prog_argv);
    // execvp()会从PATH环境变量所指的目录中查找符合参数file 的文件名，找到后便执行该文件，然后将第二个参数argv传给该欲执行的文件
    //相当于执行/bin/ls -l / (/表示根目录)
  }

  if (pid) 						//父进程执行的代码块
  {
    waitpid(pid,NULL,0);		//等待子进程执行结束
  }
}
