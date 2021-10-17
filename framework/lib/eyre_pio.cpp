#ifdef linux

#include "eyre_pio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

void *PIO::Thread::terminal_input(void *p)
{
	PIO *pio = (PIO *) p;
	
	// 开启主终端接收从终端输出的线程
	pthread_create(&(pio->t_output), NULL, PIO::Thread::terminal_output, p);
	
	while (pio->is_open)
	{
		pthread_mutex_lock(&(pio->md_input));
		while (!pio->d_input.empty())
		{
			write(pio->master_fd, pio->d_input.front().data(), pio->d_input.front().size());
			pio->d_input.pop();
		}
		pthread_mutex_unlock(&(pio->md_input));
	}
	
	return NULL;
}

void *PIO::Thread::terminal_output(void *p)
{
	PIO *pio = (PIO *) p;
	
	char toutput[1024];
	while (pio->is_open)
	{
		int nread = read(pio->master_fd, toutput, 1023);
		if (nread <= 0)	// 从终端被关闭
		{
			break;
		}
		toutput[nread] = '\0';
		if (pio->m_output_message)
		{
			pio->m_output_message(string(toutput));
		}
	}
	
	return NULL;
}

PIO::PIO()
{
	m_output_message = NULL;
	is_open = false;
	
	// 线程锁初始化
	pthread_mutex_init(&md_input, NULL);
}

PIO::~PIO()
{
	terminal_close();
	
	// 线程锁销毁
	pthread_mutex_destroy(&md_input);
}

void PIO::set_output_callback(OutputMessage output)
{
	m_output_message = output;
}

/*
 * @brief 创建pty主终端
 * @return bool，成功返回true，失败返回false（之前已经打开也会返回false）
 */
bool PIO::create_pty()
{
	if (is_open)
	{
		return false;
	}
	
	master_fd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
	if (master_fd == -1)
	{
		return false;
	}
	if (grantpt(master_fd) == -1)
	{
		close(master_fd);
		return false;
	}
	if (unlockpt(master_fd) == -1)
	{
		close(master_fd);
		return false;
	}
	char *ptyname = ptsname(master_fd);
	if (ptyname == NULL)
	{
		close(master_fd);
		return false;
	}
	slave_name = ptyname;
	return true;
}

namespace eyre_pio
{
	struct exec_args
	{
		char *path;
		char **argv;
		char **envp;
	};
}

static void open_app_with_local_env(void *p)
{
	eyre_pio::exec_args *args = (eyre_pio::exec_args *) p;
	execv(args->path, args->argv);
}

static void open_app_appoint_env(void *p)
{
	eyre_pio::exec_args *args = (eyre_pio::exec_args *) p;
	execve(args->path, args->argv, args->envp);
}

bool PIO::terminal_create()
{
	char *args[] = {"bash", NULL};
	return terminal_create("/bin/bash", args);
}

bool PIO::terminal_create(const char *path, char *const argv[])
{
	eyre_pio::exec_args args;
	args.path = strdup(path);
	size_t len;
	for (len=0; argv[len] != NULL; ++len);
	args.argv = (char **) malloc((len+1)*sizeof(char *));
	for (int i=0; i<len; ++i)
	{
		args.argv[i] = strdup(argv[i]);
	}
	args.argv[len] = NULL;
	args.envp = NULL;
	bool ret = terminal_create(open_app_with_local_env, (void *) &args);
	free(args.path);
	for (int i=0; i<len; ++i)
	{
		free(args.argv[i]);
	}
	free(args.argv);
	return ret;
}

bool PIO::terminal_create(const char *path, char *const argv[], char *const envp[])
{
	eyre_pio::exec_args args;
	args.path = strdup(path);
	size_t len;
	for (len=0; argv[len] != NULL; ++len);
	args.argv = (char **) malloc((len+1)*sizeof(char *));
	for (int i=0; i<len; ++i)
	{
		args.argv[i] = strdup(argv[i]);
	}
	args.argv[len] = NULL;
	size_t lenp;
	for (lenp=0; envp[lenp] != NULL; ++lenp);
	args.envp = (char **) malloc((lenp+1)*sizeof(char *));
	for (int i=0; i<lenp; ++i)
	{
		args.envp[i] = strdup(envp[i]);
	}
	args.envp[lenp] = NULL;
	bool ret = terminal_create(open_app_appoint_env, (void *) &args);
	free(args.path);
	for (int i=0; i<len; ++i)
	{
		free(args.argv[i]);
	}
	free(args.argv);
	for (int i=0; i<lenp; ++i)
	{
		free(args.envp[i]);
	}
	free(args.envp);
	return ret;
}

/*
 * @brief 创建虚拟终端
 * @return bool 成功返回true，失败返回false（之前已经打开也会返回false）
 */
bool PIO::terminal_create(void (*callback)(void *), void *arg, void (*settings)(int))
{
	if (is_open)
	{
		return false;
	}

	bool ret = create_pty();
	
	if (ret == false)
	{
		return false;
	}
	
	pid_t pid = vfork();
	if (pid < 0)	// 失败
	{
		close(master_fd);
		return false;
	}
	if (pid == 0)	// 子进程
	{
		slave_pid = getpid();
		close(master_fd);	// 子进程里不需要master句柄
		
		// 创建会话
		if (setsid() == -1)
		{
			ret = false;
			exit(0);
		}
		
		// 打开从终端
		int slave = open(slave_name.data(), O_RDWR);
		if (slave == -1)
		{
			ret = false;
			exit(0);
		}
		
		// 取消回显
		struct termios ter;
		if (tcgetattr(slave, &ter) == -1)
		{
			ret = false;
			exit(0);
		}
		ter.c_lflag &= ~(ECHO);
		if (tcsetattr(slave, TCSANOW, &ter) == -1)
		{
			ret = false;
			exit(0);
		}

		// 用户自定义设置
		if (settings)
		{
			settings(slave);
		}
		
		// 重定向从终端输入输出
		// 实现：
		// 1、主终端write，从终端能从标准输入得到
		// 2、从终端标准输出，主终端能read到
		// 3、从终端标准错误，主终端能read到
		dup2(slave, STDIN_FILENO);
		dup2(slave, STDOUT_FILENO);
		dup2(slave, STDERR_FILENO);
		
		close(slave);
		
		// 从终端运行目标程序
		// execv(path, argv);

		// 回调
		callback(arg);
	}

	// 父进程
	if (ret == false)
	{
		return false;
	}
	
	is_open = true;
	
	// 开启主终端给从终端输入指令的线程
	pthread_create(&t_input, NULL, Thread::terminal_input, this);
	
	return true;
}

/*
 * @brief 关闭虚拟终端
 * @return bool 成功返回true，失败返回false
 */
bool PIO::terminal_close()
{
	if (kill(slave_pid, SIGKILL) == -1)
	{
		return false;
	}
	if (close(master_fd) == -1)
	{
		return false;
	}
	is_open = false;
	return true;
}

void PIO::input_command(const string &command)
{
	pthread_mutex_lock(&md_input);
	d_input.push(command);
	pthread_mutex_unlock(&md_input);
}

#endif
