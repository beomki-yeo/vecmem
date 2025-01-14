/*
 * VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */
#pragma once

// Local include(s).
#include "vecmem/containers/data/vector_buffer.hpp"
#include "vecmem/containers/data/vector_view.hpp"
#include "vecmem/memory/memory_resource.hpp"

namespace vecmem::cuda {

   /// Function copying a buffer from the host to the device
   template< typename TYPE >
   vecmem::data::vector_buffer< TYPE >
   copy_to_device( const vecmem::data::vector_view< TYPE >& host,
                   memory_resource& resource );

   /// Function copying a buffer from the device to the host
   template< typename TYPE >
   vecmem::data::vector_buffer< TYPE >
   copy_to_host( const vecmem::data::vector_view< TYPE >& device,
                 memory_resource& resource );

   /// Helper function for copying the contents of a 1-dimensional array
   template< typename TYPE >
   void copy( const vecmem::data::vector_view< TYPE >& from,
              vecmem::data::vector_view< TYPE >& to );

} // namespace vecmem::cuda

// Include the implementation.
#include "vecmem/utils/cuda/copy.ipp"
