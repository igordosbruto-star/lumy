# CTest/CDash configuration for Lumy
# Adjust values as needed for your environment.

set(CTEST_SITE "local")
set(CTEST_BUILD_NAME "msvc-vcpkg")
set(CTEST_SOURCE_DIRECTORY ".")
set(CTEST_BINARY_DIRECTORY "build/msvc")
set(CTEST_CMAKE_GENERATOR "Visual Studio 17 2022")
set(CTEST_CONFIGURATION_TYPE "Debug")

# CDash submission
set(CTEST_DROP_METHOD "https")
set(CTEST_DROP_SITE "cdash.example.com")
set(CTEST_DROP_LOCATION "/submit.php?project=Lumy")
set(CTEST_DROP_SITE_CDASH TRUE)
