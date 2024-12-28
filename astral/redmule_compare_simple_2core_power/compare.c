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
#include "inc/tensor_dim.h"
#include "inc/a.h"
#include "inc/b.h"
#include "inc/golden.h"
#include "pulp.h"


void main_fn(testresult_t *result, void (*start)(), void (*stop)());

testcase_t testcases[] = {
  { .name = "RedMulE Result Comparison", .test = main_fn },
  {0, 0}
};

int done = 0;

int retval = 0;

int redmule_compare(uint32_t *actual_z, uint32_t *golden_z, int len) {

  uint32_t actual_word = 0;
  uint32_t golden_word = 0;

  int blockSize = (len+NUM_CORES-1)/NUM_CORES;
  int start = get_core_id()*blockSize;
  int end = start + blockSize < len? start + blockSize : len;

  for (int i=start; i<end; i++) {
    actual_word = *(actual_z+i);
    golden_word = *(golden_z+i);

    if (actual_word != golden_word) {
      return 1;
    }
  }
  return 0;
}

void main_fn(testresult_t *result, void (*start)(), void (*stop)()) {
    uint16_t m_size = M_SIZE;
    uint16_t n_size = N_SIZE;
    uint16_t k_size = K_SIZE;

    int different;
    int error;

    #ifdef STATS
      start();
    #endif

    different = redmule_compare(a, b, m_size*k_size/2);

    #ifdef STATS
      stop();
    #endif

    #ifdef CHECK
      if(get_core_id() == 0) {
        if (different == golden) {
          printf("Comparison terminated correctly!\n");
          error = 0;
        } else {
          printf("Comparison terminated incorrectly!\n");
          error = 1;
        }
      }
    #endif

  result->errors = error;
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

  retval = nbErrors;

  return retval;
}
