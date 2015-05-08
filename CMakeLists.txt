cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)

project(Wander VERSION 3.0.0 LANGUAGES C)

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
	Wander.txt
	WanderExportReadMe.txt
	WanderMisc.txt
	WanderWrld.txt)

add_executable(wander ${WANDER_SOURCES})

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
