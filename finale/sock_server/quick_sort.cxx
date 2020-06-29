#include "http_examples.h"
#include <iostream>
#include <omp.h>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <vector>

using namespace std;
using INT32 = int;
typedef ELET * CONTAINER;

INT32 partition(CONTAINER &arr, INT32 low_index, INT32 high_index) {
  INT32 lcursor, rcursor, middle;
  middle = arr[low_index];
  lcursor = low_index + 1;
  rcursor = high_index;
  while (lcursor <= rcursor) {
    while (lcursor < high_index && middle >= arr[lcursor])
      lcursor++;

    while (middle < arr[rcursor])
      rcursor--;

    if (lcursor < rcursor) {
      INT32 temp = arr[lcursor];
      arr[lcursor] = arr[rcursor];
      arr[rcursor] = temp;
    } else {
      INT32 temp = arr[low_index];
      arr[low_index] = arr[rcursor];
      arr[rcursor] = temp;
      return (rcursor);
    }
  }
  return (rcursor);
}

static INT32 max_level = 1;
void quicksort(CONTAINER &arr, INT32 low_index, INT32 high_index) {
  if (low_index < high_index) {
    INT32 right = partition(arr, low_index, high_index);
#pragma omp parallel sections
    {
#pragma omp section
      {
        // cout << "working " << low_index << " to " << right - 1 << " on " << omp_get_thread_num() << endl;
        quicksort(arr, low_index, right - 1);
      }
#pragma omp section
      {
        // cout << "working " << right + 1 << " to " << high_index << " on " << omp_get_thread_num() << endl;
        quicksort(arr, right + 1, high_index);
      }
    }
  }
}

INT32 tracedQuickSort(ELET *arr, ELET_OFST length, INT32 level, SPTR span) {
  AS_CHILD_SPAN(quick_sort_span, "quick-sort-sorting-parallel", span);
  omp_set_num_threads(level);
  max_level = level;
  std::string threads_count = "Total threads :";
  threads_count += level;
  quick_sort_span->BaggageItem(threads_count);
  INT32 i = 0;
  srand(time(0));
  auto start = std::chrono::system_clock::now();
  quicksort(arr, 0, length - 1);
  auto end1 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = end1 - start;
  cout << "Verification \n";
  for (i = 1; i < length; i++) {
    Assert(arr[i] >= arr[i - 1], printf("Incorrect sorting result at position %d", i));
  }
  cout << endl;
  cout << "Quick Sorting length = " << length << endl;
  cout << "Parallel threads = " << omp_get_max_threads() << endl;
  cout << "Spent = " << seconds.count() << "s" << endl;
  return 0;
}