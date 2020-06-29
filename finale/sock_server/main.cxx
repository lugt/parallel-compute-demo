//
// Created by xc5 on 6/11/20.
//

#include "http_examples.h"
#include "common_def.h"
#include "client.h"
#include "omp.h"

ELET *values = NULL;
const char *rand_file = NULL;

/**
 * Choosing which model to use from
 */
WorkingModule current_sortmode = NP_QUICKSORT;

namespace {

    void setUpTracer(const char *configFilePath) {
      auto configYAML = YAML::LoadFile(configFilePath);
      auto config = jaegertracing::Config::parse(configYAML);
      auto tracer = jaegertracing::Tracer::make(
        "os-finale", config, jaegertracing::logging::consoleLogger());
      opentracing::Tracer::InitGlobal(
        std::static_pointer_cast<opentracing::Tracer>(tracer));
    }

    void tracedInit(const std::unique_ptr<opentracing::Span> &parentSpan,
                    ELET_OFST &length) {

      Assert(rand_file != NULL, cerr << "No file to read from ....");
      ifstream ifs(rand_file);
      length = 30000;
      ELET_OFST real_len = length;
      Is_True(length > 0, << "length must be greater than 0, given : " << length);

      AS_CHILD_SPAN(span, "initialization", parentSpan);
      std::ostringstream oss;
      oss << "length : " << real_len;
      span->SetBaggageItem("length", oss.str());
      // Initializing the Array in memory (compare with malloc?)
      values = new ELET[real_len];
      AS_CHILD_SPAN(readSpan, "read-file-loop", span);
      for (ELET_OFST i = 0 ; i < real_len; i++) {
        ifs >> values[i];
      }
      if (false) {
        AS_CHILD_SPAN(randomSpan, "rand-loop", span);
        std::default_random_engine generator;
        std::uniform_int_distribution<ELET> distribution(DISTRIBUTE_MIN, DISTRIBUTE_MAX);
        auto dice = std::bind(distribution, generator);
        for (ELET_OFST i = 0; i < real_len; i++) {
          values[i] = dice();
        }
        randomSpan->Finish();
      }
    }

    void tracedReduce(SPTR parentSpan,
                      ELET *begin,
                      INT32 parallel_level,
                      ELET_OFST length) {
      std::string name = "traced-reduce";
      IFSORT(NP_QUICKSORT, name = "quick-sort-main")
      AS_CHILD_SPAN(span, name, parentSpan);
      std::ostringstream oss;
      oss << "length : " << length << ", Estimate : " << (log(length) * length);
      span->SetBaggageItem("params", oss.str());
      IFSORT(NP_QUICKSORT, tracedQuickSort(begin, length, parallel_level, span);)
      // IFSORT(NP_MERGESORT, tracedMergeSort(span);)
      span->Finish();
    }

    void tracedLoop(SPTR &parentSpan) {
      // start globla init of 50000
      ELET_OFST length = 30000;
      tracedInit(parentSpan, length);
      INT32 levels[] = {1, 3};

      for (INT32 level : levels) {
        for (ELET_OFST stepWidth = length; stepWidth <= length; stepWidth += (10 * 10000)) {
          tracedInit(parentSpan, stepWidth);
          tracedReduce(parentSpan, values, level, stepWidth); // stepWidth
        }
      }
      delete[] values;
    }


    void tracedFunction() {
      auto span = opentracing::Tracer::Global()->StartSpan("Main.tracedFunction");
      tracedLoop(span);
      span->Finish();
    }

}  // anonymous namespace

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <config-yaml-path> <rand-file-path>\n";
    return 1;
  }
  rand_file = argv[2];
  setUpTracer(argv[1]);
  tracedFunction();
  // Not stricly necessary to close tracer, but might flush any buffered
  // spans. See more details in opentracing::Tracer::Close() documentation.
  opentracing::Tracer::Global()->Close();
  return 0;
}
