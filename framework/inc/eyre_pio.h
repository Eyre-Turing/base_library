#ifndef EYRE_PIO_H
#define EYRE_PIO_H

#ifdef linux

#include <string>
#include <queue>
#include <pthread.h>

class PIO
{
public:
	typedef void (*OutputMessage)(std::string message);
	PIO();
	virtual ~PIO();
	
	bool terminal_create(void (*callback)(void *), void *arg);
	bool terminal_create();
	bool terminal_create(const char *path, char *const argv[]);
	bool terminal_create(const char *path, char *const argv[], char *const envp[]);
	bool terminal_close();
	
	void input_command(const std::string &command);
	void set_output_callback(OutputMessage output);
	
	class Thread
	{
	public:
		static void *terminal_input(void *p);
		static void *terminal_output(void *p);
	};
	
private:
	OutputMessage m_output_message;	// 回调，得到从终端消息
	pthread_t t_input;			// 主终端输入线程
	pthread_t t_output;			// 主终端输出线程
	std::queue<std::string> d_input;	// 主终端给从终端发送信息队列
	pthread_mutex_t md_input;		// 主终端发送信息队列线程锁
	int master_fd;		// 主终端句柄
	std::string slave_name;	// 从终端设备名
	pid_t slave_pid;		// 从终端PID
	
	bool is_open;	// 虚拟终端是否已经开启
	
	bool create_pty();
};

#endif

#endif
