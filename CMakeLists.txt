cmake_minimum_required(VERSION 3.16)
project(neuropet LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

# Optionally enable Vulkan support for GPU acceleration
option(NEUROPET_ENABLE_VULKAN "Build with Vulkan GPU backend" OFF)
if(NEUROPET_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    add_compile_definitions(HARMONICS_HAS_VULKAN=1)
    # Compile compute shaders into an embedded header used by the GPU backend
    file(GLOB VULKAN_SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/shaders/*.comp)
    set(VULKAN_SHADERS_HPP
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/include/gpu/Shaders.hpp)
    add_custom_command(
        OUTPUT ${VULKAN_SHADERS_HPP}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/scripts/compile_shaders.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics
        DEPENDS ${VULKAN_SHADER_SOURCES}
        COMMENT "Compiling Vulkan shaders")
    add_custom_target(vulkan_shaders DEPENDS ${VULKAN_SHADERS_HPP})
else()
    find_package(Vulkan QUIET)
endif()

find_package(OpenSSL REQUIRED)

set(BLAKE3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/BLAKE3)
set(BLAKE3_SOURCES
    ${BLAKE3_DIR}/c/blake3.c
    ${BLAKE3_DIR}/c/blake3_dispatch.c
    ${BLAKE3_DIR}/c/blake3_portable.c
)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    list(APPEND BLAKE3_SOURCES
        ${BLAKE3_DIR}/c/blake3_sse2_x86-64_unix.S
        ${BLAKE3_DIR}/c/blake3_sse41_x86-64_unix.S
        ${BLAKE3_DIR}/c/blake3_avx2_x86-64_unix.S
        ${BLAKE3_DIR}/c/blake3_avx512_x86-64_unix.S)
    set_source_files_properties(${BLAKE3_DIR}/c/blake3_avx512_x86-64_unix.S
        PROPERTIES COMPILE_FLAGS "-mavx512f")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    list(APPEND BLAKE3_SOURCES ${BLAKE3_DIR}/c/blake3_neon.c)
endif()
add_library(blake3 STATIC ${BLAKE3_SOURCES})
target_include_directories(blake3 PUBLIC ${BLAKE3_DIR}/c)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    target_compile_definitions(blake3 PUBLIC BLAKE3_USE_NEON)
else()
    target_compile_definitions(blake3 PUBLIC BLAKE3_NO_SSE2 BLAKE3_NO_SSE41 BLAKE3_NO_AVX2 BLAKE3_NO_AVX512 BLAKE3_NO_NEON)
endif()
add_library(BLAKE3::blake3 ALIAS blake3)

add_library(int8_kernel INTERFACE)
target_include_directories(int8_kernel INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/include>
    $<INSTALL_INTERFACE:include>)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    target_compile_options(int8_kernel INTERFACE -mavx2)
    target_compile_definitions(int8_kernel INTERFACE NEUROPET_USE_AVX2)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    target_compile_definitions(int8_kernel INTERFACE NEUROPET_USE_NEON)
endif()
if(NEUROPET_ENABLE_VULKAN)
    target_link_libraries(int8_kernel INTERFACE Vulkan::Vulkan)
    add_dependencies(int8_kernel vulkan_shaders)
endif()

add_library(zk_stub STATIC src/zk_proof_system.cpp)
target_include_directories(zk_stub PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(zk_stub PUBLIC BLAKE3::blake3 OpenSSL::Crypto)

add_library(stark_prover SHARED src/stark_prover.cpp)
target_include_directories(stark_prover PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${BLAKE3_DIR}/c)
target_link_libraries(stark_prover PUBLIC OpenSSL::Crypto BLAKE3::blake3)

add_library(validator INTERFACE)
target_include_directories(validator INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(validator INTERFACE int8_kernel BLAKE3::blake3 zk_stub)

add_library(arena INTERFACE)
target_include_directories(arena INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/include>
    $<INSTALL_INTERFACE:include>)

add_library(battle_animation INTERFACE)
target_include_directories(battle_animation INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

add_library(metrics INTERFACE)
target_include_directories(metrics INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/harmonics/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/BLAKE3/c>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(metrics INTERFACE BLAKE3::blake3)

add_library(training INTERFACE)
target_include_directories(training INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(training INTERFACE int8_kernel metrics)

add_executable(metrics_demo examples/metrics_demo.cpp)
target_link_libraries(metrics_demo PRIVATE training)

add_executable(plugin_demo examples/plugin_demo.cpp)
target_include_directories(plugin_demo PRIVATE include third_party/harmonics/include)
target_link_libraries(plugin_demo PRIVATE dl)

add_executable(onchain_arena_demo examples/onchain_arena_demo.cpp)
target_link_libraries(onchain_arena_demo PRIVATE arena)

add_executable(proof_cli tools/proof_cli.cpp)
target_include_directories(proof_cli PRIVATE include third_party/harmonics/include)
target_link_libraries(proof_cli PRIVATE validator)

install(TARGETS blake3 DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# validator tests using the Blake3 proof system
add_executable(validator_test tests/validator_test.cpp)
target_include_directories(validator_test PRIVATE tests include third_party/harmonics/tests third_party/harmonics/include)
target_link_libraries(validator_test PRIVATE validator)
add_test(NAME validator_test COMMAND validator_test)

add_executable(inference_test tests/inference_test.cpp)
target_include_directories(inference_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(inference_test PRIVATE training)
add_test(NAME inference_test COMMAND inference_test)

add_executable(attachment_test tests/attachment_test.cpp)
target_include_directories(attachment_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(attachment_test PRIVATE arena)
add_test(NAME attachment_test COMMAND attachment_test)

add_executable(battle_animation_test tests/battle_animation_test.cpp)
target_include_directories(battle_animation_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(battle_animation_test PRIVATE battle_animation)
add_test(NAME battle_animation_test COMMAND battle_animation_test)

add_executable(battle_engine_test tests/battle_engine_test.cpp)
target_include_directories(battle_engine_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(battle_engine_test PRIVATE arena)
add_test(NAME battle_engine_test COMMAND battle_engine_test)

add_executable(quorum_test tests/quorum_test.cpp)
target_include_directories(quorum_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(quorum_test PRIVATE validator)
add_test(NAME quorum_test COMMAND quorum_test)

# Dataset caching tests
add_executable(cache_test tests/cache_test.cpp)
target_include_directories(cache_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(cache_test PRIVATE training)
add_test(NAME cache_test COMMAND cache_test)


add_executable(sensor_test tests/sensor_test.cpp)
target_include_directories(sensor_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(sensor_test PRIVATE training)
add_test(NAME sensor_test COMMAND sensor_test)

add_executable(model_size_test tests/model_size_test.cpp)
target_include_directories(model_size_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(model_size_test PRIVATE training)
add_test(NAME model_size_test COMMAND model_size_test)

add_executable(int8spec_test tests/int8spec_test.cpp)
target_include_directories(int8spec_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(int8spec_test PRIVATE int8_kernel)
add_test(NAME int8spec_test COMMAND int8spec_test)

add_executable(gpu_backend_test tests/gpu_backend_test.cpp)
target_include_directories(gpu_backend_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(gpu_backend_test PRIVATE int8_kernel)
add_test(NAME gpu_backend_test COMMAND gpu_backend_test)

add_executable(bridge_encoding_test tests/bridge_encoding_test.cpp)
target_include_directories(bridge_encoding_test PRIVATE tests include third_party/harmonics/tests)
add_test(NAME bridge_encoding_test COMMAND bridge_encoding_test)

add_executable(int8_determinism_test tests/int8_determinism_test.cpp)
target_include_directories(int8_determinism_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(int8_determinism_test PRIVATE int8_kernel)
add_test(NAME int8_determinism_test COMMAND int8_determinism_test)

add_executable(p2p_proof_test tests/p2p_proof_test.cpp)
target_include_directories(p2p_proof_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(p2p_proof_test PRIVATE validator)
add_test(NAME p2p_proof_test COMMAND p2p_proof_test)

add_executable(proof_system_test tests/proof_system_test.cpp)
target_include_directories(proof_system_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(proof_system_test PRIVATE validator)
add_test(NAME proof_system_test COMMAND proof_system_test)

add_executable(zk_proof_system_test tests/zk_proof_system_test.cpp)
target_include_directories(zk_proof_system_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(zk_proof_system_test PRIVATE validator)
add_test(NAME zk_proof_system_test COMMAND zk_proof_system_test)

add_executable(stark_prover_test tests/stark_prover_test.cpp)
target_include_directories(stark_prover_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(stark_prover_test PRIVATE stark_prover)
add_test(NAME stark_prover_test COMMAND stark_prover_test)

add_executable(matchmaker_test tests/matchmaker_test.cpp)
target_include_directories(matchmaker_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(matchmaker_test PRIVATE arena)
add_test(NAME matchmaker_test COMMAND matchmaker_test)

add_executable(pouw_chain_test tests/pouw_chain_test.cpp)
target_include_directories(pouw_chain_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(pouw_chain_test PRIVATE validator)
add_test(NAME pouw_chain_test COMMAND pouw_chain_test)

add_executable(pouw_flow_test tests/pouw_flow_test.cpp)
target_include_directories(pouw_flow_test PRIVATE tests include third_party/harmonics/tests)
target_link_libraries(pouw_flow_test PRIVATE validator)
add_test(NAME pouw_flow_test COMMAND pouw_flow_test)

add_executable(graph_diff_cli_test tests/graph_diff_cli_test.cpp)
target_include_directories(graph_diff_cli_test PRIVATE tests third_party/harmonics/include third_party/harmonics/tests)
target_link_libraries(graph_diff_cli_test PRIVATE BLAKE3::blake3)
add_test(NAME graph_diff_cli_test COMMAND graph_diff_cli_test)

find_program(FORGE_EXECUTABLE forge)
if(FORGE_EXECUTABLE)
    add_test(NAME contract_tests COMMAND ${FORGE_EXECUTABLE} test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(WARNING "forge not found; contract tests disabled")
endif()

add_executable(cache_cli tools/cache_cli.cpp)
target_include_directories(cache_cli PRIVATE include)
add_executable(bridge_cli tools/bridge_cli.cpp)
target_include_directories(bridge_cli PRIVATE include)

add_executable(graph_diff_cli tools/graph_diff_cli.cpp)
target_include_directories(graph_diff_cli PRIVATE
    third_party/harmonics/include
    third_party/BLAKE3/c)
target_link_libraries(graph_diff_cli PRIVATE BLAKE3::blake3)

add_executable(simulation_cli tools/simulation_cli.cpp)
target_include_directories(simulation_cli PRIVATE include)
target_link_libraries(simulation_cli PRIVATE arena training)

add_executable(aggregator_server tools/aggregator_server.cpp)
target_include_directories(aggregator_server PRIVATE
    include
    third_party/BLAKE3/c)
target_link_libraries(aggregator_server PRIVATE BLAKE3::blake3)

add_executable(int8_kernel_bench benchmarks/int8_kernel_bench.cpp)
target_include_directories(int8_kernel_bench PRIVATE include third_party/harmonics/include)
target_link_libraries(int8_kernel_bench PRIVATE int8_kernel)
