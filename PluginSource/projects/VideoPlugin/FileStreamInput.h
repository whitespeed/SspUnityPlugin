#pragma once

extern "C" {
#include <libavutil\error.h>
#include <libavformat\avio.h>
#include <libavformat\avformat.h>
}

#include <stdio.h>
#include <string>
#include "IStreamInput.h"
class FileStreamInput:public virtual IStreamInput
{
public:
	std::string mPath;
	
	FILE *mFileHandler;
public:
	FileStreamInput();
	virtual ~FileStreamInput();

	bool initAVFormatContext(char * path);
};

static int IOReadFunc(void *data, uint8_t *buf, int buf_size) {
	FileStreamInput *hctx = (FileStreamInput*)data;
	size_t len = fread(buf, 1, buf_size, hctx->mFileHandler);
	if (len == 0) {
		// Let FFmpeg know that we have reached EOF, or do something else
		return AVERROR_EOF;
	}
	return (int)len;
}

// whence: SEEK_SET, SEEK_CUR, SEEK_END (like fseek) and AVSEEK_SIZE
static int64_t IOSeekFunc(void *data, int64_t pos, int whence) {
	if (whence == AVSEEK_SIZE) {
		// return the file size if you wish to
	}
	FileStreamInput *hctx = (FileStreamInput*)data;
	long fpos = ftell(hctx->mFileHandler); // int64_t is usually long long
	if (whence > fpos)
	{
		return -1;
	}
	int rs = fseek(hctx->mFileHandler, (long)pos, whence);
	if (rs != 0) {
		return -1;
	}
	fpos = ftell(hctx->mFileHandler); // int64_t is usually long long
	return (int64_t)fpos;
}