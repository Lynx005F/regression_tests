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
 * Author: Yvan Tortorella  <yvan.tortorella@unibo.it>
 * Author: Maurus Item  <itemm@student.ethz.ch>
 *
 * RedMulE SW test
 */

#include <stdint.h>
#include "stdio.h"
#include "archi_redmule.h"
#include "hal_redmule.h"
#include "pulp.h"


void main_fn(testresult_t *result, void (*start)(), void (*stop)());

testcase_t testcases[] = {
  { .name = "RedMulE Matrix Multiplication", .test = main_fn },
  {0, 0}
};

int done = 0;

int retval = 0;

void main_fn(testresult_t *result, void (*start)(), void (*stop)()) {

  volatile int errors = 0;
  unsigned int cluster_id = rt_cluster_id();
  unsigned int intc_data_correctable_cnt, redmule_data_correctable_cnt = 0;
  unsigned int intc_meta_correctable_cnt = 0;
  unsigned int intc_data_uncorrectable_cnt, redmule_data_uncorrectable_cnt = 0;
  unsigned int intc_meta_uncorrectable_cnt = 0;

  if(get_core_id() == 0){

    uint16_t m_size = M_SIZE;
    uint16_t n_size = N_SIZE;
    uint16_t k_size = K_SIZE;

    uint8_t *x_ext = x_inp;
    uint8_t *w_ext = w_inp;
    uint8_t *y_ext = y_inp;
    uint8_t *z_ext = z_oup;

    uint8_t volatile *x = (uint8_t volatile *) pi_l1_malloc(0, (2*m_size*n_size));
    uint8_t volatile *w = (uint8_t volatile *) pi_l1_malloc(0, (2*n_size*k_size));
    uint8_t volatile *y = (uint8_t volatile *) pi_l1_malloc(0, (2*m_size*k_size));
    uint8_t volatile *z = (uint8_t volatile *) pi_l1_malloc(0, (2*m_size*k_size));


    #ifdef USE_DMA
      #ifdef CHECK
        printf ("Using DMA");
      #endif

      volatile unsigned int dma_id = 0;
      dma_id = mchan_alloc();
      mchan_transfer((unsigned int) 2*(2*m_size*n_size),
                     (unsigned int) x_ext,
                     (unsigned int) x    );
      mchan_barrier(dma_id);
      mchan_free(dma_id);
    
      dma_id = mchan_alloc();
      mchan_transfer((unsigned int) 2*(2*n_size*k_size),
                     (unsigned int) w_ext,
                     (unsigned int) w    );
      mchan_barrier(dma_id);
      mchan_free(dma_id);
    
      dma_id = mchan_alloc();
      mchan_transfer((unsigned int) 2*(2*m_size*k_size),
                     (unsigned int) y_ext,
                     (unsigned int) y    );
      mchan_barrier(dma_id);
      mchan_free(dma_id);

      // Fill z buffer with inputs also - what data is in there doesn't matter
      // since it will get overwritten. But we need to write it once
      // so ECC does not trip!
      dma_id = mchan_alloc();
      mchan_transfer((unsigned int) 2*(2*m_size*k_size),
                     (unsigned int) y_ext,
                     (unsigned int) z    );
      mchan_barrier(dma_id);
    #else
      generate_test_data16((int) x, (int) w, (int) y, (int) m_size, (int) n_size, (int) k_size);

      // Fill z buffer with something to make sure no ECC error
      for (int i = 0; i < (2*m_size*k_size); i++) {
        z[i] = i;
      }

    #endif

    int gold_sum = 0, check_sum = 0;
    int i,j;

    int offload_id_tmp, offload_id;

    #ifdef STATS
      start();
    #endif

    // Enable RedMulE
    hwpe_cg_enable();

    hwpe_soft_clear();

    redmule_cfg ((uint32_t) x, (uint32_t) w, (uint32_t) y, (uint32_t) z, m_size, n_size, k_size, gemm_ops);

    // Start RedMulE operation
    hwpe_trigger_job();

    #ifdef STATS
      // Wait for end of computation
      redmule_evt_wait();
      stop();
    #else
      // Wait for end of computation
      redmule_evt_wait();
    #endif

    #ifdef CHECK
      // Check number of detected errors by ECC modules inside RedMulE
      redmule_data_correctable_cnt = redmule_get_data_correctable_count();
      redmule_data_uncorrectable_cnt = redmule_get_data_uncorrectable_count();

      // Disable RedMulE
      hwpe_cg_disable();

      errors = redmule16_compare_int(z, golden, m_size*k_size/2);

      *(int *) 0x1A1040A0 = errors;

      printf ("Terminated test with %d errors. See you!\n", errors);

      // Check number of detected errors by ECC modules inside interconnect
      intc_data_correctable_cnt = hwpe_hci_ecc_get_data_correctable_count(cluster_id);
      intc_meta_correctable_cnt = hwpe_hci_ecc_get_meta_correctable_count(cluster_id);
      intc_data_uncorrectable_cnt = hwpe_hci_ecc_get_data_uncorrectable_count(cluster_id);
      intc_meta_uncorrectable_cnt = hwpe_hci_ecc_get_meta_uncorrectable_count(cluster_id);
      for (int i = 0; i < 16; i++) {
        intc_meta_correctable_cnt += tcdm_scrubber_get_mismatch_count(cluster_id, i);
      }

      printf ("Data errors corrected inside RedMulE: %d. Data errors uncorrectable inside RedMulE: %d \n",
        redmule_data_correctable_cnt, redmule_data_uncorrectable_cnt);
      printf("Data errors corrected inside intc: %d. Data errors uncorrectable inside intc: %d\n",
        intc_data_correctable_cnt, intc_data_uncorrectable_cnt);
      printf("Meta errors corrected inside intc: %d. Meta errors uncorrectable inside intc: %d\n",
        intc_meta_correctable_cnt, intc_meta_uncorrectable_cnt);
    #else
      // Disable RedMulE
      hwpe_cg_disable();
    #endif
  }
  synch_barrier();
  result->errors = (errors != 0) && (redmule_data_uncorrectable_cnt==0 && intc_data_uncorrectable_cnt == 0 && intc_meta_uncorrectable_cnt == 0);
}

int main()
{

  if (rt_cluster_id() != 0)
    return bench_cluster_forward(0);

  int nbErrors = run_suite(testcases);

  synch_barrier();

  retval = nbErrors;

  return retval;
}
