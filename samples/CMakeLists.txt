project( Samples )

##if(WIN32)
##    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
##    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
##    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
##    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
##endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(
	${SOLUTION_DIR}/include
	${SOLUTION_DIR}/src
)

set( EXECUTABLE_TYPE )

if( ANDROID )
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -E")
    add_subdirectory( Android )
    SET_PROPERTY(TARGET AndroidSample PROPERTY FOLDER "samples")
else()
    if( WIN32 )
	
		add_definitions(-DUNICODE -D_UNICODE)
		
        add_subdirectory( consolePlayer )

        SET_PROPERTY(TARGET consolePlayer PROPERTY FOLDER "samples")
    endif()
endif()