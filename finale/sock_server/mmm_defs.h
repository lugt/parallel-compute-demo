//
// Created by xc5 on 6/24/20.
//

#ifndef MEMORY_MMM_DEFS_H
#define MEMORY_MMM_DEFS_H

#include "common_def.h"
#include <mutex>
#include <vector>
#include <map>
#include <fstream>
#include <vector>
#include <memory>

using namespace std;

#define MEM_SIZE 128 * 1024 * 1024
#define BLOCK_SIZE 256

#define Assert(cond, out) {if (! (cond) ) { printf("\n\n### Assertion failure at %s : %d\n", __FILE__, __LINE__); out; exit(1); } };
using std::shared_ptr;

class MSTRING {
public:
    UINT32 length = 0;
    UINT32 ofst  = 0;
    char  *data = nullptr;
};

using TPNT = std::chrono::steady_clock::time_point;
using std::chrono::steady_clock;

class ALLOC_INFO {
public:
    UINT32 mem_id = 0;
    UINT32 length = 0;
    BOOL   in_memory = true;
    UINT32 block_used = 0;
    UINT32 file_ofst = 0;
    UINT32 start_block = 0;
    UINT32 block_count = 0;
    TPNT last_used;
    BOOL replacing;
    ALLOC_INFO(UINT32 mem_id, UINT32 length, UINT32 start_block, UINT32 block_count) {
      this->mem_id = mem_id;
      this->length = length;
      this->last_used = std::chrono::steady_clock::now();
      this->file_ofst = 0;
      this->block_count = block_count;
      this->in_memory = true;
      this->replacing = false;
      this->start_block = start_block;
    }

};

using INFOPTR = shared_ptr<ALLOC_INFO>;
using STRPTR = shared_ptr<MSTRING>;

#endif //MEMORY_MMM_DEFS_H
