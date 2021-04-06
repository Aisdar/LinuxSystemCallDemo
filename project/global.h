#pragma once
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// fork的错误信息打印
void fork_error_msg(int return_code, const char* msg);

// open函数的错误信息打印
void open_error_msg(int return_code, const char* msg);

// close函数的错误信息打印
void close_error_msg(int return_code, const char* msg);