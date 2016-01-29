//
// Created by z on 1/28/16.
//
#pragma once

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <glog/logging.h>
#include <errno.h>
#include <stdint.h>
#include "lock.hpp"

#define EPOLL_WAIT_TIMEOUT (1000) // 1.0 second

class Client;

class Server {
private :
	int num_clients_;
	int id_;

public :
	int * fds;
	Client ** clients; // array of Client *
	SpinLock *cli_locks;


	Server(int num_clients, int id) : num_clients_(num_clients), id_(id) {
		fds = new int[num_clients_];
		for(int i = 0;i < num_clients_;i ++) {
			fds[i] = eventfd(0, EFD_NONBLOCK);
			//fds[i] = eventfd(0, 0);
			CHECK_NE(fds[i], -1) << " Create eventfd for client " << i << " failed.";
		}

		clients = new Client* [num_clients_];
		for(int i = 0;i < num_clients_ ;i ++) {
			clients[i] = nullptr;
		}

		cli_locks = new SpinLock [num_clients_];
		for(int i = 0;i < num_clients_ ;i ++) {
			cli_locks[i].unlock();
		}
	}


	int cli_get_fd(int id, Client *c){
		cli_locks[id].lock();
		int fd = fds[id];
		clients[id] = c;
		cli_locks[id].unlock();
		return fd;
	}


	/*
	 * Wait num_client clients to connect to me
	 */
	void wait_connection() {
		int count = 0;
		while(count < num_clients_) {
			count = 0;
			for(int i = 0;i < num_clients_;i ++) {
				cli_locks[i].lock();
				if (clients[i] != nullptr) count++;
				cli_locks[i].unlock();
			}
		}
		LOG(INFO) << "Server::conn " << num_clients_ << " connected.";

	}


	void handle_read_event(int cid, int fd) {
		uint64_t counter = 0;
		//int ret = eventfd_read(fd, &counter);
		ssize_t ret = read(fd, &counter, sizeof(counter));
		LOG(INFO) << " read size " << ret;
		if (ret < 0 && errno != EAGAIN) {
			LOG(FATAL) << "read fd error :" << strerror(errno) << " ret : " << ret << " counter : " << counter;
		}
		LOG(INFO) << "read event, cli " << cid << ", counter " << counter << " size " << ret;
	}


	/*
	 * listening events from clients
	 */
	void listen() {
		// create epoll events
		struct epoll_event events[num_clients_];
		int ep_fd = epoll_create(num_clients_);
		CHECK_GE(ep_fd, 0) << "Server::epoll_create failed.";

		for(int i = 0;i < num_clients_;i ++) {
			struct epoll_event read_event;
			read_event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
			read_event.data.fd = fds[i];
			read_event.data.u32 = static_cast<uint32_t>(i);
			int ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, fds[i], &read_event);
			CHECK_GE(ret, 0) << "Server::epoll_ctl failed.";
		}

		// begin while loop
		while(1) {
			int nfds = epoll_wait(ep_fd, &events[0], num_clients_, EPOLL_WAIT_TIMEOUT);
			CHECK_GE(nfds, 0) << "epoll_wait error.";

			LOG(INFO) << "Server:: epoll nfds : " << nfds;
			// for each ready fd, handle fd
			for(int i = 0;i < nfds; i ++) {
				int cid = static_cast<int>(events[i].data.u32);

				if( (events[i].events & EPOLLIN)) {
					int fd = events[i].data.fd;
					handle_read_event(cid, fd);
				}
				if (events[i].events & EPOLLHUP){
					LOG(FATAL) << "epoll eventfd has epoll hup. client id : " << cid;
				}
				if (events[i].events & EPOLLERR) {
					LOG(FATAL) << "epoll eventfd has epoll error. client id : " << cid;
				}

			}

		}
	}

};


class Client {

private :
	int id_;
	Server *server_;

public :
	int fd; // fd to communicate with server_

	Client() : id_(-1), server_(nullptr) {}
	Client(int id, Server * _server) : id_(id), server_(_server){ }

	/*
	 * Connect server_ with sid
	 */
	void connect() {
		// get fd from server_
		fd = server_->cli_get_fd(id_, this);
		LOG(INFO) << "Client " << id_ << " connected.";
	}


	void notify(uint64_t counter_) {
		uint64_t counter = counter_;
		ssize_t ret = 0;
		do{
			//ret = eventfd_write(fd, counter);
			ret = write(fd, &counter, sizeof(counter));

			if (ret < 0)
				LOG(INFO) << " ERROR " << strerror(errno);

		} while(ret < 0 && errno == EAGAIN);
		CHECK_GE(ret, 0) << "write error with " << strerror(errno);
		LOG(INFO) << "Client " << id_ << " signal " << counter << " size " << ret;
	}

};
