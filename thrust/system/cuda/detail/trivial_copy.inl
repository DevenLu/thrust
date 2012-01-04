/*
 *  Copyright 2008-2011 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <thrust/detail/config.h>
#include <thrust/system/cuda/detail/trivial_copy.h>
#include <thrust/system/cuda/detail/guarded_cuda_runtime_api.h>
#include <thrust/system_error.h>
#include <thrust/system/cuda_error.h>
#include <thrust/iterator/iterator_categories.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/system/cpp/detail/tag.h>
#include <thrust/system/cuda/detail/tag.h>
#include <thrust/detail/type_traits/pointer_traits.h>

namespace thrust
{
namespace system
{
namespace cuda
{
namespace detail
{

namespace detail
{

inline void checked_cudaMemcpy(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind)
{
  cudaError_t error = cudaMemcpy(dst,src,count,kind);
  if(error)
  {
    throw thrust::system_error(error, thrust::cuda_category());
  } // end error
} // end checked_cudaMemcpy()


template<typename SrcSpace,
         typename DstSpace>
  struct is_cpp_to_cuda
    : thrust::detail::integral_constant<
        bool,
        thrust::detail::is_convertible<SrcSpace, thrust::cpp::tag>::value &&
        thrust::detail::is_convertible<DstSpace, thrust::cuda::tag>::value
      >
{};


template<typename SrcSpace,
         typename DstSpace>
  struct is_cuda_to_cpp
    : thrust::detail::integral_constant<
        bool,
        thrust::detail::is_convertible<SrcSpace, thrust::cuda::tag>::value &&
        thrust::detail::is_convertible<DstSpace, thrust::cpp::tag>::value
      >
{};


template<typename SrcSpace,
         typename DstSpace>
  struct is_cuda_to_cuda
    : thrust::detail::integral_constant<
        bool,
        thrust::detail::is_convertible<SrcSpace, thrust::cuda::tag>::value &&
        thrust::detail::is_convertible<DstSpace, thrust::cuda::tag>::value
      >
{};


template<typename SrcSpace,
         typename DstSpace>
  struct cuda_memcpy_kind
    : thrust::detail::eval_if<
        is_cpp_to_cuda<SrcSpace,DstSpace>::value,
        thrust::detail::integral_constant<cudaMemcpyKind, cudaMemcpyHostToDevice>,

        thrust::detail::eval_if<
          is_cuda_to_cpp<SrcSpace,DstSpace>::value,
          thrust::detail::integral_constant<cudaMemcpyKind, cudaMemcpyDeviceToHost>,

          thrust::detail::eval_if<
            is_cuda_to_cuda<SrcSpace,DstSpace>::value,
            thrust::detail::integral_constant<cudaMemcpyKind, cudaMemcpyDeviceToDevice>,
            void
          >
        >
      >::type
{};


namespace trivial_copy_detail
{

template<typename Pointer>
  typename thrust::detail::pointer_traits<Pointer>::raw_pointer
    get(Pointer ptr)
{
  return thrust::detail::pointer_traits<Pointer>::get(ptr);
} // end get()

} // end trivial_copy_detail


} // end namespace detail


template<typename RandomAccessIterator1,
         typename Size,
         typename RandomAccessIterator2>
  void trivial_copy_n(RandomAccessIterator1 first,
                      Size n,
                      RandomAccessIterator2 result)
{
  typedef typename thrust::iterator_value<RandomAccessIterator1>::type T;

  typedef typename thrust::iterator_space<RandomAccessIterator1>::type SrcSpace;
  typedef typename thrust::iterator_space<RandomAccessIterator2>::type DstSpace;

  void *dst = detail::trivial_copy_detail::get(&*result);
  const void *src = detail::trivial_copy_detail::get(&*first);

  detail::checked_cudaMemcpy(dst, src, n * sizeof(T), detail::cuda_memcpy_kind<SrcSpace, DstSpace>::value);
}


} // end namespace detail
} // end namespace cuda
} // end namespace system
} // end namespace thrust
