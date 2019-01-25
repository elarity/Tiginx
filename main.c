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
#include <sys/select.h>
#include "src/core/cJSON.h"

#define MAXBUFFER 8192
#define MAX_FD_SIZE 1024

// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();
// daemonize函数
void be_daemon();

// 子进程数量
int worker_num = 4;
int port       = 6666;
int daemonize  = 0;

// 环境变量数组 
extern char ** environ;

static void parse_conf_file(void);

int main( int argc, char * argv[] ) {
  char * process_title = argv[ 0 ];
  
  // 声明信号以及子进程相关变量和结构体
  pid_t pid;
  pid_t child_pid[ worker_num ];
  struct sigaction sa_struct;
  sa_struct.sa_flags   = SA_RESTART;
  sa_struct.sa_handler = signal_handler;
  //  声明socket相关变量 和 结构体
  int listen_socket_fd; 
  int connect_socket_fd;
  int socket_option_value;
  socklen_t socket_length;
  struct sockaddr_in listen_socket_addr;
  struct sockaddr_in connect_socket_addr;
  char buffer[ MAXBUFFER ];
  // 声明select相关变量
  fd_set temp_fd;
  fd_set read_fd; 
  int affected_fd_num;
  int max_fd_num; 
  int client_array[ MAX_FD_SIZE ];
  struct timeval timeout;

  // 解析json配置文件
  parse_conf_file();
  
  if ( 1 == daemonize ) {
    be_daemon();
  }
  // 创建监听socket  
  listen_socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( -1 == listen_socket_fd ) {
    printf( "create listen socket error.\n" );
    exit( -1 );
  }
  socket_option_value = 1;
  setsockopt( listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option_value, sizeof( int ) );
  memset( &listen_socket_addr, 0, sizeof( struct sockaddr_in ) );
  listen_socket_addr.sin_family = AF_INET; 
  listen_socket_addr.sin_port   = htons( port ); 
  listen_socket_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  if ( -1 == bind( listen_socket_fd, ( struct sockaddr * )&listen_socket_addr, sizeof( listen_socket_addr ) ) ) {
    printf( "bind socket error.\n" );
    exit( -1 );
  }
  if ( -1 == listen( listen_socket_fd, 128 ) ) {
    printf( "listen socket error.\n" );
    exit( -1 );
  } 
 
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

      /*
      while ( 1 ) {
        connect_socket_fd = accept( listen_socket_fd, ( struct sockaddr * )&connect_socket_addr, &socket_length ); 
        recv( connect_socket_fd, &buffer, MAXBUFFER, 0 );
        printf( "进程%d : %s", getpid(), buffer );
        close( connect_socket_fd );
      } 
      */

      // BEGIN ----- select ----- BEGIN
      timeout.tv_sec  = 2;
      timeout.tv_usec = 0;
      for ( int i = 0; i < MAX_FD_SIZE; i++ ) {
        client_array[ i ] = -1;
      }
      // 首先清空read_fd和temp_fd，其次将listen_socket_fd加入到read_fd中
      FD_ZERO( &read_fd );
      FD_ZERO( &temp_fd );
      FD_SET( listen_socket_fd, &read_fd ); 
      max_fd_num = listen_socket_fd + 1; 
      while ( 1 ) {
        //printf( "max_fd_num : %d , loop.\n", max_fd_num );
        temp_fd = read_fd;
        // 每个子进程中维护一个select多路复用器
        //affected_fd_num = select( listen_socket_fd, &temp_fd, NULL, NULL, &timeout ); 
        affected_fd_num = select( max_fd_num, &temp_fd, NULL, NULL, NULL ); 
        printf( "affeceted_fd_num : %d\n", affected_fd_num );
        if ( -1 == affected_fd_num ) {
 	  printf( "select error.\n" );
	  exit( -1 );
	}
	// 如果有发生变化的fd
  	if ( 0 < affected_fd_num ) {
          // 首先listen_socket_fd是否属于变化的fd，如果是，动作应该为accept 
 	  if ( FD_ISSET( listen_socket_fd, &temp_fd ) ) {
	    //printf( "accept connect.\n" );
	    socket_length = sizeof( connect_socket_addr );
	    connect_socket_fd = accept( listen_socket_fd, ( struct sockaddr * )&connect_socket_addr, &socket_length );
 	    FD_SET( connect_socket_fd, &read_fd );
	    for ( int i = 0; i < MAX_FD_SIZE; i++ ) {
	      if ( -1 == client_array[ i ] ) {
	        if ( i <= ( MAX_FD_SIZE - 1 ) ) {
                  printf( "accept : %d\n", i );
                  max_fd_num++; 
                  client_array[ i ] = connect_socket_fd; 
		  break;
		}
	      }
	    } 
 	  }
          // 如果不是listen_socket_fd，那么动作应该为recv/send
	  else {
	    printf( "recv read.\n" );
            // 先用傻逼轮训办法读取
 	    for ( int i = 0; i < MAX_FD_SIZE; i++ ) {
	      connect_socket_fd = client_array[ i ];
 	      if ( 0 < connect_socket_fd && FD_ISSET( connect_socket_fd, &temp_fd ) ) {
	        recv( connect_socket_fd, buffer, MAXBUFFER, 0 );	
 		printf( "%s\n", buffer );
	        close( connect_socket_fd );
		FD_CLR( connect_socket_fd, &read_fd );
 		client_array[ i ] = -1;
  	      } 
	      else {
		continue;
	      }
	    }
	  }
	}
	// 如果没有任何发生变化的fd
        else if ( 0 == affected_fd_num ) {
	  sleep( 1 );
 	  continue;
        }
        // END ---- select ---- END


        // 下面注释的四行是原来的accept逻辑
 	/*
        connect_socket = accept( listen_socket, ( struct sockaddr * )&connect_socket_addr, &socket_length ); 
        recv( connect_socket, &buffer, MAXBUFFER, 0 );
        printf( "进程%d : %s", getpid(), buffer );
        close( connect_socket );
	*/
      }
    }
    // 在父进程中.
    else if ( 0 < pid ) {
      child_pid[ i ] = pid; 
    }
  }

  // 为主进程安装信号管理器
  sigaction( SIGCHLD, &sa_struct, NULL );

  while ( 1 ) {
    sleep( 1 );
  }

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

void be_daemon() {

  pid_t  pid; 
  struct sigaction sa;
  int    i;
  int    open_fd_max_num;
  int    fd0, fd1, fd2;
  char * worker_dir = NULL;
  
  pid = fork();    
  if ( 0 > pid ) {
    printf( "fork error\n" );
    exit( -1 );
  }  
  else if ( 0 < pid ) {
    exit( -1 );
  }
  setsid();
  pid = fork();
  if ( 0 > pid ) {
    printf( "fork error\n" );
    exit( -1 );
  } 
  else if ( 0 < pid ) {
    exit( -1 );
  } 
  // 设置umask ， 给进程最大的创建文件夹以及文件权限  
  umask( 0 );
  // 将目录更改为根目录，防止挂载目录无法卸载
  worker_dir = getcwd( NULL, 0 );
  chdir( worker_dir );
  free( worker_dir );
  // 忽略sighup信号
  sa.sa_handler = SIG_IGN;
  sigemptyset( &sa.sa_mask );   
  sa.sa_flags  = 0;
  sigaction( SIGHUP, &sa, NULL ); 
  // 关闭父进程继承过来的文件描述
  open_fd_max_num = sysconf( _SC_OPEN_MAX ); 
  open_fd_max_num = RLIM_INFINITY == open_fd_max_num ? 1024 : open_fd_max_num ;
  for ( i = 0; i < open_fd_max_num; i++ ) {
    //close( i );
  }
  // 重定向标准输入输出
  //fd0 = open( "/dev/null", O_RDWR );   
  //fd1 = dup( 0 );
  //fd2 = dup( 0 );
  //printf( "daemonize over\n" );
}

static void parse_conf_file(void)
{
  // 声明配置文件路径
  int  file_size;  // bytes
  FILE * conf_file_fp;
  char * worker_dir = NULL;
  char * conf_file = "/conf/tiginx.conf";
  char * full_conf_file = NULL;
  char * conf_content;
  cJSON * conf_content_root;
  // 声明配置项目相关变量
  cJSON * conf_port;
  cJSON * conf_worker_num;
  cJSON * conf_daemonize;

  // 解析配置文件
  worker_dir = getcwd( NULL, 0 ); 
  full_conf_file = ( char * )malloc( sizeof( worker_dir ) + sizeof( conf_file ) + 50 );
  bzero(full_conf_file, sizeof( worker_dir ) + sizeof( conf_file ) + 50);
  strcat( full_conf_file, worker_dir );
  free( worker_dir );
  strcat( full_conf_file, conf_file );
  conf_file_fp = fopen( full_conf_file, "r" );  
  free( full_conf_file );
  if ( !conf_file_fp ) {
    printf( "open file error.\n" );
    exit( -1 );
  }
  fseek( conf_file_fp, 0, SEEK_END ); 
  file_size = ftell( conf_file_fp );
  rewind( conf_file_fp );
  conf_content = ( char * )malloc( file_size );   
  memset( conf_content, 0, sizeof( conf_content ) );
  fread( conf_content, sizeof( char ), file_size, conf_file_fp );
  fclose( conf_file_fp );
  conf_content_root = cJSON_Parse( conf_content ); 
  conf_port       = cJSON_GetObjectItem( conf_content_root, "port" );
  conf_worker_num = cJSON_GetObjectItem( conf_content_root, "worker_num" );
  conf_daemonize  = cJSON_GetObjectItem( conf_content_root, "daemonize" );
  port       = conf_port->valueint;
  worker_num = conf_worker_num->valueint; 
  daemonize  = conf_daemonize->valueint; 
  free( conf_content );
}
