/*
 * VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Local include(s).
#include "test_hip_containers_kernels.hpp"

// VecMem include(s).
#include "vecmem/memory/contiguous_memory_resource.hpp"
#include "vecmem/memory/hip/device_memory_resource.hpp"
#include "vecmem/memory/hip/host_memory_resource.hpp"
#include "vecmem/utils/hip/copy.hpp"

#include "vecmem/containers/array.hpp"
#include "vecmem/containers/vector.hpp"
#include "vecmem/containers/jagged_vector.hpp"

#include "vecmem/containers/data/jagged_vector_buffer.hpp"

// GoogleTest include(s).
#include <gtest/gtest.h>

// System include(s).
#include <set>

/// Test fixture for the on-device vecmem jagged container tests
class hip_jagged_containers_test : public testing::Test {

public:
   /// Constructor, setting up the input data for the tests.
   hip_jagged_containers_test()
   : m_mem(),
     m_vec( { vecmem::vector< int >( { 1, 2, 3, 4 }, &m_mem ),
              vecmem::vector< int >( { 5, 6 }, &m_mem ),
              vecmem::vector< int >( { 7, 8, 9, 10 }, &m_mem ),
              vecmem::vector< int >( { 11 }, &m_mem ),
              vecmem::vector< int >( &m_mem ),
              vecmem::vector< int >( { 12, 13, 14, 15, 16 }, &m_mem ) },
            &m_mem ),
     m_constants( m_mem ) {

      m_constants[ 0 ] = 2; m_constants[ 1 ] = 1;
   }

protected:
   /// Host (managed) memory resource
   vecmem::hip::host_memory_resource m_mem;
   /// The base vector to perform tests with
   vecmem::jagged_vector< int > m_vec;
   /// An array to use in the tests
   vecmem::array< int, 2 > m_constants;
};

/// Test a "linear" transformation using the host (managed) memory resource
TEST_F( hip_jagged_containers_test, mutate_in_kernel ) {

   // Create the data object describing the jagged vector.
   auto vec_data = vecmem::get_data( m_vec );

   // Run the linear transformation.
   linearTransform( vecmem::get_data( m_constants ), vec_data, vec_data );

   // Check the results.
   EXPECT_EQ( m_vec.at( 0 ).at( 0 ), 214 );
   EXPECT_EQ( m_vec.at( 0 ).at( 1 ), 5 );
   EXPECT_EQ( m_vec.at( 0 ).at( 2 ), 7 );
   EXPECT_EQ( m_vec.at( 0 ).at( 3 ), 9 );
   EXPECT_EQ( m_vec.at( 1 ).at( 0 ), 222 );
   EXPECT_EQ( m_vec.at( 1 ).at( 1 ), 13 );
   EXPECT_EQ( m_vec.at( 2 ).at( 0 ), 226 );
   EXPECT_EQ( m_vec.at( 2 ).at( 1 ), 17 );
   EXPECT_EQ( m_vec.at( 2 ).at( 2 ), 19 );
   EXPECT_EQ( m_vec.at( 2 ).at( 3 ), 21 );
   EXPECT_EQ( m_vec.at( 3 ).at( 0 ), 234 );
   EXPECT_EQ( m_vec.at( 5 ).at( 0 ), 236 );
   EXPECT_EQ( m_vec.at( 5 ).at( 1 ), 27 );
   EXPECT_EQ( m_vec.at( 5 ).at( 2 ), 29 );
   EXPECT_EQ( m_vec.at( 5 ).at( 3 ), 31 );
   EXPECT_EQ( m_vec.at( 5 ).at( 4 ), 33 );
}

/// Test a "linear" transformation while hand-managing the memory copies
TEST_F( hip_jagged_containers_test, set_in_kernel ) {

   // Helper object for performing memory copies.
   vecmem::hip::copy copy;

   // Create the output data on the host.
   vecmem::jagged_vector< int > output( &m_mem );
   output = m_vec; // Just to have it be set up with the correct sizes...
   auto output_data_host = vecmem::get_data( output );

   // Create the output data on the device.
   vecmem::hip::device_memory_resource device_resource;
   vecmem::data::jagged_vector_buffer< int >
       output_data_device( output_data_host, device_resource, &m_mem );
   copy.setup( output_data_device );

   // Run the linear transformation.
   linearTransform( copy.to( vecmem::get_data( m_constants ), device_resource,
                             vecmem::copy::type::host_to_device ),
                    copy.to( vecmem::get_data( m_vec ), device_resource,
                             &m_mem, vecmem::copy::type::host_to_device ),
                    output_data_device );
   copy( output_data_device, output_data_host,
         vecmem::copy::type::device_to_host );

   // Check the results.
   EXPECT_EQ( output[ 0 ][ 0 ], 214 );
   EXPECT_EQ( output[ 0 ][ 1 ], 5 );
   EXPECT_EQ( output[ 0 ][ 2 ], 7 );
   EXPECT_EQ( output[ 0 ][ 3 ], 9 );
   EXPECT_EQ( output[ 1 ][ 0 ], 222 );
   EXPECT_EQ( output[ 1 ][ 1 ], 13 );
   EXPECT_EQ( output[ 2 ][ 0 ], 226 );
   EXPECT_EQ( output[ 2 ][ 1 ], 17 );
   EXPECT_EQ( output[ 2 ][ 2 ], 19 );
   EXPECT_EQ( output[ 2 ][ 3 ], 21 );
   EXPECT_EQ( output[ 3 ][ 0 ], 234 );
   EXPECT_EQ( output[ 5 ][ 0 ], 236 );
   EXPECT_EQ( output[ 5 ][ 1 ], 27 );
   EXPECT_EQ( output[ 5 ][ 2 ], 29 );
   EXPECT_EQ( output[ 5 ][ 3 ], 31 );
   EXPECT_EQ( output[ 5 ][ 4 ], 33 );
}

/// Test a "linear" transformation while hand-managing the memory copies
TEST_F( hip_jagged_containers_test, set_in_contiguous_kernel ) {

   // Helper object for performing memory copies.
   vecmem::hip::copy copy;

   // Make the input data contiguous in memory.
   vecmem::contiguous_memory_resource cont_resource( m_mem, 16384 );
   vecmem::jagged_vector< int > input( &cont_resource );
   input = m_vec;

   // Create the output data on the host, in contiguous memory.
   vecmem::jagged_vector< int > output( &cont_resource );
   output = m_vec; // Just to have it be set up with the correct sizes...
   auto output_data_host = vecmem::get_data( output );

   // Create the output data on the device.
   vecmem::hip::device_memory_resource device_resource;
   vecmem::data::jagged_vector_buffer< int >
       output_data_device( output_data_host, device_resource, &m_mem );
   copy.setup( output_data_device );

   // Run the linear transformation.
   linearTransform( copy.to( vecmem::get_data( m_constants ),
                             device_resource ),
                    copy.to( vecmem::get_data( input ), device_resource,
                             &m_mem ),
                    output_data_device );
   copy( output_data_device, output_data_host );

   // Check the results.
   EXPECT_EQ( output[ 0 ][ 0 ], 214 );
   EXPECT_EQ( output[ 0 ][ 1 ], 5 );
   EXPECT_EQ( output[ 0 ][ 2 ], 7 );
   EXPECT_EQ( output[ 0 ][ 3 ], 9 );
   EXPECT_EQ( output[ 1 ][ 0 ], 222 );
   EXPECT_EQ( output[ 1 ][ 1 ], 13 );
   EXPECT_EQ( output[ 2 ][ 0 ], 226 );
   EXPECT_EQ( output[ 2 ][ 1 ], 17 );
   EXPECT_EQ( output[ 2 ][ 2 ], 19 );
   EXPECT_EQ( output[ 2 ][ 3 ], 21 );
   EXPECT_EQ( output[ 3 ][ 0 ], 234 );
   EXPECT_EQ( output[ 5 ][ 0 ], 236 );
   EXPECT_EQ( output[ 5 ][ 1 ], 27 );
   EXPECT_EQ( output[ 5 ][ 2 ], 29 );
   EXPECT_EQ( output[ 5 ][ 3 ], 31 );
   EXPECT_EQ( output[ 5 ][ 4 ], 33 );
}
