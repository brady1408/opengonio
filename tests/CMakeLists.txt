include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(Catch2)

add_executable(opengonio_tests
    test_SampleFifo.cpp
    test_CorrelationMeter.cpp
    test_EnergyTracker.cpp)

target_include_directories(opengonio_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(opengonio_tests PRIVATE Catch2::Catch2WithMain)
target_compile_features(opengonio_tests PRIVATE cxx_std_20)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(opengonio_tests)
