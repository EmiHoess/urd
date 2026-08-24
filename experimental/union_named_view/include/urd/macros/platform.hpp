#pragma once

// __device_host__ — marks a function callable from both CUDA device and host.
// __host__        — marks a function as host-only.
// Both expand to nothing when not compiling with CUDA.

#ifdef __CUDACC__
#  include <cuda.h>
#  include <cuda_runtime.h>
#  define __device_host__ __device__ __host__
#else
#  ifndef __device_host__
#    define __device_host__
#  endif
#  ifndef __host__
#    define __host__
#  endif
#endif
