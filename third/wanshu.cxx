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


int TOTAL=1000;
double *a = NULL;
double *b = NULL;
int total_cnt = 0;
vector<int> *vc = NULL;
vector<int> *final = NULL;

// Add in the numbers prepending part
// Work in O(N)
inline void addAll(double *dest, int cnt) {
  int section_size = (TOTAL / cnt);

  int tid, i, j;
  omp_set_num_threads(cnt);
  double sum = 0;

#ifdef VERIFY
  b[0] = a[0];
  for (int i = 0 ; i < TOTAL; i++) {
    b[i] = a[i] + b[i - 1];
  }
#endif
  
  // Add in each part
#pragma omp parallel private(tid, j) shared(a, section_size)
  {
    tid = omp_get_thread_num();
    // printf("Running thread %d\n", tid);
    for(j = 0; j < section_size - 1; j++){
      a[tid*section_size + j+1] += a[tid * section_size + j];
    }
  }

  // Serialize: Add the section total to the subsequent section's last item
  for(i = 2; i < cnt + 1; i++){
    a[i*section_size-1] += a[(i-1)*section_size-1];
  }
    
  //Add the subsotal to all following items
#pragma omp parallel private(tid,j) shared(a)
  {
    tid = omp_get_thread_num();
    if(tid != 0){
      for(j = 0;j < section_size-1; j++){
        a[tid*section_size + j] += a [tid * section_size - 1];
      }
    }
  }

  // Remaining elements
  if(TOTAL % cnt != 0){
    for(i = cnt*section_size; i < TOTAL; i++)
    {
        a[i] +=a[i-1];
    }
  }
  
}

int main(int argc, char **argv) {
  int cnt = 4;
  //memset(a, 0, TOTAL);
  vc = new vector<int>();
  if (argc > 1) {
    cnt = atoi(argv[1]);
  }
  if (argc > 2) {
    TOTAL = atoi(argv[2]);
  }
  a = (double *) malloc(TOTAL * sizeof(double));
  b = (double *) malloc(TOTAL * sizeof(double));
  if (a == NULL || b == NULL) {
    return -1;
  }

  printf("Parallel cnt : %d, Total : %d\n", cnt, TOTAL);
  fflush(stdout);

  using std::chrono::steady_clock;
  using namespace std::chrono;
  
  
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator (seed);  // mt19937 is a standard mersenne_twister_engine
  for (int i = 0; i < TOTAL; i++) {
    double r = generator() / ((double) (RAND_MAX));
    a[i] = r;
  }
  printf("Finished init \n");

  

  std::chrono::steady_clock::time_point starttime = std::chrono::steady_clock::now();  
  clock_t start=clock();
  time_t st = time(NULL);
  addAll(a, cnt);
  // sleep(1);
  
  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::now();
  clock_t end=clock();
  time_t et = time(NULL);

  fflush(stdout);
  duration<double> time_span = duration_cast<duration<double>>(endtime - starttime);
  printf("Clocks elapsed : %lf, Time : %lf, Wall time: %lf \n C++ Span : %lf\n", (double) (end - start), (double) (end-start) / (double) CLOCKS_PER_SEC, (double) (et-st), time_span.count());

  /*
  // verification
  for (int i = 1; i < TOTAL; i++ ){
    // printf("%d, a=[%lf], b=[%lf]\n", i, a[i], b[i]);
    if (a[i] < a[i-1]
#ifdef VERIFY
        || abs(b[i] - a[i]) > 0.00001
#endif        
        ) {
      fprintf(stderr, "Found exception .... on i = %d\n", i);
      fflush(stderr);
      exit (-2);
    }
  }
  printf("Verification passed. \n");
  for (int i = TOTAL - 10; i > 0 && i < TOTAL; i++) {
    printf("[%d] : %lf\n", i, a[i]);
  }
  */
  return 0;
}

