#pragma once
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// fork�Ĵ�����Ϣ��ӡ
void fork_error_msg(int return_code, const char* msg);

// open�����Ĵ�����Ϣ��ӡ
void open_error_msg(int return_code, const char* msg);

// close�����Ĵ�����Ϣ��ӡ
void close_error_msg(int return_code, const char* msg);