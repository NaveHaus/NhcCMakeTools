# PresetsGraph tests

nhc_add_test_executable(MacroContextTests
  SOURCES MacroContextTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(CoreGraphTests
  SOURCES CoreGraphTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(ConditionTests
  SOURCES ConditionTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(IncludeGraphTests
  SOURCES IncludeGraphTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(InheritanceGraphTests
  SOURCES InheritanceGraphTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(PresetModelTests
  SOURCES PresetModelTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)

nhc_add_test_executable(GraphManagerTests
  SOURCES GraphManagerTests.cpp
  USES NhcPresetGraph NhcTestLib
  FOLDER "Tests"
)
