include(CMakeFindDependencyMacro)

# Add dependencies
find_dependency(Threads REQUIRED)

# Include targets file
include("${CMAKE_CURRENT_LIST_DIR}/fmus-embedTargets.cmake")
