cmake_minimum_required (VERSION 3.22)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

project(logger
  LANGUAGES CXX
  VERSION 1.0.1
  DESCRIPTION "A simple single instance logger"
)
#configure file
set(LOG_FOLDER "${CMAKE_BINARY_DIR}/../data/logs/")
set(LOG_PREFIX "rss-config-") #todo: allow this to be optionally set by the top-level project
set(LOG_SUFFIX ".log")
configure_file("logger_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/logger_config.h" @ONLY)

add_library(liblogger
	logger.cc
	logger.hh
	${CMAKE_CURRENT_BINARY_DIR}/logger_config.h
)
target_include_directories(liblogger
	PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(liblogger PROPERTIES CXX_STANDARD 20)
target_compile_options(liblogger PUBLIC -Werror -Wextra -Wpedantic)
