#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


using namespace std;

// ������Ϣ
void error_msg(int return_code, char* msg);
// �ػ�����demo
void daemon();