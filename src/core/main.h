#ifndef H_G
#define H_G
// 引入必要头文件
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/********** BEGIN : main.c文件 : BEGIN ***********/
// 定义解析完毕配置项后用于保存配置的struct变量
typedef struct tgx_config_struct {
  int worker_num;
  int port;
  int daemonize;
  char * event;
} tgx_config_t;
// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();
// daemonize函数
void be_daemon();
// 配置文件解析函数
void parse_conf_file( void );
// 创建listen socket
int create_listen_socket( void );
// 初始化默认配置
void init_default_config( void );
// fork进程
void fork_process( void );
/********** END : main.c文件 : END ***********/

// 定义全局使用的一些常量
#define MAXBUFFER 8192
#define MAX_FD_SIZE 1024

#endif
