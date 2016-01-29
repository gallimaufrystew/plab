//
// Created by z on 1/28/16.
//

#include "coordinator.hpp"
#include <pthread.h>
#include <glog/logging.h>

#define N_CLIENT (1)
#define N_WRITE (1)

struct ser_arg{
	Server *server;
};

struct cli_arg{
	Client *client;
};

void * server_fn(void *thr_args) {
	struct ser_arg* args = static_cast<struct ser_arg*>(thr_args);
	Server *server = args->server;

	server->wait_connection();

	server->listen();

	return NULL;
}



void *client_fn(void *thr_args) {
	struct cli_arg* args = static_cast<struct cli_arg*>(thr_args);
	Client *client = args->client;

	client->connect();

	for(int i = 0;i < N_WRITE; i++) {
		uint64_t counter = 100;
		client->notify(counter);
	}

	LOG(INFO) << "Client exiting.";

	return NULL;
}


int main(int argc, char **argv) {

	pthread_t server_thr;
	pthread_t client_thrs[N_CLIENT];

	int server_id = 0;
	Server server(N_CLIENT, server_id);
	Client clients[N_CLIENT];
	for(int i = 0;i < N_CLIENT;i ++) {
		clients[i] = Client(i, &server);
	}


	struct ser_arg sarg;
	sarg.server = &server;
	pthread_create(&server_thr, NULL, server_fn, (void*)&sarg);

	struct cli_arg *cargs = new struct cli_arg[N_CLIENT];
	for(int i = 0;i < N_CLIENT;i ++) {
		cargs[i].client = &clients[i];
		pthread_create(&client_thrs[i], NULL, client_fn, (void*)&cargs[i]);
	}

	for(int i = 0;i < N_CLIENT;i ++) {
		pthread_join(client_thrs[i], NULL);
	}

	pthread_join(server_thr, NULL);

	return 0;
}
