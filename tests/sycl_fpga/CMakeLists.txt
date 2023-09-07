# VecMem project, part of the ACTS project (R&D line)
#
# (c) 2023 CERN for the benefit of the ACTS project
#
# Mozilla Public License Version 2.0

# Project include(s).
include( vecmem-compiler-options-sycl-fpga )

#message(${CMAKE_CXX_COMPILER})

# Test all of the SYCL library's features.
vecmem_add_test( sycl_fpga_emu
   #"test_sycl_fpga_memory_resources.cpp"
   "test_sycl_fpga_containers.cpp" #"test_sycl_fpga_jagged_containers.cpp"
   LINK_LIBRARIES vecmem::core vecmem::sycl )
set_target_properties( vecmem_test_sycl_fpga_emu PROPERTIES COMPILE_FLAGS "${EMULATOR_COMPILE_FLAGS}")
set_target_properties( vecmem_test_sycl_fpga_emu PROPERTIES LINK_FLAGS "${EMULATOR_LINK_FLAGS}")
