/* Copyright (C) 2015 INRA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <mitm/mitm.hpp>
#include "internal.hpp"
#include <vector>
#include <iostream>
#include <cuda.h>

namespace mitm {

typedef signed char int8;

__global__
void
initialize(int8 *x, std::size_t N)
{
  x[threadIdx.x] = threadIdx.x;
}


void
gpgpu_properties_show()
{
  int nDevices;

  cudaError_t err = cudaGetDeviceCount(&nDevices);
  if (err != cudaSuccess) {
    std::cerr << "GPGPU initialization fail: "
              << cudaGetErrorString(err) << "\n";
  } else {
    for (int i = 0; i < nDevices; i++) {
      cudaDeviceProp prop;
      cudaGetDeviceProperties(&prop, i);

      std::cout << "Device " << i
                << "\n---------------------------------\n"
                << "\nMajor revision number:         " << prop.major
                << "\nMinor revision number:         " << prop.minor
                << "\nName:                          " << prop.name
                << "\nTotal global memory:           " << prop.totalGlobalMem
                << " (" << (prop.totalGlobalMem / (1024.0 * 1024)) << " Gb)"
                << "\nTotal shared memory per block: "
                << prop.sharedMemPerBlock
                << "\nTotal registers per block:     " << prop.regsPerBlock
                << "\nWarp size:                     " << prop.warpSize
                << "\nMaximum memory pitch:          " << prop.memPitch
                << " (" << (prop.memPitch / (1024.0 * 1024)) << " Gb)"
                << "\nMaximum threads per block:     "
                << prop.maxThreadsPerBlock
                << "\nMaximum dimension 0 of block:  "
                << prop.maxThreadsDim[0]
                << "\nMaximum dimension 1 of block:  "
                << prop.maxThreadsDim[1]
                << "\nMaximum dimension 2 of block:  "
                << prop.maxThreadsDim[2]
                << "\nMaximum dimension 0 of grid:   " << prop.maxGridSize[0]
                << "\nMaximum dimension 1 of grid:   " << prop.maxGridSize[1]
                << "\nMaximum dimension 2 of grid:   " << prop.maxGridSize[2]
                << "\nClock rate:                    " << prop.clockRate
                << "\nTotal constant memory:         " << prop.totalConstMem
                << "\nTexture alignment:             "
                << prop.textureAlignment
                << "\nConcurrent copy and execution: "
                << (prop.deviceOverlap ? "Yes" : "No")
                << "\nNumber of multiprocessors:     "
                << prop.multiProcessorCount
                << "\nKernel execution timeout:      "
                << (prop.kernelExecTimeoutEnabled ? "Yes" : "No")
                << '\n';
    }
  }
}

result
heuristic_algorithm_gpgu(const SimpleState &s, index limit,
                         mitm::real kappa, mitm::real delta,
			 mitm::real theta)
{
(void)s;
(void)limit;
(void)kappa;
(void)delta;
  std::vector <int8> s_x(100, 0);

  std::cout << "Run in GPGPU\n";
  gpgpu_properties_show();

  int8 *x;

  cudaMalloc((void**)&x, s_x.size() * sizeof(int8_t));
  cudaMemcpy(x, s_x.data(), s_x.size() * sizeof(int8_t), cudaMemcpyHostToDevice);

  int block_size = 4;
  int n_blocks = s_x.size() /block_size + (s_x.size() % block_size == 0 ? 0:1);

  // initialize<<<n_blocks, block_size>>>(x, s_x.size());

  cudaMemcpy(s_x.data(), x, s_x.size() * sizeof(int8_t), cudaMemcpyDeviceToHost);
  cudaFree(x);

  return result();
}

}
