#include "mpi.h"
#include <vector>
#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <sys/time.h>
#include <time.h>

DEFINE_int32(num_int, 1024, "number of int to send");


#define TIMER(val) do { \
  struct timeval tm; \
  gettimeofday(&tm, NULL); \
  val = tm.tv_sec * 1000 + tm.tv_usec/1000; \
} while(0)


class DistControl {
  public : 
  int rank; 
  int num_rank;
  MPI_Request req;

  DistControl (int rank_in, int num_rank_in) 
    :rank(rank_in), num_rank(num_rank_in)
  {
  }

  void barrier()
  {
    int ret = MPI_Barrier(MPI_COMM_WORLD);
    CHECK_EQ(ret, MPI_SUCCESS);
  }
 
  template<typename T> 
  void asend(std::vector<T> *send_vec, int to_rank)
  {
    int rc = MPI_Isend(send_vec->data() , FLAGS_num_int * sizeof(T), MPI_BYTE, to_rank, 0, MPI_COMM_WORLD, &req);
    CHECK_EQ(rc, MPI_SUCCESS);
  }


  template<typename T> 
  void asend(std::vector<T> *send_vec, int to_rank, size_t offset, size_t num)
  {
    int rc = MPI_Isend(send_vec->data() + offset , num * sizeof(T), MPI_BYTE, to_rank, 0, MPI_COMM_WORLD, &req);
    CHECK_EQ(rc, MPI_SUCCESS);
  }


  template<typename T>
  void send(std::vector<T> *send_vec, int to_rank)
  {
    int rc = MPI_Send(send_vec->data(), send_vec->size() * sizeof(T), MPI_BYTE, to_rank, 0, MPI_COMM_WORLD);
    CHECK_EQ(rc, MPI_SUCCESS);
  }

  template<typename T>
  void arecv(std::vector<T> *recv_vec, int from_rank, size_t num)
  {
    int rc = MPI_Irecv(recv_vec->data(), num * sizeof(T), MPI_BYTE, from_rank, 0, MPI_COMM_WORLD, &req);
    CHECK_EQ(rc, MPI_SUCCESS);
  }

  template<typename T>
  void arecv(std::vector<T> *recv_vec, int from_rank, size_t offset, size_t num)
  {
    int rc = MPI_Irecv(recv_vec->data() + offset , num * sizeof(T), MPI_BYTE, from_rank, 0, MPI_COMM_WORLD, &req);
    CHECK_EQ(rc, MPI_SUCCESS);
  }

  template<typename T>
  void recv(std::vector<T> *recv_vec, int to_rank, size_t size)
  {
    MPI_Status status;
    int rc = MPI_Recv(recv_vec->data(), size * sizeof(T), MPI_BYTE, to_rank, 0, MPI_COMM_WORLD, &status);
    CHECK_EQ(rc, MPI_SUCCESS);
  }


  void wait()
  {
    MPI_Status status;
    int rc = MPI_Wait(&req, &status);
    CHECK_EQ(rc, MPI_SUCCESS);
  }
};



template<typename T>
void perf_async(DistControl *dc, std::vector<T> *my_vec)
{
  int64_t t1, t2, t3;

  TIMER(t1);
  if (dc->rank == 0) {
    /* send */
    dc->asend<int>(my_vec, 1);
  } else {
    /* recv */
    dc->arecv<int>(my_vec, 0, FLAGS_num_int);
  }
  TIMER(t2);

  dc->wait();

  TIMER(t3);

  if (dc->rank == 0 )
    LOG(INFO) << " asend time : " << t2 - t1;
  else
    LOG(INFO) << " arecv time : " << t2 - t1;

  if (dc->rank == 0)
    LOG(INFO) << " send complete wait time : " << t3 - t2;
  else
    LOG(INFO) << " recv complete wait time : " << t3 - t2;

  if (dc->rank == 0)
    LOG(INFO) << " send whole wait time : " << t3 - t1;
  else
    LOG(INFO) << " recv whole wait time : " << t3 - t1;


  size_t GB = 1024 * 1024 * 1024;
  size_t bytes = sizeof(int) * FLAGS_num_int;
  double s1GB = (bytes * 1000) / double(GB) / (double)(t3 - t2) ;
  double s2GB = (bytes * 1000) / double(GB) / (double)(t3 - t1) ;

  if (dc->rank == 0) {
    LOG(INFO) << " ASYNC GB/s sender : " << s1GB << "  " << s2GB;
  } else {
    LOG(INFO) << " ASYNC GB/s recver : " << s1GB << "  " << s2GB;
  }
}


template<typename T>
void perf_sync(DistControl *dc, std::vector<T> *my_vec)
{
  int64_t t1, t2;

  TIMER(t1);
  if (dc->rank == 0) {
    /* send */
    dc->send<int>(my_vec, 1);
  } else {
    /* recv */
    dc->recv<int>(my_vec, 0, FLAGS_num_int);
  }
  TIMER(t2);

  if (dc->rank == 0 )
    LOG(INFO) << " send time : " << t2 - t1;
  else
    LOG(INFO) << " recv time : " << t2 - t1;

  size_t GB = 1024 * 1024 * 1024;
  size_t bytes = sizeof(int) * FLAGS_num_int;
  double s1GB = (bytes * 1000) / double(GB) / (double)(t2 - t1) ;

  if (dc->rank == 0) {
    LOG(INFO) << " SYNC GB/s sender : " << s1GB ;
  } else {
    LOG(INFO) << " SYNC GB/s recver : " << s1GB;
  }
}



template<typename T>
void check_vec(std::vector<T> *vec, size_t size) {
  for(size_t i = 0;i < size;i ++) {
    CHECK_EQ((*vec)[i], i);
  }
}

template<typename T>
void init_vec(std::vector<T> *vec, size_t size) {
  for(size_t i = 0;i < size;i ++) {
    (*vec)[i] = i;
  }
}


template<typename T>
void perf_async_off(DistControl *dc, std::vector<T> *vec, size_t off, size_t num) 
{
  long t1, t2, t3;
  TIMER(t1);
  if (dc->rank == 0) {
    dc->asend<int>(vec, 1, off, num);
  } else {
    dc->arecv<int>(vec, 0, off, num);
  }
  TIMER(t2);

  if (dc->rank == 1) {
    sleep(1);
  }
  TIMER(t2);
  dc->wait();

  TIMER(t3);

  if (dc->rank == 0 )
    LOG(INFO) << " asendoff time : " << t2 - t1;
  else
    LOG(INFO) << " arecvoff time : " << t2 - t1;

  if (dc->rank == 0)
    LOG(INFO) << " asendoff complete wait time : " << t3 - t2;
  else
    LOG(INFO) << " arecvoff complete wait time : " << t3 - t2;

  if (dc->rank == 0)
    LOG(INFO) << " asendoff whole wait time : " << t3 - t1;
  else
    LOG(INFO) << " arecvoff whole wait time : " << t3 - t1;


  size_t GB = 1024 * 1024 * 1024;
  size_t bytes = sizeof(int) * num;
  double s1GB = (bytes * 1000) / double(GB) / (double)(t3 - t2) ;
  double s2GB = (bytes * 1000) / double(GB) / (double)(t3 - t1) ;

  if (dc->rank == 0) {
    LOG(INFO) << " ASYNC GB/s sender : " << s1GB << "  " << s2GB << " size GB " << bytes / (double)GB;
  } else {
    LOG(INFO) << " ASYNC GB/s recver : " << s1GB << "  " << s2GB << " size GB " << bytes / (double)GB;
  }
}


template<typename T>
void do_perf_async_off(DistControl *dc, std::vector<T> *vec)
{
  
  perf_async_off<int>(dc, vec, 0, FLAGS_num_int);
  /* ROUND 1 */
  if (dc->rank == 0) {
    /* send */
    perf_async_off<int>(dc, vec, 0, FLAGS_num_int / 2);
  } else { 
    /* recv */
    size_t off = FLAGS_num_int / 2;
    perf_async_off<int>(dc, vec, off, FLAGS_num_int/2);
  }

  /* ROUND 2 */

}



int main(int argc, char ** argv) 
{
  google::ParseCommandLineFlags(&argc, &argv, false);

  int rank, num_rank, rc;
  MPI_Init(&argc, &argv);
  rank = 1;

  rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  CHECK_EQ(rc, MPI_SUCCESS);
  rc = MPI_Comm_size(MPI_COMM_WORLD, &num_rank);
  CHECK_EQ(rc, MPI_SUCCESS);

  DistControl dc(rank, num_rank);
  
  LOG(INFO) <<  rank << " / " << num_rank << " transfer size " << FLAGS_num_int
            << " size in MB " << (sizeof(int) * FLAGS_num_int) / 1024 / 1024 ;

  /* init vector */
  size_t vec_arr_size = 3;
  std::vector<int> ** my_vec_array = new  std::vector<int> * [vec_arr_size];
  for(size_t i = 0;i < vec_arr_size;i ++) {
    my_vec_array[i] = new std::vector<int>(FLAGS_num_int);
    if (rank == 0)
      init_vec<int>(my_vec_array[i], FLAGS_num_int);
  }


  /* perf async */
  /*
  for(int xx = 0; xx < 3; xx ++)  {
    for(int i = 0;i < vec_arr_size;i ++) { 
      if (rank == 0)  init_vec<int>(my_vec_array[i], FLAGS_num_int);
      dc.barrier();
      if (rank == 0) {
        perf_async<int>(&dc, my_vec_array[i]);
      } else { 
        perf_async<int>(&dc, my_vec_array[0]);
      }
    }
  }
  */

  /*
  for(int i = 0;i < vec_arr_size ;i ++) {
    if (rank == 0)  init_vec<int>(my_vec_array[i], FLAGS_num_int); // if you want to use the same buffer, change the index
    dc.barrier();
    if (rank == 0) {
      perf_sync<int>(&dc, my_vec_array[0]);
    } else { 
      perf_sync<int>(&dc, my_vec_array[i]);
    }
  }
  */

  do_perf_async_off(&dc, my_vec_array[0]);





  /*
  if (rank == 1) {
    // check value
    for(size_t i = 0;i < vec_arr_size;i ++) {
      check_vec<int>(my_vec_array[i], FLAGS_num_int);
    }
  }
  */


  MPI_Finalize();

  return 0;
}


