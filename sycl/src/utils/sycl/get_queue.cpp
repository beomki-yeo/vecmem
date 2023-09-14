/* VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Local include(s).
#include "get_queue.hpp"

// System include(s).
#include <cassert>

namespace vecmem::sycl::details {

cl::sycl::queue& get_queue(vecmem::sycl::queue_wrapper& queue) {

    assert(queue.queue() != nullptr);
    return *(reinterpret_cast<cl::sycl::queue*>(queue.queue()));
}

const cl::sycl::queue& get_queue(const vecmem::sycl::queue_wrapper& queue) {

    assert(queue.queue() != nullptr);
    return *(reinterpret_cast<const cl::sycl::queue*>(queue.queue()));
}
}  // namespace vecmem::sycl::details
