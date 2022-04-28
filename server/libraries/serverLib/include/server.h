#ifndef SERVER_H
#define SERVER_H

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#define BACKLOG_LENGHT 1024
#define BUFF_LENGHT 1024

typedef struct sockaddr_in sa_in;
typedef struct sockaddr sa;

class server {
    public:
	server(int, size_t);
	~server();

	void run_t();
	void run_n();

    private:
	void listen_t();
	int accept_conn();
	void handle_conn(int);

	void setup_threadpool();
	void worker_t();

	void check(int, std::string);

	int m_sock;
	size_t m_threads_num;
	std::vector<std::shared_ptr<std::thread>> m_threads;

	std::condition_variable m_c;
	std::mutex m_mutex;

	std::queue<int> m_queue;
};

#endif	// SERVER_H
