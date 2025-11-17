cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)

project(Wander VERSION 3.0.0 LANGUAGES C)

find_program(NROFF nroff
	DOC "Path to nroff for building documentation")
if(NOT NROFF)
	message(WARNING "Could not find nroff; documentation will not be built")
endif(NOT NROFF)

set(WANDER_SOURCES
	wand1.c
	wand2.c
	wanddef.h
	wandglb.c
	wandsys.c)
set(WANDER_A3
	a3.misc
	a3.wrld)
set(WANDER_CASTLE
	castle.misc
	castle.wrld)
set(WANDER_LIBRARY
	library.misc
	library.wrld)
set(WANDER_TUT
	tut.misc
	tut.wrld)
set(WANDER_DOCS
	READ_ME
	READ_ME_TOO
	README.md
	WanderExportReadMe.txt)

add_executable(wander ${WANDER_SOURCES})

if(NROFF)
	# There was a Makefile rule for WanderExportReadMe.txt, but all it
	# did was copy & paste a dependency that doesn't exist. Go figure.
	add_custom_command(OUTPUT Wander.txt
		COMMAND ${NROFF} ${Wander_SOURCE_DIR}/wander.nr > ${Wander_BINARY_DIR}/Wander.txt
		MAIN_DEPENDENCY wander.nr
		DEPENDS mac
		WORKING_DIRECTORY ${Wander_SOURCE_DIR})
	add_custom_command(OUTPUT WanderMisc.txt
		COMMAND ${NROFF} ${Wander_SOURCE_DIR}/misc.nr > ${Wander_BINARY_DIR}/WanderMisc.txt
		MAIN_DEPENDENCY misc.nr
		DEPENDS mac
		WORKING_DIRECTORY ${Wander_SOURCE_DIR})
	add_custom_command(OUTPUT WanderWrld.txt
		COMMAND ${NROFF} ${Wander_SOURCE_DIR}/wrld.nr > ${Wander_BINARY_DIR}/WanderWrld.txt
		MAIN_DEPENDENCY wrld.nr
		DEPENDS mac
		WORKING_DIRECTORY ${Wander_SOURCE_DIR})
	add_custom_target(doc ALL
		DEPENDS Wander.txt WanderMisc.txt WanderWrld.txt)
	set(WANDER_DOCS
		${WANDER_DOCS}
		${Wander_BINARY_DIR}/Wander.txt
		${Wander_BINARY_DIR}/WanderMisc.txt
		${Wander_BINARY_DIR}/WanderWrld.txt)
else(NROFF)
	set(WANDER_DOCS
		${WANDER_DOCS}
		Wander.txt
		WanderMisc.txt
		WanderWrld.txt)
endif(NROFF)

install(TARGETS wander RUNTIME DESTINATION bin)
install(FILES ${WANDER_A3} DESTINATION share/wander/a3)
install(FILES ${WANDER_CASTLE} DESTINATION share/wander/castle)
install(FILES ${WANDER_LIBRARY} DESTINATION share/wander/library)
install(FILES ${WANDER_TUT} DESTINATION share/wander/tut)
install(FILES ${WANDER_DOCS} DESTINATION share/doc/wander)

set(CPACK_PACKAGE_VENDOR "Jared Miller")
set(CPACK_PACKAGE_VERSION_MAJOR ${Wander_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Wander_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Wander_VERSION_PATCH})
#set(CPACK_PACKAGE_DESCRIPTION_FILE )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fantasy story tool")
#set(CPACK_PACKAGE_ICON )
#set(CPACK_RESOURCE_FILE_LICENSE )
#set(CPACK_RESOURCE_FILE_README )
#set(CPACK_RESOURCE_FILE_WELCOME )
include(CPack)
