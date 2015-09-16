#include "papi.h"
#include <iostream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/time.h>
#include <string>

#define CACHE_LINE 64
#define EVENT_COUNT (4)

#define TIMER(val) do { \
  struct timeval tm; \
  gettimeofday(&tm, NULL); \
  val = tm.tv_sec * 1000 + tm.tv_usec/1000; \
} while(0)


DEFINE_string(opt, "a", " data type, a, b, a1, b1");
DEFINE_int64(gb, 1, " size in gb");

/* 64 bytes */
struct AType {
  double a[8];
  static size_t valid_bytes() {
    return 64;
  }
  void init(double v) {
    for(size_t i = 0;i < 8;i ++) {
      a[i] = v;
    }
  }

  void apply() {
    for(size_t i = 0;i < 8; i ++) {
      a[i] += 1.0;
    }
  }
};
typedef struct AType AType;


/* 64 + 64 bytes */
struct BType {
  double a[8];
  double b[8];
  static size_t valid_bytes() { 
    return 64;
  }
  void init(double v) {
    for(size_t i = 0;i < 8;i ++) {
      a[i] = v;
      b[i] = v;
    }
  }

  void apply() {
    for(size_t i = 0;i < 8;i ++) {
      a[i] += 1.0;
    }
  }
};
typedef struct BType BType;


struct A1Type { 
  double a;
  static size_t valid_bytes() {
    return 8;
  }
  void init(double v) { 
    a = v;
  }

  void apply() { 
    a += 1.0;
  }
}; 
typedef struct A1Type A1Type;

struct B1Type { 
  double a;
  double b;
  static size_t valid_bytes() { 
    return 8;
  }
  void init(double v) {
    a = v;
    b = v;
  }

  void apply() { 
    a += 1.0;
  }
};
typedef struct B1Type B1Type;


void register_event()
{ 
  int events[EVENT_COUNT] = {PAPI_L3_DCA, PAPI_L3_DCR, PAPI_L3_DCW, PAPI_TLB_DM};
  int ret = PAPI_start_counters(events, EVENT_COUNT);
  CHECK_EQ(ret, PAPI_OK);
}

void report_event()
{

  std::vector<std::string> event_str(EVENT_COUNT);
  event_str[0] = "PAPI_L3_DCA";
  event_str[1] = "PAPI_L3_DCR";
  event_str[2] = "PAPI_L3_DCW";
  event_str[3] = "PAPI_TLB_DM";

  long long int values[EVENT_COUNT];
  int ret = PAPI_read_counters(values, EVENT_COUNT);
  CHECK_EQ(ret, PAPI_OK);
  for(size_t i = 0;i < EVENT_COUNT;i ++) {
    LOG(INFO) << " Event " << event_str[i] << " " << values[i];
  }
}



template<typename T>
uint64_t get_valid_size(size_t len) {
  return len * T::valid_bytes();
}

template<typename T>
double comp_band(size_t len, long ms)
{
  double sec = double(ms) / 1000.0;
  double gb = (double(get_valid_size<T>(len))) / 1024.0 / 1024.0 / 1024.0;
  return gb / sec;
}


template<typename T>
size_t prep_data(T* & data_ptr, size_t bytes)
{
  size_t len = bytes / T::valid_bytes();
  data_ptr = new T[len];
  for(size_t i = 0;i < len;i ++) {
    (data_ptr)[i].init((double)i);
  }
  return len;
}


template<typename T>
void apply_data(T* data_ptr, size_t len)
{
  for(size_t i = 0;i < len;i ++) {
    data_ptr[i].apply();
  }
}

template<typename T>
void perf_data(T* data_ptr, size_t len, std::string opt)
{
  long t1, t2;
  TIMER(t1);
  apply_data<T>(data_ptr, len);
  TIMER(t2);

  double band = comp_band<T>(len, t2-t1);
  LOG(INFO) << " " << opt << " Perf data time : " << (t2 - t1) << " ms." << " band : " << band ;
}

template<typename T>
void work() {
  T* data_ptr = nullptr;
  size_t len = prep_data<T>(data_ptr, FLAGS_gb * 1024 * 1024 * 1024);
  LOG(INFO) << " prep data ok. len = " << len ;

  /* warm up */
  for (size_t i = 0;i < 5;i ++) { 
    perf_data<T>(data_ptr, len, FLAGS_opt);
  }
 
  size_t tot_cnt = 5;
  register_event();
  for (size_t i = 0;i < tot_cnt;i ++) { 
    perf_data<T>(data_ptr, len, FLAGS_opt);
  }
  report_event();
  LOG(INFO) << " perf data ok.";

  delete data_ptr;
  data_ptr = nullptr;
}


int main(int argc, char ** argv) {

  google::ParseCommandLineFlags(&argc, &argv, false);

  LOG(INFO) << " sizeof AType " << sizeof(AType);
  LOG(INFO) << " sizeof BType " << sizeof(BType);



  if (FLAGS_opt == "a")  {
    work<AType>();
  } else if (FLAGS_opt == "b" ) {
    work<BType>();
  } else if (FLAGS_opt == "a1" ) {
    work<A1Type>();
  } else if (FLAGS_opt == "b1") { 
    work<B1Type>();
  }

  return 0;
}
