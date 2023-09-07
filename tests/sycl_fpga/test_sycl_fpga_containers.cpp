/* VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// SYCL include(s).
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>

// Local include(s).
#include "vecmem/memory/sycl/device_memory_resource.hpp"
#include "vecmem/memory/sycl/host_memory_resource.hpp"
#include "vecmem/memory/sycl/shared_memory_resource.hpp"
#include "vecmem/utils/sycl/queue_wrapper.hpp"
#include "vecmem/utils/sycl/copy.hpp"
//#include "vecmem/utils/sycl/local_accessor.hpp"

// Simple asynchronous handler function
auto handle_async_error = [](::sycl::exception_list elist) {
    for (auto& e : elist) {
        try {
            std::rethrow_exception(e);
        } catch (::sycl::exception& e) {
            std::cout << "ASYNC EXCEPTION!!\n";
            std::cout << e.what() << "\n";
        }
    }
};

int main() {

    // Select either the FPGA emulator, FPGA simulator or FPGA device
#if FPGA_SIMULATOR
    auto selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
    auto selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
    auto selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif

    // Create a queue bound to the chosen device.
    // If the device is unavailable, a SYCL runtime exception is thrown.
    sycl::queue queue(selector, handle_async_error);

    auto device = queue.get_device();

    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;

    vecmem::sycl::queue_wrapper wrapper;
    // vecmem::sycl::queue_wrapper wrapper(&queue);

    // The shared memory resource.
    // vecmem::sycl::shared_memory_resource resource(&queue);
    // vecmem::sycl::shared_memory_resource resource;
}