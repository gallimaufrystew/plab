#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <glog/logging.h>
#include <sys/eventfd.h>


struct write_thr_args {
	int efd;
};

struct read_thr_args {
	int efd;
};

using WArgs = struct write_thr_args;
using RArgs = struct read_thr_args;


void *write_fn(void *thr_args) {
	WArgs * wargs = static_cast<WArgs*>(thr_args);
	int efd = wargs->efd;




}


void *read_fn(void *thr_args) {
	WArgs * wargs = static_cast<WArgs*>(thr_args);
	int efd = wargs->efd;


}


int main(int argc, char **argv) {

	int efd = eventfd(0, EFD_NONBLOCK);

	pthread_t writer;
	pthread_t reader;

	int ret = 0;

	WArgs * wargs = (WArgs*)malloc(sizeof(WArgs));
	wargs->efd = efd;

	RArgs * rargs = (RArgs*)malloc(sizeof(RArgs));
	rargs->efd = efd;

	pthread_create(&writer, NULL, write_fn, (void*)(wargs));

	pthread_create(&reader, NULL, read_fn, (void*)(rargs));


}