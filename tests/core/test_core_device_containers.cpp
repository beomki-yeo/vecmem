/** VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Local include(s).
#include "vecmem/containers/data/jagged_vector_buffer.hpp"
#include "vecmem/containers/data/jagged_vector_view.hpp"
#include "vecmem/containers/data/vector_buffer.hpp"
#include "vecmem/containers/data/vector_view.hpp"
#include "vecmem/containers/device_vector.hpp"
#include "vecmem/containers/jagged_vector.hpp"
#include "vecmem/containers/vector.hpp"
#include "vecmem/memory/contiguous_memory_resource.hpp"
#include "vecmem/memory/host_memory_resource.hpp"
#include "vecmem/utils/copy.hpp"

// GoogleTest include(s).
#include <gtest/gtest.h>

// System include(s).
#include <cstring>
#include <type_traits>

/// Test case for the custom device container types
class core_device_container_test : public testing::Test {

    protected:
    /// Memory resource for the test(s)
    vecmem::host_memory_resource m_resource;
    /// Helper object for the memory copies.
    vecmem::copy m_copy;

};  // class core_device_container_test

/// Test that the "simple" data types are trivially constructible.
TEST_F(core_device_container_test, trivial_construct) {

    EXPECT_TRUE(std::is_trivially_default_constructible<
                vecmem::data::jagged_vector_view<const int> >());
    EXPECT_TRUE(std::is_trivially_constructible<
                vecmem::data::jagged_vector_view<const int> >());
    EXPECT_TRUE(std::is_trivially_copy_constructible<
                vecmem::data::jagged_vector_view<const int> >());

    EXPECT_TRUE(std::is_trivially_default_constructible<
                vecmem::data::jagged_vector_view<int> >());
    EXPECT_TRUE(std::is_trivially_constructible<
                vecmem::data::jagged_vector_view<int> >());
    EXPECT_TRUE(std::is_trivially_copy_constructible<
                vecmem::data::jagged_vector_view<int> >());

    EXPECT_TRUE(std::is_trivially_default_constructible<
                vecmem::data::vector_view<const int> >());
    EXPECT_TRUE(std::is_trivially_constructible<
                vecmem::data::vector_view<const int> >());
    EXPECT_TRUE(std::is_trivially_copy_constructible<
                vecmem::data::vector_view<const int> >());

    EXPECT_TRUE(std::is_trivially_default_constructible<
                vecmem::data::vector_view<int> >());
    EXPECT_TRUE(
        std::is_trivially_constructible<vecmem::data::vector_view<int> >());
    EXPECT_TRUE(std::is_trivially_copy_constructible<
                vecmem::data::vector_view<int> >());
}

/// Test(s) for @c vecmem::data::vector_buffer
TEST_F(core_device_container_test, vector_buffer) {

    // Create a dummy vector in regular host memory.
    std::vector<int> host_vector{1, 2, 3, 4, 5};
    auto host_data = vecmem::get_data(host_vector);

    // Create an "owning copy" of the host vector's memory.
    vecmem::data::vector_buffer<int> device_data(host_data.size(), m_resource);
    m_copy(vecmem::get_data(host_vector), device_data);

    // Do some basic tests.
    EXPECT_EQ(device_data.size(), host_vector.size());
    EXPECT_EQ(memcmp(host_data.ptr(), device_data.ptr(),
                     host_data.size() * sizeof(int)),
              0);
}

/// Test(s) for @c vecmem::data::jagged_vector_buffer
TEST_F(core_device_container_test, jagged_vector_buffer) {

    // Create a dummy jagged vector in regular host memory.
    std::vector<std::vector<int> > host_vector{{1, 2, 3, 4, 5},
                                               {6, 7},
                                               {8, 9, 10, 11},
                                               {12, 13, 14, 15, 16, 17, 18},
                                               {},
                                               {19, 20}};
    auto host_data = vecmem::get_data(host_vector, &m_resource);

    // Set up an "alternative" memory resource for the test.
    vecmem::contiguous_memory_resource cresource(m_resource, 16384);

    // Create a buffer to hold the same data.
    vecmem::data::jagged_vector_buffer<int> device_data1(host_data, m_resource);
    vecmem::data::jagged_vector_buffer<int> device_data2(host_data, m_resource,
                                                         &cresource);

    // Test the internal state of the buffer.
    EXPECT_EQ(device_data1.m_ptr, device_data1.host_ptr());
    EXPECT_EQ(device_data1.m_size, host_vector.size());
    EXPECT_NE(device_data2.m_ptr, device_data2.host_ptr());
    EXPECT_EQ(device_data2.m_size, host_vector.size());
    for (std::size_t i = 0; i < host_vector.size(); ++i) {
        EXPECT_EQ(device_data1.host_ptr()[i].size(), host_vector[i].size());
        EXPECT_EQ(device_data2.host_ptr()[i].size(), host_vector[i].size());
    }
    for (std::size_t i = 0; i < host_vector.size() - 1; ++i) {
        EXPECT_EQ(device_data1.host_ptr()[i].ptr() + host_vector[i].size(),
                  device_data1.host_ptr()[i + 1].ptr());
        EXPECT_EQ(device_data2.host_ptr()[i].ptr() + host_vector[i].size(),
                  device_data2.host_ptr()[i + 1].ptr());
    }
}

/// Test(s) for a "resizable" @c vecmem::data::vector_buffer
TEST_F(core_device_container_test, resizable_vector_buffer) {

    // Create an input vector in regular host memory.
    std::vector<int> host_vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Create a resizable buffer from that data.
    static constexpr std::size_t BUFFER_SIZE = 100;
    vecmem::data::vector_buffer<int> resizable_buffer(
        BUFFER_SIZE, host_vector.size(), m_resource);
    m_copy.setup(resizable_buffer);
    EXPECT_EQ(resizable_buffer.capacity(), BUFFER_SIZE);
    m_copy(vecmem::get_data(host_vector), resizable_buffer);
    EXPECT_EQ(resizable_buffer.size(), host_vector.size());

    // Create a "device vector" on top of that resizable data.
    vecmem::device_vector<int> device_vector(resizable_buffer);

    // Perform some simple tests as a start.
    EXPECT_EQ(device_vector.size(), host_vector.size());
    EXPECT_EQ(device_vector.capacity(), BUFFER_SIZE);
    EXPECT_EQ(device_vector.max_size(), BUFFER_SIZE);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(device_vector[i], i + 1);
    }

    // Modify the device vector in different ways, and check that it would work
    // as expected.
    device_vector.clear();
    EXPECT_EQ(device_vector.size(), 0);

    device_vector.push_back(10);
    EXPECT_EQ(device_vector.size(), 1);
    EXPECT_EQ(device_vector.at(0), 10);

    device_vector.emplace_back(15);
    EXPECT_EQ(device_vector.size(), 2);
    EXPECT_EQ(device_vector.back(), 15);

    device_vector.assign(20, 123);
    EXPECT_EQ(device_vector.size(), 20);
    for (int value : device_vector) {
        EXPECT_EQ(value, 123);
    }

    device_vector.resize(40, 234);
    EXPECT_EQ(device_vector.size(), 40);
    for (std::size_t i = 0; i < 20; ++i) {
        EXPECT_EQ(device_vector[i], 123);
    }
    for (std::size_t i = 20; i < 40; ++i) {
        EXPECT_EQ(device_vector[i], 234);
    }
    device_vector.resize(25);
    EXPECT_EQ(device_vector.size(), 25);
    for (std::size_t i = 0; i < 20; ++i) {
        EXPECT_EQ(device_vector[i], 123);
    }
    for (std::size_t i = 20; i < 25; ++i) {
        EXPECT_EQ(device_vector[i], 234);
    }

    device_vector.pop_back();
    EXPECT_EQ(device_vector.size(), 24);
    for (std::size_t i = 0; i < 20; ++i) {
        EXPECT_EQ(device_vector[i], 123);
    }
    for (std::size_t i = 20; i < 24; ++i) {
        EXPECT_EQ(device_vector[i], 234);
    }

    // Copy the modified data back into the "host vector", and check if that
    // succeeded.
    m_copy(resizable_buffer, host_vector);
    EXPECT_EQ(host_vector.size(), 24);
    for (std::size_t i = 0; i < 20; ++i) {
        EXPECT_EQ(host_vector[i], 123);
    }
    for (std::size_t i = 20; i < 24; ++i) {
        EXPECT_EQ(host_vector[i], 234);
    }
}
