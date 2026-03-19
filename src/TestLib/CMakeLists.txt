find_package(Catch2 3.11 REQUIRED)

# Define NhcTestLib as an explicit STATIC library
nhc_add_library(NhcTestLib STATIC
 SOURCES TestMain.cpp
 USES Catch2::Catch2
 FOLDER "Libaries"
)