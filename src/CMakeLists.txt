project( libSAP )

add_library( SAP STATIC )

set( SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/audioStream.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audioStreamMP3.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audioStreamOGG.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audioStreamPCM.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/audioStream.h
	#
	# ${CMAKE_CURRENT_SOURCE_DIR}/FFTUtility.cpp
	# ${CMAKE_CURRENT_SOURCE_DIR}/FFTUtility.h
	#
	${CMAKE_CURRENT_SOURCE_DIR}/oalBuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/oalBuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/oalDevice.h
	${CMAKE_CURRENT_SOURCE_DIR}/oalDevice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/oalSource.h
	${CMAKE_CURRENT_SOURCE_DIR}/oalSource.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/oalStreamSource.h
	${CMAKE_CURRENT_SOURCE_DIR}/oalStreamSource.cpp
	#
	${CMAKE_CURRENT_SOURCE_DIR}/oalDef.h
)

if( SUPPORT_OGG )
    add_definitions(-DADD_OGG_SUPPORT)
endif()
if( SUPPORT_MP3 )
    add_definitions(-DADD_MP3_SUPPORT)
endif()
if( SUPPORT_WAV )
    add_definitions(-DADD_WAV_SUPPORT)
endif()

target_sources( SAP
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/audioStream.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/audioStream.h
	PRIVATE # ${CMAKE_CURRENT_SOURCE_DIR}/FFTUtility.cpp
	PRIVATE # ${CMAKE_CURRENT_SOURCE_DIR}/FFTUtility.h
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalBuffer.h
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalBuffer.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalDevice.h
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalDevice.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalSource.h
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalSource.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalStreamSource.h
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalStreamSource.cpp
	PRIVATE #
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/oalDef.h
)

if( SUPPORT_OGG )
	target_sources( SAP
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/audioStreamOGG.cpp
	)
endif()
if( SUPPORT_MP3 )
	target_sources( SAP
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/audioStreamMP3.cpp
	)
endif()
if( SUPPORT_WAV )
	target_sources( SAP
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/audioStreamPCM.cpp
	)
endif()
target_sources( SAP
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/io/archieve.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/string/path.cpp
)

target_include_directories( SAP 
	PUBLIC ${OAL_INC_DIR}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${XMAS_SOLUTION_DIR}/3rd/LockFreeQueue/arrayImp
	PRIVATE ${OGG_INC_DIR}
	PRIVATE ${VORBIS_INC_DIR}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)