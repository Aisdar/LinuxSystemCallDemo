#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


using namespace std;

// 错误信息
void error_msg(int return_code, char* msg);
// 守护进程demo
void daemon();