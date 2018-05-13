//========= Copyright 2015-2018, HTC Corporation. All rights reserved. ===========

#pragma once
#include <stdio.h>
#include <stdarg.h>
#include "stdafx.h"
#include<windows.h>

#define ENABLE_LOG
#ifdef ENABLE_LOG
#define LOG(...) Logger::instance()->log(__VA_ARGS__)
#else
#define LOG
#endif
typedef void(__stdcall * UnityLog) (const char * str);
class Logger {
public:
	static Logger* instance();
	void log(const char* str, ...);
	static UnityLog _unity;
protected:
	virtual ~Logger();
private:
	Logger();
	static Logger* _instance;
	FILE* file;
};