// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "server.h"

std::string genRes(char *);

server::server(int port, size_t thread_num) : m_threads_num{thread_num} {
	m_sock = ::socket(PF_INET, SOCK_STREAM, 0);
	check(m_sock, "could not create listening socket");

	int optval = 1;
	check(
	    ::setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)),
	    "could not set socket options");

	sa_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	check(bind(m_sock, (sa *)&server_addr, sizeof(server_addr)),
	      "could not bind");
}
server::~server() { ::close(m_sock); }

void server::run_t() {
	setup_threadpool();
	check(::listen(m_sock, BACKLOG_LENGHT), "could not listen");

	int newsocket;
	while ((newsocket = accept_conn()) > 0) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.push(std::move(newsocket));
		m_c.notify_all();
	}
}

void server::run_n() {
	check(::listen(m_sock, BACKLOG_LENGHT), "could not listen");

	fd_set current_sockets, ready_sockets;

	FD_ZERO(&current_sockets);
	FD_SET(m_sock, &current_sockets);

	while (true) {
		ready_sockets = current_sockets;

		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
			std::cerr << "select error";
		}

		for (int i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &ready_sockets)) {
				if (i == m_sock) {
					int client_sock = accept_conn();
					FD_SET(client_sock, &current_sockets);
				} else {
					handle_conn(i);
					FD_CLR(i, &current_sockets);
				}
			}
		}
	}
}

int server::accept_conn() {
	int client_sock;
	check(client_sock = ::accept(m_sock, NULL, 0), "accept failed");
	return client_sock;
}

void server::handle_conn(int sock) {
	char buff[BUFF_LENGHT];
	::read(sock, buff, BUFF_LENGHT);
	std::string res = genRes(buff);

	std::cout << "Raw request: " << std::string(buff) << std::endl;
	std::cout << "Data: " << res << std::endl;

	::write(sock, res.c_str(), res.length());
	::shutdown(sock, SHUT_RDWR);
	::close(sock);
}

void server::setup_threadpool() {
	for (std::size_t i = 0; i < m_threads_num; ++i) {
		auto worker = std::bind(&server::worker_t, this);
		auto t = new std::thread(std::move(worker));

		std::shared_ptr<std::thread> thread(std::move(t));
		m_threads.push_back(thread);
	}

	for (std::size_t i = 0; i < m_threads.size(); ++i) {
		m_threads[i]->detach();
	}
}

void server::worker_t() {
	while (true) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_c.wait(lock, [&] { return !m_queue.empty(); });

		while (!m_queue.empty()) {
			int sock = m_queue.front();
			m_queue.pop();
			m_c.notify_one();

			handle_conn(sock);
		}
	}
}

std::string genRes(char *buff) {
	char *dataPtr = std::strstr(buff, "\r\n\r\n");
	std::string data;

	if (dataPtr != NULL) {
		dataPtr += 4;
		data = std::string(dataPtr);
	} else {
		data = std::string(buff);
	}

	std::stringstream header;
	header << "HTTP/1.1 200 OK\r\n"
	       << "Connection: close\r\n"
	       << "Content-Type: text/plain\r\n"
	       << "Content-Length: " << data.length() << "\r\n"
	       << "\r\n";
	return header.str() + data;
}

void server::check(int expr, std::string msg) {
	if (expr == -1) {
		throw std::runtime_error(msg);
	}
}
