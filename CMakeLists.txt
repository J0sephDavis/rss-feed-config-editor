cmake_minimum_required (VERSION 3.22)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
# fetch FTXUI
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(ftxui
	GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
	GIT_TAG cdf2890 # version 5.0.0
	GIT_PROGRESS TRUE
)
FetchContent_GetProperties(ftxui)
if (NOT ftxui_POPULATED)
	FetchContent_Populate(ftxui)
	add_subdirectory(
		${ftxui_SOURCE_DIR}
		${ftxui_BINARY_DIR}
		EXCLUDE_FROM_ALL
	)
endif()
# end FTXUI

project(configEditor
  LANGUAGES CXX
  VERSION 0.0.1
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
	PRIVATE
		liblogger
		rapidxml
	PRIVATE
		ftxui::screen
		ftxui::dom
		ftxui::component
)

set_target_properties(configEditor PROPERTIES CXX_STANDARD 20)
target_compile_options(configEditor PUBLIC -Werror -Wextra -Wpedantic -std=c++20)
