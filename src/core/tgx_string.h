#include "main.h"

typedef struct tgx_string_struct {
  char * str;      // 指向字符串内容的指针
  size_t length;   // 字符串长度 
} tgx_string_t;

// 将字符串转换为tgx字符串
tgx_string_t * to_tgx_string( char * );

// 对比两个tgx字符串
int tgx_strcmp( const tgx_string_t *, const tgx_string_t * );

// 获取一个tgx字符串的长度
size_t tgx_strlen( const tgx_string_t * );

// 拼接两个tgx字符串
void tgx_strcat( tgx_string_t *, tgx_string_t * );
