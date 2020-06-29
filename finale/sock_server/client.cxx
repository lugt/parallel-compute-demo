//
// Created by xc5 on 6/25/20.
//
#include "http_examples.h"
#include "client.h"

void testing_bandwidth_with_timer(ELET_OFST length, const char *func_name, void (*func)(SPTR), SPTR span) {
  using namespace std::chrono;
  AS_CHILD_SPAN(real_span, func_name, span);
  steady_clock::time_point clock_begin = steady_clock::now();
  func(real_span);
  steady_clock::time_point clock_end = steady_clock::now();
  real_span->Finish();
  steady_clock::duration time_span = clock_end - clock_begin;
  double nseconds = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;
  double nmb = double(sizeof(ELET) * length) / 1024.0 / 1024.0;
  std::cout << "Processed data " << nmb << " MB in function  " << func_name << ", spent " << nseconds << " seconds.";
  std::cout << " Bandwidth: " << (nmb / nseconds) << "MBps";
  std::cout << std::endl;
}

