# Tests should link NhcTestLib instead of directly depending on Catch2.
# NhcTestLib provides Catch2 integration and the main() entry point.

# Add test subdirectories
add_subdirectory(TestLib)