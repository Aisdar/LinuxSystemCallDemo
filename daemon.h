#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


using namespace std;

void error_msg(int return_code, char* msg) {
	if (return_code < 0) {
		perror(msg);
		exit(-1);
	}
}

// ÊØ»¤½ø³Ìdemo
void daemon();