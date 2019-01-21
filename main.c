#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();

// 子进程数量
const int worker_num = 4;

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
  
  pid_t pid;
  // 用child_pid数组用来保存子进程的进程id
  pid_t child_pid[ worker_num ];
  // 声明sigaction-struct结构体变量
  struct sigaction sa_struct;
  sa_struct.sa_flags   = SA_RESTART;
  sa_struct.sa_handler = signal_handler;
  // 
  int listen_socket; 
  int connect_socket;
  int socket_option_value;
  socklen_t socket_length;
  struct sockaddr_in listen_socket_addr;
  struct sockaddr_in connect_socket_addr;
  char buffer[ 4096 ];
  
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
  listen_socket_addr.sin_port   = htons( 6666 ); 
  listen_socket_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  if ( -1 == bind( listen_socket, ( struct sockaddr * )&listen_socket_addr, sizeof( listen_socket_addr ) ) ) {
    printf( "bind socket error.\n" );
    exit( -1 );
  }
  if ( -1 == listen( listen_socket, 128 ) ) {
    printf( "listen socket error.\n" );
    exit( -1 );
  } 
  while ( 1 ) {
    connect_socket = accept( listen_socket, ( struct sockaddr * )&connect_socket_addr, &socket_length );
    if ( -1 == connect_socket ) {
      printf( "accpet socket error.\n" );
      exit( -1 );
    }
    recv( connect_socket, &buffer, 4096, 0 ); 
    printf( "%s\n", buffer ); 
    char msg[ 4096 ] = "hello,nihao";
    send( connect_socket, &msg, 4096, 0 );
    close( connect_socket );
  } 
  close( listen_socket );
  return 0;
 
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
      sleep( 10 );
      exit( -1 );
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
