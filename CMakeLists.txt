cmake_minimum_required (VERSION 3.22)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

project(configEditor
  LANGUAGES CXX
  VERSION 0.0.0
)
add_executable(configEditor
	main.cc
)
#add_subdirectory(include/c1402-regex) # include if we decide to validate regex. would likely prove useful
add_subdirectory(include/logger)
add_subdirectory(include/rapidxml-1.13)

target_include_directories(configEditor
	PUBLIC include include/rapidxml-1.13 include/logger
	"${PROJECT_BINARY_DIR}"
)
target_link_libraries(configEditor
	PUBLIC
		logger
		rapidxml
)

set_target_properties(configEditor PROPERTIES CXX_STANDARD 20)
target_compile_options(configEditor PUBLIC -Werror -Wextra -Wpedantic -std=c++20)
