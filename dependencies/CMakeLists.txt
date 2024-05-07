INCLUDE(FetchContent)

SET(FETCHCONTENT_QUIET OFF)
SET(FETCHCONTENT_UPDATES_DISCONNECTED ON)

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
