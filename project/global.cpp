#include "global.h"



void fork_error_msg(int return_code, const char* msg) {
	if (return_code < 0) {
		perror(msg);
		exit(-1);
	}
}


void open_error_msg(int return_code, const char* msg) {
	if (return_code < 0) {
		perror(msg);
		exit(-1);
	}
}

void close_error_msg(int return_code, const char* msg) {
	if (0 != return_code) {
		perror(msg);
		exit(-1);
	}
}