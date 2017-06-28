cmake_minimum_required(VERSION 3.5.1)
project(MiD_idxcrypt)

# Manually add the sources/headers using the set command as follows:
set(EXE_SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/ANSI_UTF16_Converter.cpp
	${CMAKE_SOURCE_DIR}/File_Struct.cpp
	${CMAKE_SOURCE_DIR}/idxcrypt.cpp
	${CMAKE_SOURCE_DIR}/Linux_File.cpp
	${CMAKE_SOURCE_DIR}/mem_impl.cpp
	${CMAKE_SOURCE_DIR}/MyLinuxSysFunctions.cpp
	${CMAKE_SOURCE_DIR}/Win32_File.cpp
)
set(EXE_HEADER_FILES 
	${CMAKE_SOURCE_DIR}/ANSI_UTF16_Converter.h
	${CMAKE_SOURCE_DIR}/File_Struct.h
	${CMAKE_SOURCE_DIR}/Linux_File.h
	${CMAKE_SOURCE_DIR}/mem_impl.h
	${CMAKE_SOURCE_DIR}/MyLinuxSysFunctions.h
	${CMAKE_SOURCE_DIR}/Win32_File.h
)

# Create the exe from the source files and headers (makes them visible in the project tree)
add_executable(MiD_idxcrypt ${EXE_SOURCE_FILES} ${EXE_HEADER_FILES})

# Bring the headers into the project (in additional includes) (directories not files)
target_include_directories(MiD_idxcrypt PUBLIC 
	${CMAKE_SOURCE_DIR}/MiDAesLib/include
	${CMAKE_SOURCE_DIR}/MiDHashLib/include
	${CMAKE_SOURCE_DIR}/MiDHmacLib/include
	${CMAKE_SOURCE_DIR}/MiD_PBKDF2/include
	${CMAKE_SOURCE_DIR}/openssl/include
)

# Set -m32 for Linker and Compiler flags when building in 32-bit mode under UNIX 
# Check whether we're building in 32 or 64 mode
# Since we only build CXX, we set C flag to -m32 when we want to compile for 32-bit under 64-bit 
# and do a comparison (we can compare another custom made flag)

# UNIX AND NOT APPLE => for Linux, BSD, Solaris, Minix

if(CMAKE_SIZEOF_VOID_P EQUAL 8)

	if((UNIX AND NOT APPLE) AND CMAKE_C_FLAGS STREQUAL "-m32")
		set_target_properties(MiD_idxcrypt PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
		MESSAGE("Building in 32-bit mode under 64-bit UNIX : -m32 set in LINK_FLAGS and COMPILE_FLAGS")
		SET(BUILD_ENV "32")
	
	elseif((UNIX AND NOT APPLE) AND NOT(CMAKE_C_FLAGS STREQUAL "-m32"))
		MESSAGE("Building in 64-bit mode under 64-bit UNIX")
		SET(BUILD_ENV "64")
	
	endif((UNIX AND NOT APPLE) AND CMAKE_C_FLAGS STREQUAL "-m32")

	if(MSVC)
		MESSAGE("Building in 64-bit mode under MSVC")
		SET(BUILD_ENV "64")
	endif(MSVC)
	
else(CMAKE_SIZEOF_VOID_P EQUAL 8)

	if(UNIX AND NOT APPLE)
		MESSAGE("Building in 32-bit mode under 32-bit UNIX")
		SET(BUILD_ENV "32")
		
	elseif(MSVC)
		MESSAGE("Building in 32-bit mode under MSVC")
		SET(BUILD_ENV "32")
	
	endif(UNIX AND NOT APPLE)
	
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)


# Linking against external non standard libraries (openssl + Static in this case) (makes them visible in additional dependencies) 
# Order is very important in UNIX - linker discards libraries that are linked before actually being used
# Order : compile => need symbol reference => look in libraries 

if (BUILD_ENV STREQUAL "32")

	if(WIN32)

		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/openssl/lib/32/Debug/libeay32.lib)
		MESSAGE("Linking against 32-bit Openssl-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/openssl/lib/32/Release/libeay32.lib)
		MESSAGE("Linking against 32-bit Openssl-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiDHashLib/lib/32/Debug/MiDHashLib_Static.lib)
		MESSAGE("Linking against 32-bit MidHashLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiDHashLib/lib/32/Release/MiDHashLib_Static.lib)
		MESSAGE("Linking against 32-bit MidHashLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/32/Debug/MiDHmacLib_Static.lib)
		MESSAGE("Linking against 32-bit MiDHmacLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/32/Release/MiDHmacLib_Static.lib)
		MESSAGE("Linking against 32-bit MiDHmacLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/32/Debug/MiD_PBKDF2_Static.lib)
		MESSAGE("Linking against 32-bit MiD_PBKDF2_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/32/Release/MiD_PBKDF2_Static.lib)
		MESSAGE("Linking against 32-bit MiD_PBKDF2_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiDAesLib/lib/32/Debug/MiDAesLib_Static.lib)
		MESSAGE("Linking against 32-bit MiDAesLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiDAesLib/lib/32/Release/MiDAesLib_Static.lib)
		MESSAGE("Linking against 32-bit MiDAesLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC Shlwapi.lib)
		MESSAGE("Linking against Shlwapi")
		
	elseif(UNIX AND NOT APPLE)
		
		if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		
			find_library(static_debug_lib1_32 NAMES libMiDHashLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHashLib/lib/32/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib1_32})
			MESSAGE("Linking against 32-bit MiDHashLib_Static-Debug (unix)")
			
			find_library(static_debug_lib2_32 NAMES libMiDHmacLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/32/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib2_32})
			MESSAGE("Linking against 32-bit MiDHmacLib_Static-Debug (unix)")
		
			find_library(static_debug_lib3_32 NAMES libMiD_PBKDF2_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/32/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib3_32})
			MESSAGE("Linking against 32-bit MiD_PBKDF2_Static-Debug (unix)")
			
			find_library(static_debug_lib4_32 NAMES libMiDAesLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDAesLib/lib/32/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib4_32})
			MESSAGE("Linking against 32-bit MiDAesLib_Static-Debug (unix)")
			
			find_library(static_debug_libcrypto32 NAMES libcrypto.a PATHS "${CMAKE_SOURCE_DIR}/openssl/lib/32/Debug" NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_libcrypto32})
			MESSAGE("Linking against 32-bit Openssl-Debug (unix)")
			
			target_link_libraries( MiD_idxcrypt PUBLIC ${CMAKE_DL_LIBS})
			MESSAGE("Linking against libdl (unix)")
			
		elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		
			find_library(static_release_lib1_32 NAMES libMiDHashLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHashLib/lib/32/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib1_32})
			MESSAGE("Linking against 32-bit MiDHashLib_Static-release (unix)")
			
			find_library(static_release_lib2_32 NAMES libMiDHmacLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/32/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib2_32})
			MESSAGE("Linking against 32-bit MiDHmacLib_Static-release (unix)")
		
			find_library(static_release_lib3_32 NAMES libMiD_PBKDF2_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/32/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib3_32})
			MESSAGE("Linking against 32-bit MiD_PBKDF2_Static-release (unix)")
			
			find_library(static_release_lib4_32 NAMES libMiDAesLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDAesLib/lib/32/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib4_32})
			MESSAGE("Linking against 32-bit MiDAesLib_Static-release (unix)")
			
			find_library(static_release_libcrypto32 NAMES libcrypto.a PATHS "/usr/lib/i386-linux-gnu" NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_libcrypto32})		
			MESSAGE("Linking against 32-bit preinstalled Openssl-Release (unix)")
			
			target_link_libraries( MiD_idxcrypt ${CMAKE_DL_LIBS})
			MESSAGE("Linking against libdl (unix)")
			
		endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
	
	endif(WIN32)
		
elseif(BUILD_ENV STREQUAL "64")

	if(WIN32)

		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/openssl/lib/64/Debug/libeay32.lib)
		MESSAGE("Linking against 64-bit Openssl-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/openssl/lib/64/Release/libeay32.lib)
		MESSAGE("Linking against 64-bit Openssl-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiDHashLib/lib/64/Debug/MiDHashLib_Static.lib)
		MESSAGE("Linking against 64-bit MidHashLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiDHashLib/lib/64/Release/MiDHashLib_Static.lib)
		MESSAGE("Linking against 64-bit MidHashLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/64/Debug/MiDHmacLib_Static.lib)
		MESSAGE("Linking against 64-bit MiDHmacLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/64/Release/MiDHmacLib_Static.lib)
		MESSAGE("Linking against 64-bit MiDHmacLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/64/Debug/MiD_PBKDF2_Static.lib)
		MESSAGE("Linking against 64-bit MiD_PBKDF2_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/64/Release/MiD_PBKDF2_Static.lib)
		MESSAGE("Linking against 64-bit MiD_PBKDF2_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC debug ${CMAKE_SOURCE_DIR}/MiD_AesLib/lib/64/Debug/MiDAesLib_Static.lib)
		MESSAGE("Linking against 64-bit MiDAesLib_Static-Debug (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC optimized ${CMAKE_SOURCE_DIR}/MiD_AesLib/lib/64/Release/MiDAesLib_Static.lib)
		MESSAGE("Linking against 64-bit MiDAesLib_Static-Release (windows)")
		
		target_link_libraries( MiD_idxcrypt PUBLIC Shlwapi.lib)
		MESSAGE("Linking against Shlwapi")
		
	elseif(UNIX AND NOT APPLE)
	
		if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		
			find_library(static_debug_lib1_64 NAMES libMiDHashLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHashLib/lib/64/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib1_64})
			MESSAGE("Linking against 64-bit MiDHashLib_Static-Debug (unix)")
			
			find_library(static_debug_lib2_64 NAMES libMiDHmacLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/64/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib2_64})
			MESSAGE("Linking against 64-bit MiDHmacLib_Static-Debug (unix)")
		
			find_library(static_debug_lib3_64 NAMES libMiD_PBKDF2_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/64/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib3_64})
			MESSAGE("Linking against 64-bit MiD_PBKDF2_Static-Debug (unix)")
			
			find_library(static_debug_lib4_64 NAMES libMiDAesLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDAesLib/lib/64/Debug")
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_lib4_64})
			MESSAGE("Linking against 64-bit MiDAesLib_Static-Debug (unix)")
			
			find_library(static_debug_libcrypto64 NAMES libcrypto.a PATHS "${CMAKE_SOURCE_DIR}/openssl/lib/64/Debug" NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
			target_link_libraries( MiD_idxcrypt PUBLIC debug ${static_debug_libcrypto64})
			MESSAGE("Linking against 64-bit Openssl-Debug (unix)")
			
			target_link_libraries( MiD_idxcrypt ${CMAKE_DL_LIBS})
			MESSAGE("Linking against libdl (unix)")
			
		elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		
			find_library(static_release_lib1_64 NAMES libMiDHashLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHashLib/lib/64/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib1_64})
			MESSAGE("Linking against 64-bit MiDHashLib_Static-release (unix)")
			
			find_library(static_release_lib2_64 NAMES libMiDHmacLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDHmacLib/lib/64/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib2_64})
			MESSAGE("Linking against 64-bit MiDHmacLib_Static-release (unix)")
		
			find_library(static_release_lib3_64 NAMES libMiD_PBKDF2_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiD_PBKDF2/lib/64/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib3_64})
			MESSAGE("Linking against 64-bit MiD_PBKDF2_Static-release (unix)")
			
			find_library(static_release_lib4_64 NAMES libMiDAesLib_Static.a PATHS "${CMAKE_SOURCE_DIR}/MiDAesLib/lib/64/release")
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_lib4_64})
			MESSAGE("Linking against 64-bit MiDAesLib_Static-release (unix)")
			
			find_library(static_release_libcrypto64 NAMES libcrypto.a PATHS "/usr/lib/i386-linux-gnu" NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
			target_link_libraries( MiD_idxcrypt PUBLIC optimized ${static_release_libcrypto64})		
			MESSAGE("Linking against 64-bit preinstalled Openssl-Release (unix)")
			
			target_link_libraries( MiD_idxcrypt ${CMAKE_DL_LIBS})
			MESSAGE("Linking against libdl (unix)")
			
		endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

	endif(WIN32)
	
endif (BUILD_ENV STREQUAL "32")


# Set the relative location for executable installation
# Make sure you define CMAKE_INSTALL_PREFIX to determine the abstract location 
# Otherwise, the abstract location will be : /usr/local (unix), C:/Program Files (Windows)
# Use "sudo make install" to apply

# Copies the executable from CMAKE_BINARY_DIR=build or build/32 depending on... to CMAKE_INSTALL_PREFIX/lib

# Set extensions

if(WIN32)
	set(Exte ".exe")
	MESSAGE("Executable extension set to .exe")
else(WIN32)
	set(Exte "")
	MESSAGE("Executable extension unmodified")
endif(WIN32)

if(MSVC)
	set(DirDebug "/Debug")
	set(DirRel "/Release")
else(MSVC)
	set(DirDebug "")
	set(DirRel "")
endif(MSVC)

if(MSVC OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("Configuring Debug version")
	install(PROGRAMS ${CMAKE_BINARY_DIR}${DirDebug}/MiD_idxcrypt${Exte}
		DESTINATION bin/${BUILD_ENV}/Debug
		RENAME MiD_idxcrypt${Exte})
		
endif(MSVC OR CMAKE_BUILD_TYPE STREQUAL "Debug")

if(MSVC OR CMAKE_BUILD_TYPE STREQUAL "Release")
	MESSAGE("Configuring Release version")
	install(PROGRAMS ${CMAKE_BINARY_DIR}${DirRel}/MiD_idxcrypt${Exte}
		DESTINATION bin/${BUILD_ENV}/Release
		RENAME MiD_idxcrypt${Exte})
	
endif(MSVC OR CMAKE_BUILD_TYPE STREQUAL "Release")