#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <ctime>
#include <cuda_runtime_api.h>
#include <helper_cuda.h>
#include <helper_string.h>
#include <cooperative_groups.h>
#include <cassert>

namespace cg = cooperative_groups;
constexpr unsigned int SHARED_ARRAY_SIZE = 1024;
size_t nSamples = SHARED_ARRAY_SIZE;// << 14; // 2^24
using VALUE_TYPE = unsigned int;
template<typename T>
__device__ inline void swap_if_greater(T& value1, T& value2) {
  if(value1 <= value2) return;
  T t = value1;
  value1 = value2;
  value2 = t;
}

template<typename T>
__global__ void oddEvenBatcherSort(T* data) {
  cg::thread_block cta = cg::this_thread_block();
  __shared__ T shared_buffer[SHARED_ARRAY_SIZE];
  data += blockIdx.x * SHARED_ARRAY_SIZE + threadIdx.x;
  shared_buffer[threadIdx.x] = data[0];
  shared_buffer[threadIdx.x + SHARED_ARRAY_SIZE / 2] = data[SHARED_ARRAY_SIZE / 2];
  for (uint size = 2; size <= SHARED_ARRAY_SIZE; size *= 2) {
    uint stride = size / 2;
    uint offset = threadIdx.x & (stride - 1);

    cg::sync(cta);
    uint pos = 2 * threadIdx.x - offset;
    swap_if_greater(shared_buffer[pos], shared_buffer[pos + stride]);
    stride /= 2;

    for (; stride > 0; stride /= 2) {
      cg::sync(cta);
      uint pos = 2 * threadIdx.x - (threadIdx.x & (stride - 1));

      if (offset >= stride)
        swap_if_greater(shared_buffer[pos - stride], shared_buffer[pos]);
    }
  }

  cg::sync(cta);
  data[0] = shared_buffer[threadIdx.x];
  data[SHARED_ARRAY_SIZE / 2] = shared_buffer[threadIdx.x + SHARED_ARRAY_SIZE / 2];
}
template<typename T>
__global__ void oddEvenBatcherSortMerge(T* data, uint size, uint stride) {
  uint value_id = blockIdx.x * blockDim.x + threadIdx.x;
  uint pos = 2 * value_id - (value_id & (stride - 1));
  if (stride < size / 2) {
    uint offset = value_id & (size / 2 - 1);
    if (offset >= stride)
      swap_if_greater(data[pos - stride], data[pos]);
  } else {
    swap_if_greater(data[pos], data[pos + stride]);
  }
}

void cuda_assert(cudaError_t err, const char* text) {
	if(err != cudaSuccess) {
		std::cout << text << " (error code " << cudaGetErrorString(err)
              << ")!" << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

int main(int argc, const char* argv[]) {
  if(argc > 1) {
    auto shl = std::atoi(argv[1]);
    if(shl > 1 && shl <= 20)
      nSamples <<= shl;
  }
  size_t nBytes = nSamples * sizeof(VALUE_TYPE);

  VALUE_TYPE* host_data = (VALUE_TYPE*)malloc(nSamples * sizeof(VALUE_TYPE));
  std::mt19937_64 generator(time(nullptr));
  std::generate_n(host_data, nSamples, [&generator]() { return(generator()); });
  VALUE_TYPE* device_data = nullptr;
	cuda_assert(cudaMalloc((void**)&device_data, nSamples * sizeof(VALUE_TYPE)),
                          "Failed to allocate device memory!");
  cuda_assert(cudaMemcpy(device_data, host_data, nBytes,
                          cudaMemcpyHostToDevice),
                          "Failed to copy vector from host to device" );
  assert(nSamples >= SHARED_ARRAY_SIZE);
  assert(nSamples % SHARED_ARRAY_SIZE == 0);
  assert((nSamples & (nSamples - 1)) == 0);

  uint blockCount = nSamples / SHARED_ARRAY_SIZE;
  uint threadCount = SHARED_ARRAY_SIZE / 2;

  cuda_assert(cudaDeviceSynchronize(), "failed!");
  oddEvenBatcherSort<<<blockCount, threadCount>>>(device_data);
  cuda_assert(cudaGetLastError(), "Failed to launch kernel");
  if(blockCount > 1) {
    blockCount = nSamples * 2 / SHARED_ARRAY_SIZE;
    threadCount = SHARED_ARRAY_SIZE / 4;
    for(uint size = 2 * SHARED_ARRAY_SIZE; size <= nSamples; size *= 2) {
      for(unsigned stride = size / 2; stride > 0; stride >>= 1) {
        oddEvenBatcherSortMerge<<<blockCount, threadCount>>>(device_data, size, stride);
        cuda_assert(cudaGetLastError(), "Failed to launch kernel");
      }
    }
  }
  
  cuda_assert(cudaDeviceSynchronize(), "cudaDeviceSynchronize failed!");
	
  cuda_assert(cudaMemcpy(host_data, device_data, nBytes, cudaMemcpyDeviceToHost),
        "Failed to copy vector from device to host");

  if(std::is_sorted(host_data, host_data + nSamples))
    std::cout << "ok" << std::endl;
  else
    std::cout << "failed" << std::endl;
  
  cuda_assert(cudaFree(device_data), "Failed to free device memory");
  free(host_data);

  return(EXIT_SUCCESS);
}
