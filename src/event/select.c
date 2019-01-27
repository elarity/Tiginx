#include "../core/main.h"
#include "select.h"

void select_loop( int listen_socket_fd ) {
  // 声明select相关变量
  int i;
  fd_set temp_fd;
  fd_set read_fd; 
  int affected_fd_num;
  int max_fd_num; 
  int client_array[ MAX_FD_SIZE ];
  int connect_socket_fd;
  struct timeval timeout;
  struct sockaddr_in connect_socket_addr;
  socklen_t socket_length;
  char buffer[ MAXBUFFER ];

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
	for ( i = 0; i < MAX_FD_SIZE; i++ ) {
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
 	for ( i = 0; i < MAX_FD_SIZE; i++ ) {
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
  }
}
