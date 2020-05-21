#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include <random>
#include <chrono>
using namespace std;

#define MAX_CNT 10000000
int total_cnt = 0;

typedef double *dataType;
typedef const double *cdataType;
typedef const double dataEltType;

//// Merge two blocks
//inline void  Merge_one  (dataType datal , dataType datalend , dataType datar, dataType datarend, , dataType data2){
//    int i1 = 0, i2 = 0, i3 = 0;
//
//    while (i1 < sizel && i2 < sizer) {
//      if (datal[i1] < datar[sizel + i2])
//        data2[i3++] = datal[i1++];
//      else
//        data2[i3++] = datar[sizel + (i2++)];
//    }
//	while(i1<sizel)
//      data2[i3++] = datal[i1++];
//	while(i2<sizer)
//      data2[i3++] = datar[sizel + (i2++)];
//}

inline int rank_me(cdataType start, cdataType end, dataEltType num) {
  cdataType cursor = start;
  int count = 0;
  cdataType loc = std::lower_bound(start, end, num);
  return (loc - start);
}

// Sort all datas in vc, and put int dest.
// Work in O(N * lgN)
void sortAll(cdataType a, cdataType b, dataType c, int cnt) {
  omp_lock_t alock;
  omp_init_lock(&alock);
  int section_size = (total_cnt / cnt);
  omp_set_num_threads(cnt);
  int i;
  int u[cnt + 4];
  int v[cnt + 4];

  u[0]=v[0]=0;
  u[cnt]=v[cnt]=total_cnt;

#pragma omp parallel for private(i)
  for (i=1; i<cnt; i++) {
    u[i] = ((long long) i * (long long) total_cnt / (long long) cnt) - 1;
    v[i] = rank_me(b, b + total_cnt, a[u[i]]); //求秩
  }

//
#pragma omp parallel for private(i)
  for (i = 0; i < cnt; i++) {
    // std::merge(a+u[i], a+u[i+1], b+v[i], b+v[i+1], c+u[i]+v[i]); //归并
    int l = u[i], m = v[i], cursor = u[i] + v[i];
    const int lmax = u[i+1];
    const int mmax = v[i+1];
    while (l < lmax && m < mmax) {
      if (a[l] > b[m]) {
        c[cursor++] = b[m++];
      } else {
        c[cursor++] = a[l++];
      }
    }
  }
}

int trial(int argc, char **argv) {
  int cnt = 4;
  total_cnt = MAX_CNT;
  double *a = NULL;
  double *b = NULL;
  double *c = NULL;
  a = (double *) malloc(total_cnt * sizeof(double));
  if (a == NULL) {
    return -1;
  }

  b = (double *) malloc(total_cnt * sizeof(double));
  if (b == NULL) {
    return -1;
  }


  c = (double *) malloc(total_cnt * 3 * sizeof(double));
  if (c == NULL) {
    return -1;
  }

  //memset(a, 0, total_cnt);
  if (argc > 1) {
    cnt = atoi(argv[1]);
  }

  if (argc > 2) {
    total_cnt = atoi(argv[2]);
  }

  printf("Total elements: %d,  Parallel cnt : %d\n", total_cnt, cnt);
  fflush(stdout);

  using std::chrono::steady_clock;
  using namespace std::chrono;


  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator (seed);  // mt19937 is a standard mersenne_twister_engine
  for (int i = 0; i < total_cnt; i++) {
    double r = generator() / ((double) (RAND_MAX));
    a[i] = r;
  }
  for (int i = 0; i < total_cnt; i++) {
    double r = generator() / ((double) (RAND_MAX));
    b[i] = r;
  }
  std::sort(a, a+total_cnt);
  std::sort(b, b+total_cnt);
  printf("Finished init \n");


  std::chrono::steady_clock::time_point starttime = std::chrono::steady_clock::now();
  clock_t start=clock();
  time_t st = time(NULL);
  sortAll(a, b, c, cnt);
  // sleep(1);

  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::now();
  clock_t end=clock();
  time_t et = time(NULL);

  fflush(stdout);
  duration<double> time_span = duration_cast<duration<double>>(endtime - starttime);
  printf("Clocks elapsed : %lf, Time : %lf, Wall time: %lf \n C++ Span : %lf\n", (double) (end - start), (double) (end-start) / (double) CLOCKS_PER_SEC, (double) (et-st), time_span.count());

//
//  // Retry with serial mode
//  for (int i = 0; i < total_cnt; i++) {
//    double r = generator() / ((double) (RAND_MAX));
//    a[i] = r;
//  }
//  for (int i = 0; i < total_cnt; i++) {
//    double r = generator() / ((double) (RAND_MAX));
//    b[i] = r;
//  }
//  std::sort(a, a+total_cnt);
//  std::sort(b, b+total_cnt);
//  printf("Serial Finished init \n");
//
//  starttime = std::chrono::steady_clock::now();
//  start=clock();
//  st = time(NULL);
//  std::merge(a, a+total_cnt, b, b+total_cnt, c);
//  // sleep(1);
//
//  endtime = std::chrono::steady_clock::now();
//  end=clock();
//  et = time(NULL);
//
//  fflush(stdout);
//  time_span = duration_cast<duration<double>>(endtime - starttime);
//  printf("Clocks elapsed : %lf, Time : %lf, Wall time: %lf \n C++ Span : %lf\n", (double) (end - start), (double) (end-start) / (double) CLOCKS_PER_SEC, (double) (et-st), time_span.count());

  return 0;
}

int main(int argc, char** argv) {
  char **arguments = (char **) malloc(sizeof(char *) * 8);
  arguments[0] = argv[0];
  arguments[1] = "1";
  trial(2, arguments);
  arguments[1] = "2";
  trial(2, arguments);
  arguments[1] = "4";
  trial(2, arguments);
  arguments[1] = "8";
  trial(2, arguments);
  arguments[1] = "16";
  trial(2, arguments);
}
