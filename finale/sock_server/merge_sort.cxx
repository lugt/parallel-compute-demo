#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
//
// Created by xc5 on 6/29/20.
//
#include "http_examples.h"
#include "omp.h"


void
merge_sort (ELET *arr, ELET_OFST size, INT32 threads)
{
  INT32 sub_array_size = size / 2;
  ELET *rbegin = arr + sub_array_size;
  if (size <= 2) {
    std::inplace_merge(arr, rbegin, rbegin + sub_array_size);
    return;
  }
#pragma omp parallel sections
  {
#pragma omp section
    {
      merge_sort (arr, sub_array_size, threads / 2);
    }
#pragma omp section
    {
      merge_sort (arr + sub_array_size, size - sub_array_size, threads - threads / 2);
    }
  }
  std::inplace_merge (arr, rbegin, arr + size);
}


INT32 tracedMergeSort(ELET *arr, ELET_OFST length, INT32 level, SPTR span) {
  AS_CHILD_SPAN(quick_sort_span, "merge-sorting-parallel", span);
  omp_set_num_threads(level);
  std::string threads_count = "Total threads :";
  threads_count += level;
  quick_sort_span->BaggageItem(threads_count);
  ELET *result = new ELET[length];
  INT32 i = 0;
  auto start = std::chrono::system_clock::now();
  merge_sort(arr, length, length);
  auto end1 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = end1 - start;
  cout << "Verification \n";
  stringstream strs;
  strs << "order-" << "merge-" << level << ".txt";
  ofstream of(strs.str());
  for (i = 1; i < length; i++) {
    Assert(arr[i] >= arr[i - 1], printf("Incorrect sorting result at position %d", i));
    of << arr[i] << ((i != length - 1) ? " " : "");
  }
  cout << endl;
  cout << "Merge Sorting length = " << length << endl;
  cout << "Parallel threads = " << omp_get_max_threads() << endl;
  cout << "Spent = " << seconds.count() << "s" << endl;
  delete[] result;
  return 0;
}
#pragma clang diagnostic pop