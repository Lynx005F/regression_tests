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

int main()
{
  if (rt_cluster_id() != 0)
    return bench_cluster_forward(0);

  printf("Testing FPU from Core 0!\n");

  volatile float a, b, c, d;
  a = 2.0;
  b = 0.5;
  c = 1.0;

  float result;

  result = a * b - c;
  printf("0: FMSUB: %f \n", result);

  result = a * b;
  printf("1: MUL  : %f \n", result);

  result = a * b + c;
  printf("2: FMADD: %f \n", result);

  result = a + c;
  printf("3: ADD  : %f \n", result);

  // Currently unsupported
  // result = a / b;
  // printf("4: DIV  : %f \n", result);

  return 0;
}
