project( consolePlayer )

add_executable(	consolePlayer
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${SOLUTION_DIR}/include/io/archieve.cpp
	${SOLUTION_DIR}/include/string/encoding.cpp
	${SOLUTION_DIR}/include/string/path.cpp
)

target_link_libraries( consolePlayer
	SAP
	libmp3lame.lib
	libvorbis.lib
	libvorbisfile.lib
	ogg.lib
	OpenAL32.lib
)