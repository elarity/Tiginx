#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();

// 子进程数量
const int worker_num = 4;

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
