//
// Created by xc5 on 6/29/20.
//
#include "http_examples.h"
#include "omp.h"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"

void rank_sort(ELET *data_in, ELET *data_out, ELET_OFST length) {
  UINT32 i, j, k;
#pragma omp parallel private(i, j, k)
  {
#pragma omp for
    for (i = 0; i < length; i++) {
      k = 0;
      for (j = 0; j < length; j++) {
        if (data_in[i] > data_in[j] || (i > j && data_in[i] == data_in[j])) {
          k++; // Count for rank
        }
      }
      data_out[k] = data_in[i];
    }
  }
}

INT32 tracedRankSort(ELET *arr, ELET_OFST length, INT32 level, SPTR span) {
  AS_CHILD_SPAN(quick_sort_span, "rank-sort-sorting-parallel", span);
  omp_set_num_threads(level);
  std::string threads_count = "Total threads :";
  threads_count += level;
  quick_sort_span->BaggageItem(threads_count);
  ELET *result = new ELET[length];
  INT32 i = 0;
  srand(time(0));
  auto start = std::chrono::system_clock::now();
  rank_sort(arr, result, length);
  auto end1 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = end1 - start;
  cout << "Verification starting \n";
  stringstream strs;
  strs << "order-" << "rank-" << level << ".txt";
  ofstream of(strs.str());
  for (i = 1; i < length; i++) {
    Assert(result[i] >= result[i - 1], printf("Incorrect sorting result at position %d", i));
    of << result[i] << ((i != length - 1) ? " " : "");
  }
  cout << endl;
  cout << "Rank Sorting length = " << length << endl;
  cout << "Parallel threads = " << omp_get_max_threads() << endl;
  cout << "Spent = " << seconds.count() << "s" << endl;
  delete[] result;
  return 0;
}

#pragma clang diagnostic pop