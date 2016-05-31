#ifndef FD_OPERATOR_H_
#define FD_OPERATOR_H_

#define FILE_SLICE_LENGTH 1024*64
#define BUFFER_HIGH_WATER_LEVEL_THRESHOLD FILE_SLICE_LENGTH * 64

#include <fcntl.h>
#include <stdint.h>
#include <string>
#include <atomic>
#include <memory>
#include <map>
#include <set>
#include <muduo/base/Logging.h>
#include "StringBuffer.h"

using namespace std;

typedef enum WHENCE{FD_SEEK_SET, FD_SEEK_CUR, FD_SEEK_END}WHENCE;
typedef enum FDMODE
{
	READ_MODE = O_RDONLY|O_NONBLOCK,
	WRITE_MODE = O_WRONLY|O_NONBLOCK|O_CREAT,
	WRITE_APPEND = O_WRONLY|O_NONBLOCK|O_APPEND|O_CREAT,
}FDMODE;

class FdOperatorManager;
class FdOperator;
class FdLoop;
typedef map<std::string, std::set<std::shared_ptr<FdOperator> > > FDMAP;

class FdOperator:public std::enable_shared_from_this<FdOperator>
{
	friend class FdOperatorManager;
public:
	explicit FdOperator(std::shared_ptr<FdLoop> loop_ptr);
	~FdOperator();
	int getFd() const {return fd_;}
	int getEvents() const {return events_;}
	void setRevents(int events){revents_ = events;}
	void handleEvents();

	bool openFd(const std::string& filePath, FDMODE mode);
	long seekFd(uint64_t offset, WHENCE whence);
	void readFd(uint64_t length, std::function<void(std::string&, bool)> fileReadFinishCb);
	void writeFd(std::string& data, std::function<void(bool)> fileWriteFinishCb);
	void closeFd();
	void registerFd();
	void flushFd();

private:
	trantor::StringBuffer output_buffer_;
	trantor::StringBuffer input_buffer_;
	int fd_;
	int events_;
	int revents_;
	std::shared_ptr<FdLoop> loop_;
	FDMODE fd_mode_;
	std::function<void(std::string&, bool)> file_read_finish_cb_;
	std::function<void(bool)> file_write_finish_cb_;
	bool file_read_eof_;
	bool file_write_end_;
	std::atomic<bool> registered_to_poll_;
	std::string file_path_;
	std::string file_write_data_;
	uint64_t file_read_length_;

	static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

	void clearFd();
	void enableReading() { events_ |= kReadEvent; }
	void disableReading() { events_ &= ~kReadEvent; }
	void enableWriting() { events_ |= kWriteEvent; }
	void disableWriting() { events_ &= ~kWriteEvent; }
	void handleRead();
	void handleWrite();
};

#endif
