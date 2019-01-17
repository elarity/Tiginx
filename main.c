#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

//void signal_handler(  );

// 子进程数量
const int worker_num = 4;

int main( int argc, char * argv[] ) {
  
  pid_t pid;
  // 用child_pid数组用来保存子进程的进程id
  pid_t child_pid[ worker_num ];
 
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
      sleep( 20 );
      exit( -1 );
    }
    // 在父进程中.
    else if ( 0 < pid ) {
      //printf( "fork.\n" );
      child_pid[ i ] = pid; 
    }
  }

  // 主进程要回收子进程，防止僵尸进程.
  while ( 1 ) {
    sleep( 1 );
    for ( int i = 0; i < worker_num; i++ ) {
      int wait_status_value;
      waitpid( child_pid[ i ], &wait_status_value, WNOHANG );
      //if (WIFEXITED(stat_val))//正常退出
      //if (WIFSIGNALED(stat_val))//查看被什么信号关闭
    }
  }

  // 为祝进程安装信号管理器
  //sigaction(  );
  /*
  int worker_num_total = sizeof( child_pid ) / sizeof( pid_t );
  for ( int i = 0; i < worker_num_total ; i++ ) {
    printf( "%d\n", child_pid[ i ] );
  } 
  */
  return 0;
}

//void sigaction() {}
