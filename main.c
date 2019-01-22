#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "src/core/cJSON.h"

#define MAXBUFFER 8192

// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();

// 子进程数量
int worker_num = 4;
int port       = 6666;

// 环境变量数组 
extern char ** environ;

int main( int argc, char * argv[] ) {
    
  /*
  for ( int i = 0; NULL != argv[ i ]; i++ ) {
    printf( "%p : %s\n", argv[ i ], argv[ i ] );
  }
  */
  //printf( "%p\n", argv[ 0 ] ); 
  char * process_title = argv[ 0 ];
  
  // 声明信号以及子进程相关变量和结构体
  pid_t pid;
  pid_t child_pid[ worker_num ];
  struct sigaction sa_struct;
  sa_struct.sa_flags   = SA_RESTART;
  sa_struct.sa_handler = signal_handler;
  //  声明socket相关变量 和 结构体
  int listen_socket; 
  int connect_socket;
  int socket_option_value;
  socklen_t socket_length;
  struct sockaddr_in listen_socket_addr;
  struct sockaddr_in connect_socket_addr;
  char buffer[ MAXBUFFER ];
  // 声明配置文件路径
  int  file_size;  // bytes
  FILE * conf_file_fp;
  char * conf_file = "./conf/tiginx.conf";
  char * conf_content;
  cJSON * conf_content_root;
  // 声明配置项目相关变量
  cJSON * conf_port;
  cJSON * conf_worker_num;
  
  // 解析配置文件
  conf_file_fp = fopen( conf_file, "r" );  
  if ( !conf_file_fp ) {
    printf( "open file error.\n" );
    exit( -1 );
  }
  fseek( conf_file_fp, 0, SEEK_END ); 
  file_size = ftell( conf_file_fp );
  rewind( conf_file_fp );
  conf_content = ( char * )malloc( 10000 );   
  memset( conf_content, 0, sizeof( conf_content ) );
  fread( conf_content, sizeof( char ), file_size, conf_file_fp );
  free( conf_content );
  fclose( conf_file_fp );
  conf_content_root = cJSON_Parse( conf_content ); 
  conf_port       = cJSON_GetObjectItem( conf_content_root, "port" );
  conf_worker_num = cJSON_GetObjectItem( conf_content_root, "worker_num" );
  //printf( "%d\n", conf_worker_num->valueint );   
  port       = conf_port->valueint;
  worker_num = conf_worker_num->valueint; 
  // 创建监听socket  
  listen_socket = socket( AF_INET, SOCK_STREAM, 0 );
  if ( -1 == listen_socket ) {
    printf( "create listen socket error.\n" );
    exit( -1 );
  }
  socket_option_value = 1;
  setsockopt( listen_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option_value, sizeof( int ) );
  memset( &listen_socket_addr, 0, sizeof( struct sockaddr_in ) );
  listen_socket_addr.sin_family = AF_INET; 
  listen_socket_addr.sin_port   = htons( port ); 
  listen_socket_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  if ( -1 == bind( listen_socket, ( struct sockaddr * )&listen_socket_addr, sizeof( listen_socket_addr ) ) ) {
    printf( "bind socket error.\n" );
    exit( -1 );
  }
  if ( -1 == listen( listen_socket, 128 ) ) {
    printf( "listen socket error.\n" );
    exit( -1 );
  } 
  /*
  while ( 1 ) {
    connect_socket = accept( listen_socket, ( struct sockaddr * )&connect_socket_addr, &socket_length );
    if ( -1 == connect_socket ) {
      printf( "accpet socket error.\n" );
      exit( -1 );
    }
    recv( connect_socket, &buffer, MAXBUFFER, 0 ); 
    printf( "%s\n", buffer ); 
    char msg[ MAXBUFFER ] = "hello,nihao";
    send( connect_socket, &msg, MAXBUFFER, 0 );
    close( connect_socket );
  } 
  close( listen_socket );
  return 0;
  */
 
  // fork子进程
  for ( int i = 0; i < worker_num; i++ ) {
    pid = fork(); 
    // fork error.
    if ( 0 > pid ) {
      printf( "fork error.\n" );
      exit( -1 ); 
    }
    // 在子进程中.
    else if ( 0 == pid ) {
      //sleep( 10 );
      //exit( -1 );
      while ( 1 ) {
        connect_socket = accept( listen_socket, ( struct sockaddr * )&connect_socket_addr, &socket_length ); 
        recv( connect_socket, &buffer, MAXBUFFER, 0 );
        printf( "进程%d : %s", getpid(), buffer );
        close( connect_socket );
      }
    }
    // 在父进程中.
    else if ( 0 < pid ) {
      //printf( "fork.\n" );
      child_pid[ i ] = pid; 
    }
  }

  // 为主进程安装信号管理器
  sigaction( SIGCHLD, &sa_struct, NULL );

  // 主进程要回收子进程，防止僵尸进程.
  //while ( 1 ) {
    //sleep( 1 );
    //for ( int i = 0; i < worker_num; i++ ) {
      //int wait_status_value;
      //waitpid( child_pid[ i ], &wait_status_value, WNOHANG );
      //if (WIFEXITED(stat_val))//正常退出
      //if (WIFSIGNALED(stat_val))//查看被什么信号关闭
    //}
  //}
  //sleep( 1000000 );
  while ( 1 ) {
    sleep( 1 );
  }

  /*
  int worker_num_total = sizeof( child_pid ) / sizeof( pid_t );
  for ( int i = 0; i < worker_num_total ; i++ ) {
    printf( "%d\n", child_pid[ i ] );
  } 
  */
  return 0;
}

void signal_handler( int signal ) {
  // 子进程exit时候，内核会给父进程发送SIGCHLD信号，主进程回收子进程
  if ( SIGCHLD == signal ) {
    int   waitpid_status;
    pid_t pid;
    pid = waitpid( -1, &waitpid_status, WNOHANG );   
  }
}
