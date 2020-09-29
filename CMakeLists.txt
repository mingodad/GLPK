cmake_minimum_required(VERSION 3.0)
project(Glpk C)
enable_testing()
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "CMake build type." FORCE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DWITH_SPLAYTREE -DNDEBUG -flto")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DWITH_SPLAYTREE -DNDEBUG -flto")
endif ()

set(C_MATH_LIBRARY "m")

#
# Emscripten settings
#
if (EMSCRIPTEN)
	# Make Emscripten output html pages.
	# set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# To be able to detect Emscripten in code.
	add_definitions("-DEMSCRIPTEN=1")

	set(EMSCRIPTEN_FEATURES "")
	# We want only web.
	#set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s ENVIRONMENT=web,worker") # -s AUTO_JS_LIBRARIES=0")
	# We want atexit.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s EXIT_RUNTIME=1")
	# We want exceptions.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s DISABLE_EXCEPTION_CATCHING=0")
	# We want Webassembly.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1")
	# Filesystem support.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s FORCE_FILESYSTEM=1 -lidbfs.js")
	# Expose callMain since we use it in our client.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"callMain\"]'")
	if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		# Do not remove whitespace etc. in the JS parts (easier debugging).
		set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -g1")
	endif()
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		# Do not remove whitespace etc. in the JS parts (easier debugging).
		#set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} --source-map-base /myglpk-ext-dbg/ ")
	endif()
	# We don't want the page to hang while executing.
	# There will be some effort to make this work; e.g. creating files.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s PROXY_TO_WORKER=1 --proxy-to-worker")
	# Add preamble.
	set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/pre.js")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FEATURES}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FEATURES}")
	set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EMSCRIPTEN_FEATURES}")

	# Need an external protoc.
	if (NOT PROTOC)
		message(FATAL_ERROR "PROTOC needs to be set for Emscripten." )
	endif()

	# Extra things to make various detections in third-party/ work.
	set(SIZEOF_INT_P_run_result 4)
	set(GFLAGS_INTTYPES_FORMAT C99)
endif ()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory("examples")
add_subdirectory("src")
