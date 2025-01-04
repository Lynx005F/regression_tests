/*
 * Copyright (C) 2022-2023 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Author: Maurus Item  <itemm@student.ethz.ch>
 *
 * Comparison Benchmark for non-redundant redmule
 */

#include <stdint.h>
#include "stdio.h"
#include "pulp.h"
#include "data.h"

L1_DATA int out[LEN] __attribute__ ((aligned (4)));

void main_fn(testresult_t *result, void (*start)(), void (*stop)());

testcase_t testcases[] = {
  { .name = "Julia Row", .test = main_fn },
  {0, 0}
};


int growth(float cx, float cy, float max_abs, int max_iter) {
    float abs = 0;
    int iter = 0;

    float x = 0;
    float y = 0;

    float x_squared, y_squared, y_next;

    while (abs <= max_abs && iter < max_iter) {
        y_squared = y * y;
        x_squared = x * x;
        y_next = 2 * x * y + cy;
        abs = x_squared + y_squared;
        x = x_squared - y_squared + cx;
        y = y_next;
        iter = iter + 1;
    }
    
    return iter;
}

void growth_chunk() {
  uint16_t len = LEN;
  float step = 0.01;

  int blockSize = (len+NUM_CORES-1)/NUM_CORES;
  int start = get_core_id()*blockSize;
  int end = start + blockSize < len? start + blockSize : len;

  for (int i = start; i < end; i++) {
    out[i] = growth(i * step, 0.42, 100000000, 300);
  }
}

int check_result(){
  int errors = 0;
  for (int i=0; i<LEN; i++) {
    if (out[i] != golden[i]) {
      errors += 1;
    }
  }
  return errors;
}

void main_fn(testresult_t *result, void (*start)(), void (*stop)()) {

  #ifdef STATS
    start();
  #endif

  growth_chunk();

  #ifdef STATS
    stop();
  #endif

  #ifdef CHECK
    result->errors = check_result();
  #else
    result->errors = 0;
  #endif
}

int main()
{

  if (rt_cluster_id() != 0)
    return bench_cluster_forward(0);

  #ifdef REDUNDANCY
      hmr_self_enable_dmr();
      hmr_set_dmr_config_all(0    , // Core ID
                             true , // Rapid recovery enabled
                             true , // Setback enabled
                             false); // Synch req
      printf("Available Config: %x\n", hmr_get_available_config(rt_cluster_id()));
      printf("after setup: %x\n", hmr_get_active_cores(rt_cluster_id()));

      hmr_setup_barrier(hmr_get_active_cores(0));
  #endif

  int nbErrors = run_suite(testcases);

  synch_barrier();

  return nbErrors;
}
