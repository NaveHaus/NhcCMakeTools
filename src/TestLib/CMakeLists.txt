find_package(Catch2 3.11 REQUIRED)

# Define NhcTestLib as an explicit STATIC library
add_library(NhcTestLib STATIC
    TestMain.cpp
)

# Link Catch2 with PUBLIC usage requirements so tests get headers/definitions
target_link_libraries(NhcTestLib PUBLIC Catch2::Catch2)

# Apply standard project configuration
nhc_configure_library(NhcTestLib)