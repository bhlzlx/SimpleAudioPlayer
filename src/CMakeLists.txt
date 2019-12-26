project( libSAP )

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

add_library( SAP STATIC
	${SOURCE}
)