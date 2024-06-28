INCLUDE(FetchContent)

SET(FETCHCONTENT_QUIET OFF)
SET(FETCHCONTENT_UPDATES_DISCONNECTED ON)

###############################################################################
## asio

SET(ASIO_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/asio/include" PARENT_SCOPE)

###############################################################################
## libutil

IF(LIBNETWRK_EXAMPLES)
    FetchContent_Declare(
        libutil
        URL https://github.com/dvsku/libutil/archive/edd58aa8cbefcf986d63a5a3ecf0a7f37592afb3.zip
    )

    FetchContent_MakeAvailable(libutil)
ENDIF()

###############################################################################
## googletest

IF(LIBNETWRK_TEST)
	SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
	)

	FetchContent_MakeAvailable(googletest)

	SET(GTEST_INCLUDES "${googletest_SOURCE_DIR}/googletest/include" PARENT_SCOPE)
ENDIF()
