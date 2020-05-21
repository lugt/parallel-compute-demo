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

#define MAX_CNT 100000000

int *a = NULL;
int total_cnt = 0;
vector<int> *vc = NULL;
vector<int> *final = NULL;

typedef vector<int> dataType;
typedef vector<int> IVEC;

// Sort a specific part (slice) of data.
void sortPart(int *ptr, int start, int end) {
  vector<int>::iterator starter = vc->begin();
  vector<int>::iterator ending = vc->begin();
  starter += start;
  ending += end;
  std::sort(starter, ending);
}

inline int getkey(dataType &data, int pos) {
  return data[pos];
}

// Merge two blocks
inline void  Merge_one  (dataType &datal , int sizel , dataType &datar , int sizer , int *data2){
    int i1 = 0, i2 = 0, i3 = 0;
       
    while (i1 < sizel && i2 < sizer) {
      if (datal[i1] < datar[sizel + i2]) 
        data2[i3++] = datal[i1++];
      else 
        data2[i3++] = datar[sizel + (i2++)];
    }
	while(i1<sizel)
      data2[i3++] = datal[i1++];
	while(i2<sizer)
      data2[i3++] = datar[sizel + (i2++)];
}

// Use ideas similar to merge sort to do the merge between parts sorted.
inline void mergeAll(int *dest, IVEC &orig, int section_size) {
  int totalSections = MAX_CNT / section_size;
  int cursors[totalSections];
  int merged_size = section_size;

  
  for (int i = 0; i < totalSections - 1; i++) {
    //    cursors[i] = 0;
    // merge them all
    std::inplace_merge(orig.begin(),
                       orig.begin() + merged_size,
                       orig.begin() + merged_size + section_size); 
    merged_size += section_size;
    
    /*
     Merge_one(orig, merged_size, orig, section_size, dest);
    merged_size += section_size;
    for (int j = 0; j < merged_size; j++) {
      orig[j] = dest[j];
    }
    */
  }

  if(merged_size != MAX_CNT) {
    fprintf(stderr, "Merged size is not complete, %d", merged_size);
    exit(1);
  }
  
}

// Sort all datas in vc, and put int dest.
// Work in O(N * lgN)
void sortAll(int *dest, int cnt) {
  omp_lock_t alock;
  omp_init_lock(&alock);
  int section_size = (MAX_CNT / cnt); 
  omp_set_num_threads(cnt);
   
  // #pragma omp parallel for default(none) private(vc)  // for reduction(+:sum)
  int q = 0;
#pragma omp parallel for private(q)  // for reduction(+:sum)
  for (int n = 0; n < cnt; n++) {
    q = section_size * n;
    // Sort each part of the N-slice, O(N * (lgN - lgCount)) complexity
    sortPart(a, q, q + section_size);
    printf("N = %d, Sorting from %d to %d \n", n, q, q + section_size);
  }
  // Use merge-sort idea to merge the results, O(N) complexity
  mergeAll(dest, *vc, section_size);
}

int main(int argc, char **argv) {
  int cnt = 4;
  a = (int *) malloc(MAX_CNT * sizeof(int));
  if (a == NULL) {
    return -1;
  }
  //memset(a, 0, MAX_CNT);
  vc = new vector<int>();
  if (argc > 1) {
    cnt = atoi(argv[1]);
  }

  printf("Parallel cnt : %d\n", cnt);
  fflush(stdout);

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator (seed);  // mt19937 is a standard mersenne_twister_engine
  for (int i = 0; i < MAX_CNT; i++) {
    int r = generator();
    vc->push_back(r);
  }
  printf("Finished init \n");

  clock_t start=clock();
  time_t st = time(NULL);
  sortAll(a, cnt);
  clock_t end=clock();
  time_t et = time(NULL);

  fflush(stdout);
  printf("Clocks elapsed : %lf, Time : %lf, Wall time: %lf \n", (double) (end - start), (double) (end-start) / (double) CLOCKS_PER_SEC, (double) (et-st));

  /*
  // verification
  for (int i = 1; i < MAX_CNT; i++ ){
    if (a[i] < a[i-1]) {
      fprintf(stderr, "Found exception .... on i = %d\n", i);
      fflush(stderr);
      exit (-2);
    }
  }
  for (int i = MAX_CNT - 10; i > 0 && i < MAX_CNT; i++) {
    printf("[%d] : %d\n", i, (*vc)[i]);
  }
  */
  return 0;
}

|