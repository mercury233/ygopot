#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

#include <windows.h>

class Thread {
public:
	static bool �˳�����;
	static void NewThread(int (*thread_func)(void*), void* param) {
		if( CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_func, param, 0, 0) )
			������++;
	}
	static int ������;
};

#else // _WIN32

#include <pthread.h>

class Thread {
public:
	static void NewThread(int (*thread_func)(void*), void* param) {
		pthread_t thread;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&thread, &attr, (void * (*)(void *))thread_func, param);
	}
};

#endif // _WIN32

#endif // THREAD_H
