#include "src/core/main.h"

// 定义信号处理函数
void signal_handler( int );
// 定义处理进程名称的函数
void set_process_title();
// daemonize函数
void be_daemon();
// 配置文件解析函数
static void parse_conf_file( void );
// select-loop
void select_loop( int );
extern void epoll_loop( int );

// 子进程数量
int worker_num = 4;
int port       = 6666;
int daemonize  = 0;
char * event;

// 环境变量数组 
extern char ** environ;

int main( int argc, char * argv[] ) {
  char * process_title = argv[ 0 ];
  
  // 声明信号以及子进程相关变量和结构体
  int i;
  pid_t pid;
  pid_t * child_pid;
  struct sigaction sa_struct;
  sa_struct.sa_flags   = SA_RESTART;
  sa_struct.sa_handler = signal_handler;
  //  声明socket相关变量 和 结构体
  int listen_socket_fd; 
  int socket_option_value;
  struct sockaddr_in listen_socket_addr;

  // 解析json配置文件
  parse_conf_file();

  child_pid = (pid_t *)malloc(sizeof(pid_t) * worker_num);
  if (child_pid == NULL) {
    printf("malloc error\n");
    exit( -1 );
  }
     
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
  for ( i = 0; i < worker_num; i++ ) {
    pid = fork(); 
    // fork error.
    if ( 0 > pid ) {
      printf( "fork error.\n" );
      exit( -1 ); 
    }
    // 在子进程中.
    else if ( 0 == pid ) {
      if ( 0 == strcmp( "select", event ) ) {
        select_loop( listen_socket_fd );
      }
      else if ( 0 == strcmp( "epoll", event ) ) {
        epoll_loop( listen_socket_fd );
      }
      else {
        printf( "event loop : select , others not support!\n" );
        exit( -1 );
      }
    }
    // 在父进程中.
    else if ( 0 < pid ) {
      child_pid[ i ] = pid; 
    }
  }

  // 为主进程安装信号管理器
  sigaction( SIGCHLD, &sa_struct, NULL );
  // 父进程进入无限循环
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

/*
 * @desc : 将程序daemon化
 */
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

/*
 * @desc : 解析json配置文件的函数
 */
static void parse_conf_file( void ) {
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
  cJSON * conf_event;
  // 解析配置文件
  worker_dir = getcwd( NULL, 0 );
  if (worker_dir == NULL) {
    printf("getcwd error\n");
    exit(-1);
  }
  full_conf_file = ( char * )malloc( sizeof( worker_dir ) + sizeof( conf_file ) + 50 );
  if (full_conf_file == NULL) {
    printf("malloc error\n");
    exit(-1);
  }
  bzero(full_conf_file, sizeof( worker_dir ) + sizeof( conf_file ) + 50);
  strcat( full_conf_file, worker_dir );
  strcat( full_conf_file, conf_file );
  conf_file_fp = fopen( full_conf_file, "r" );  
  if ( !conf_file_fp ) {
    printf( "open file error.\n" );
    exit( -1 );
  }
  fseek( conf_file_fp, 0, SEEK_END ); 
  file_size = ftell( conf_file_fp );
  if (file_size == -1) {
    printf("ftell error\n");
    exit(-1);
  }
  rewind( conf_file_fp );
  conf_content = ( char * )malloc( file_size );   
  if (conf_content == NULL) {
    printf("malloc error\n");
    exit(-1);
  }
  memset( conf_content, 0, sizeof( conf_content ) );
  fread( conf_content, sizeof( char ), file_size, conf_file_fp );
  conf_content_root = cJSON_Parse( conf_content ); 
  conf_port       = cJSON_GetObjectItem( conf_content_root, "port" );
  conf_worker_num = cJSON_GetObjectItem( conf_content_root, "worker_num" );
  conf_daemonize  = cJSON_GetObjectItem( conf_content_root, "daemonize" );
  conf_event      = cJSON_GetObjectItem( conf_content_root, "event" );
  // 端口
  port       = conf_port->valueint;
  // 子进程数量
  worker_num = conf_worker_num->valueint;
  // 是否daemon运行
  daemonize  = conf_daemonize->valueint; 
  // 事件模型
  event = ( char * )malloc( sizeof( conf_event->valuestring ) );
  if (event == NULL) {
    printf("malloc error\n");
    exit( -1 );
  }
  bzero( event, sizeof( conf_event->valuestring ));
  strcpy( event, conf_event->valuestring );
  free( worker_dir );
  free( full_conf_file );
  free( conf_content );
  free( conf_content_root );
  fclose( conf_file_fp );
}
