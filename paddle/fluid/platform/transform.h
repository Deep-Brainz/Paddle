/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include "paddle/fluid/platform/device_context.h"
#include "paddle/fluid/platform/enforce.h"
#include "paddle/fluid/platform/hostdevice.h"
#include "paddle/fluid/platform/place.h"

#include <algorithm>
#include <type_traits>
#ifdef __NVCC__
#include <thrust/execution_policy.h>
#include <thrust/transform.h>
#include "paddle/fluid/platform/details/device_ptr_cast.h"
#endif

namespace paddle {
namespace platform {

// Transform on host or device. It provides the same API in std library.
template <typename DeviceContext>
struct Transform {
  template <typename InputIter, typename OutputIter, typename UnaryOperation>
  void operator()(const DeviceContext& context, InputIter first, InputIter last,
                  OutputIter result, UnaryOperation op);

  template <typename InputIter1, typename InputIter2, typename OutputIter,
            typename BinaryOperation>
  void operator()(const DeviceContext& context, InputIter1 first1,
                  InputIter1 last1, InputIter2 first2, OutputIter result,
                  BinaryOperation op);
};

template <>
struct Transform<platform::CPUDeviceContext> {
  template <typename InputIter, typename OutputIter, typename UnaryOperation>
  void operator()(const platform::CPUDeviceContext& context, InputIter first,
                  InputIter last, OutputIter result, UnaryOperation op) {
    std::transform(first, last, result, op);
  }

  template <typename InputIter1, typename InputIter2, typename OutputIter,
            typename BinaryOperation>
  void operator()(const platform::CPUDeviceContext& context, InputIter1 first1,
                  InputIter1 last1, InputIter2 first2, OutputIter result,
                  BinaryOperation op) {
    std::transform(first1, last1, first2, result, op);
  }
};

#ifdef __NVCC__
template <>
struct Transform<platform::CUDADeviceContext> {
  template <typename InputIter, typename OutputIter, typename UnaryOperation>
  void operator()(const platform::CUDADeviceContext& context, InputIter first,
                  InputIter last, OutputIter result, UnaryOperation op) {
    auto place = context.GetPlace();
    PADDLE_ENFORCE(is_gpu_place(place), "It must use GPU place.");
    thrust::transform(thrust::cuda::par.on(context.stream()),
                      details::DevPtrCast(first), details::DevPtrCast(last),
                      details::DevPtrCast(result), op);
  }

  template <typename InputIter1, typename InputIter2, typename OutputIter,
            typename BinaryOperation>
  void operator()(const platform::CUDADeviceContext& context, InputIter1 first1,
                  InputIter1 last1, InputIter2 first2, OutputIter result,
                  BinaryOperation op) {
    auto place = context.GetPlace();
    PADDLE_ENFORCE(is_gpu_place(place), "It must use GPU place.");
    thrust::transform(thrust::cuda::par.on(context.stream()),
                      details::DevPtrCast(first1), details::DevPtrCast(last1),
                      details::DevPtrCast(first2), details::DevPtrCast(result),
                      op);
  }
};
#endif

}  // namespace platform
}  // namespace paddle
