#pragma once
#include <list>
#include <mutex>
#include "imf/net/loop.h"
#include "imf/net/threadloop.h"
#include "imf/ssp/sspclient.h"
#include "CameraInput.h"
#include <string>

extern "C" {
#include <libavformat\avformat.h>
#include <libswresample\swresample.h>
#include <libavutil\pixdesc.h>
#include <libswscale\swscale.h>
}

struct H264Data
{
	uint8_t* data;
	size_t len;
	uint32_t type;
	uint32_t pts;
	uint32_t frm_no;
	virtual ~H264Data() {
		len = 0;
		frm_no = -1;
		type = -1;
		pts = -1;
		if (data != NULL)
			delete[] data;
		data = NULL;
	}
}; H264Data;

class H264Queue
{
public:
	H264Queue()
	{
		
	}
	~H264Queue()
	{
		release();
	}
	void queue(H264Data* data)
	{
		std::lock_guard<std::mutex> lock(mOpMutex);
		mDataList.push_back(data);
	}
	H264Data* dequeue()
	{
		std::lock_guard<std::mutex> lock(mOpMutex);
		if (mDataList.size() <= 0)
			return NULL;

		H264Data* data = mDataList.front();
		mDataList.pop_front();
		return data;
	}
	size_t size()
	{
		std::lock_guard<std::mutex> lock(mOpMutex);
		return mDataList.size();
	}
	void release()
	{
		std::lock_guard<std::mutex> lock(mOpMutex);
		if (!mDataList.empty())
			mDataList.clear();
	}
private:
	std::list<H264Data*> mDataList;
	std::mutex mOpMutex;
};

class FFMpegDecoder : ICodec
{

	enum BufferState {EMPTY, NORMAL, FULL};

	struct VideoInfo {
		bool isEnabled;
		int width;
		int height;
		double lastTime;
		double totalTime;
		BufferState bufferState;
	};

	struct AudioInfo {
		bool isEnabled;
		unsigned int channels;
		unsigned int sampleRate;
		double lastTime;
		double totalTime;
		BufferState bufferState;
	};

public:
	FFMpegDecoder();
	~FFMpegDecoder();

	bool Init(char &filePath) = 0;
	bool Decode() = 0;
	void Seek(double time) = 0;
	void Destroy() = 0;

	VideoInfo getVideoInfo();
	AudioInfo getAudioInfo();
	void setVideoEnable(bool isEnable);
	void setAudioAllChDataEnable(bool isEnable);
	double	getVideoFrame(unsigned char** outputY, unsigned char** outputU, unsigned char** outputV);
	double	getAudioFrame(unsigned char** outputFrame, int& frameSize);
	void freeVideoFrame();
	void freeAudioFrame();

	int getMetaData(char**& key, char**& value);

private:
	bool mIsInitialized;
	bool mIsAudioAllChEnabled;
	bool mUseTCP;				//	For RTSP stream.
	bool mIsConnected;
	bool mIsSeekToAny;
	int64_t mDtsIndex;

	std::mutex mVideoMutex;
	std::mutex mAudioMutex;

	int mFrameBufferNum;

	AVFormatContext* mAVFormatContext;
	AVStream*		mVideoStream;
	AVStream*		mAudioStream;
	AVCodec*		mVideoCodec;
	AVCodec*		mAudioCodec;
	AVCodecContext*	mVideoCodecContext;
	AVCodecContext*	mAudioCodecContext;

	SwrContext*	mSwrContext;
	SwsContext*   mSwsContext;
	imf::SspClient * mSspClient;
	imf::ThreadLoop * mThreadLooper;

	AVPacket	mPacket;
	//std::list<AVFrame*> mVideoFrames;
	//std::list<AVFrame*> mAudioFrames;
	unsigned int mVideoBuffMax;
	unsigned int mAudioBuffMax;

	std::string mUrl;
	imf::SspVideoMeta mVideoMeta;
	imf::SspAudioMeta mAudioMeta;
	imf::SspAudioMeta mSSpMeta;

	VideoInfo		mVideoInfo;
	AudioInfo	mAudioInfo;

	//TODO: support audio. add audio sync machanic
	//int initSwrContext();  //InitAudioInputContext

	AVFrame* convertToYUV420P(AVFrame* src);

	void updateBufferState();
	bool isH264QueueReady();
	bool isBuffBlocked();
	void freeFrontFrame(std::list<AVFrame*>* frameBuff, std::mutex* mutex);
	void flushBuffer(std::list<AVFrame*>* frameBuff, std::mutex* mutex);
	int	loadConfig();
	void printErrorMsg(int errorCode);
	void setup(imf::Loop *loop, const char* ip);
	void on_264(uint8_t * data, size_t len, uint64_t pts, uint32_t frm_no, uint32_t type);
	void on_meta(struct imf::SspVideoMeta *v, struct imf::SspAudioMeta *a, struct imf::SspMeta *);
	void on_disconnect();
};