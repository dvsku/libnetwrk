INCLUDE(FetchContent)

SET(FETCHCONTENT_QUIET OFF)
SET(FETCHCONTENT_UPDATES_DISCONNECTED ON)

###############################################################################
## asio

SET(ASIO_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/asio/include" PARENT_SCOPE)

###############################################################################
## dv_utilities

IF(LIBNETWRK_EXAMPLES)
    SET(DVUTIL_TEST OFF)

    FetchContent_Declare(
        dv_utilities
        URL https://github.com/dvsku/dv_utilities/archive/refs/heads/master.zip
    )

    FetchContent_MakeAvailable(dv_utilities)

    SET(DVUTIL_INCLUDES "${dv_utilities_SOURCE_DIR}/include" PARENT_SCOPE)
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
