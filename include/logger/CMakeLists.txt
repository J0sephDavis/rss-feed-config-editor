cmake_minimum_required (VERSION 3.22)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

project(logger
  LANGUAGES CXX
  VERSION 1.0.0
  DESCRIPTION "A simple single instance logger"
)
add_library(liblogger
	logger.cc
	logger.hh
)
#configure file
set(LOG_FOLDER "${PROJECT_BINARY_DIR}/../data/logs/")
configure_file("logger_config.h.in" "logger_config.h" @ONLY)
#
target_include_directories(liblogger
	PRIVATE
		${CMAKE_BINARY_DIR} #for logger_config.h
	PUBLIC
		${CMAKE_SOURCE_DIR}
)
set_target_properties(liblogger PROPERTIES CXX_STANDARD 20)
target_compile_options(liblogger PUBLIC -Werror -Wextra -Wpedantic)
