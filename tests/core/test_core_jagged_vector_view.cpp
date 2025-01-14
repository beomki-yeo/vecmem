/*
 * VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#include "vecmem/memory/host_memory_resource.hpp"
#include "vecmem/containers/vector.hpp"
#include "vecmem/containers/jagged_vector.hpp"
#include "vecmem/containers/jagged_device_vector.hpp"
#include "vecmem/containers/data/jagged_vector_data.hpp"
#include "vecmem/containers/data/jagged_vector_view.hpp"

#include <gtest/gtest.h>

class core_jagged_vector_view_test : public testing::Test {
    protected:
    vecmem::host_memory_resource m_mem;
    vecmem::jagged_vector<int> m_vec;
    vecmem::data::jagged_vector_data<int> m_data;
    vecmem::jagged_device_vector<int> m_jag;

    core_jagged_vector_view_test(
        void
    ) :
        m_vec({
            vecmem::vector<int>({1, 2, 3, 4}, &m_mem),
            vecmem::vector<int>({5, 6}, &m_mem),
            vecmem::vector<int>({7, 8, 9, 10}, &m_mem),
            vecmem::vector<int>({11}, &m_mem),
            vecmem::vector<int>(&m_mem),
            vecmem::vector<int>({12, 13, 14, 15, 16}, &m_mem)
        }, &m_mem),
        m_data(m_vec, &m_mem),
        m_jag(m_data)
    {
    }
};

TEST_F(core_jagged_vector_view_test, top_level_size) {
    EXPECT_EQ(m_jag.size(), 6);
}

TEST_F(core_jagged_vector_view_test, row_size) {
    EXPECT_EQ(m_jag.at(0).size(), 4);
    EXPECT_EQ(m_jag.at(1).size(), 2);
    EXPECT_EQ(m_jag.at(2).size(), 4);
    EXPECT_EQ(m_jag.at(3).size(), 1);
    EXPECT_EQ(m_jag.at(4).size(), 0);
    EXPECT_EQ(m_jag.at(5).size(), 5);
}

TEST_F(core_jagged_vector_view_test, two_d_access) {
    EXPECT_EQ(m_jag.at(0, 0), 1);
    EXPECT_EQ(m_jag.at(0, 1), 2);
    EXPECT_EQ(m_jag.at(0, 2), 3);
    EXPECT_EQ(m_jag.at(0, 3), 4);
    EXPECT_EQ(m_jag.at(1, 0), 5);
    EXPECT_EQ(m_jag.at(1, 1), 6);
    EXPECT_EQ(m_jag.at(2, 0), 7);
    EXPECT_EQ(m_jag.at(2, 1), 8);
    EXPECT_EQ(m_jag.at(2, 2), 9);
    EXPECT_EQ(m_jag.at(2, 3), 10);
}

TEST_F(core_jagged_vector_view_test, two_d_access_const) {
    EXPECT_EQ(m_jag.at(0, 0), 1);
    EXPECT_EQ(m_jag.at(0, 1), 2);
    EXPECT_EQ(m_jag.at(0, 2), 3);
    EXPECT_EQ(m_jag.at(0, 3), 4);
    EXPECT_EQ(m_jag.at(1, 0), 5);
    EXPECT_EQ(m_jag.at(1, 1), 6);
    EXPECT_EQ(m_jag.at(2, 0), 7);
    EXPECT_EQ(m_jag.at(2, 1), 8);
    EXPECT_EQ(m_jag.at(2, 2), 9);
    EXPECT_EQ(m_jag.at(2, 3), 10);
}

TEST_F(core_jagged_vector_view_test, mutate) {
    m_jag.at(0, 0) *= 2;
    m_jag.at(0, 1) *= 2;
    m_jag.at(0, 2) *= 2;
    m_jag.at(0, 3) *= 2;
    m_jag.at(1, 0) *= 2;
    m_jag.at(1, 1) *= 2;
    m_jag.at(2, 0) *= 2;
    m_jag.at(2, 1) *= 2;
    m_jag.at(2, 2) *= 2;
    m_jag.at(2, 3) *= 2;

    EXPECT_EQ(m_jag.at(0, 0), 2 * 1);
    EXPECT_EQ(m_jag.at(0, 1), 2 * 2);
    EXPECT_EQ(m_jag.at(0, 2), 2 * 3);
    EXPECT_EQ(m_jag.at(0, 3), 2 * 4);
    EXPECT_EQ(m_jag.at(1, 0), 2 * 5);
    EXPECT_EQ(m_jag.at(1, 1), 2 * 6);
    EXPECT_EQ(m_jag.at(2, 0), 2 * 7);
    EXPECT_EQ(m_jag.at(2, 1), 2 * 8);
    EXPECT_EQ(m_jag.at(2, 2), 2 * 9);
    EXPECT_EQ(m_jag.at(2, 3), 2 * 10);
}
