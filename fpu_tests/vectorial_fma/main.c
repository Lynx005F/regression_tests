/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
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
 */

#include <stdio.h>
#include <pulp.h>

typedef float16 f16v __attribute__ ((vector_size (4)));

int main()
{ 
  if (rt_core_id() != 0)
    return bench_cluster_forward(0);

  int errors = 0;
  int time;

  // Test for f16v
  printf("Testing FP16 vectorial.\n");
  f16v a16 = {2.0f, 2.0f};
  f16v b16 = {0.5f, 0.5f};
  f16v c16 = {1.0f, 1.0f};
  f16v result16_0, result16_1, result16_2, result16_3;

  reset_timer(0);
  start_timer(0);
  /* // TODO find compiler that gets vfmac.r.h
  asm volatile (
    // FMSUB: result16a_0 = a16a * b16a - c16a
    "vfmac.r.h %0, %1, %2, %3\n"
    : "=r"(result16_0)
    : "r"(a16), "r"(b16), "r"(c16)
  );
  */

  asm volatile (
    // MUL: result16a_1 = a16a * b16a
    "vfmul.h %0, %1, %2\n"
    : "=r"(result16_1)
    : "r"(a16), "r"(b16)
  );

  /* // TODO find compiler that gets vfmac.h
  asm volatile (
    // FMADD: result16a_2 = a16a * b16a + c16a
    "vfmac.h %0, %1, %2, %3\n" 
    : "=r"(result16_2)
    : "r"(a16), "r"(b16), "r"(c16)
  );
  */

  asm volatile (
    // ADD: result16a_3 = a16a + c16a
    "vfadd.h %0, %1, %2\n"
    : "=r"(result16_3)
    : "r"(a16), "r"(c16)
  );
  stop_timer(0);

  /*
  for (int i = 0; i < 2; i++) {
    if (result16_0[i] != 0.0f) {
      printf("FMSUB failed for FP16 vectorial index %d, result is %04x\n", i, *(uint16_t*)&result16_0[i]);
      errors++;
    }
  }
  */

  for (int i = 0; i < 2; i++) {
    if (result16_1[i] != 1.0f) {
      printf("MUL failed for FP16 at vectorial index %d, result is %04x\n", i, *(uint16_t*)&result16_1[i]);
      errors++;
    }
  }

  /*
  for (int i = 0; i < 2; i++) {
    if (result16_2[i] != 2.0f) {
      printf("FMADD failed for FP16 vectorial at index %x, result is %04x\n", i, *(uint16_t*)&result16_2[i]);
      errors++;
    }
  }
  */

  for (int i = 0; i < 2; i++) {
    if (result16_3[i] != 3.0f) {
      printf("ADD failed for FP16 vectorial at index %d, result is %04x\n", i, *(uint16_t*)&result16_3[i]);
      errors++;
    }
  }

  time = get_time(0);

  printf("Test took %d cycle(s).\n", time);

  if (errors == 0) {
    printf("All tests passed!\n");
  } else {
    printf("%d test(s) failed.\n", errors);
  }

  return errors;
}
