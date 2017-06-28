Project tree : 

Project_Root/
			
      |---CMakeLists.txt
	  
	  |---build
			
      |	|---win32/
			
      |	|	|---32/
	  
	  |	|	|---64/
	  
	  |	|---linux/
			
      |	|	|---32/
	  
	  |	|	|	|---Debug/
	  
	  |	|	|	|---Release/
	  
	  |	|	|---64/
	  
	  |	|	|	|---Debug/
	  
	  |	|	|	|---Release/
			
      |---install
	  
	  |	|---win32/
	  
	  |	|---linux/

Here we explain how to build + install under Windows using MSVC and under Linux.

```1 - MSVC : (i.e. on a Windows VM)```
	
All commands shall be executed from a normal command line.
	
**a/Create MSVC projects** : multiconfiguration (Debug, Release, RelWithDebInfo and MinSizeRel are created at the same time)
		
*32-bit* : 
		
		cd build\win32\32 (of lib_root)
		cmake -G "Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX="..\..\..\install\win32" ..\..\..
		
*64-bit* : 
		
		cd build\win32\64 (of lib_root)
		cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX="..\..\..\install\win32" ..\..\..

**b/Build** : (equivalent to make)

*32-bit* : (in build\win32\32)
		
		cmake --build . --config Release
		cmake --build . --config Debug
		
*64-bit* : (in build\win32\64)
		
		cmake --build . --config Release
		cmake --build . --config Debug
		
**c/Install** : (equivalent to make install)

*32-bit* : (in build\win32\32)
		
		cmake --build . --target install
		
*64-bit* : (in build\win32\64)
		
		cmake --build . --target install

```2 - GNU/LINUX : (i.e. on Linux VM)```

**a/Create Makefiles** : 1-configuration (Debug, Release, RelWithDebInfo and MinSizeRel are created one at a time)

*N.B :* 

Make sure you have g++-multilib (gcc-multilib) installed to be able to compile 32-bit under 64-bit

*32-bit :*
		
**Debug :**
		
		cd build/linux/32/Debug
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_FLAGS:STRING="-m32" -DCMAKE_CXX_FLAGS:STRING="-g -O0 -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..
		
**Release :**
		
		cd build/linux/32/Release
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_C_FLAGS:STRING="-m32" -DCMAKE_CXX_FLAGS:STRING="-O3 -DNDEBUG -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..
		
**Release With Debug Info:** 
		
		cd build/linux/32/ReleaseWithDebugInfo
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCMAKE_C_FLAGS:STRING="-m32" -DCMAKE_CXX_FLAGS:STRING="-O2 -g -DNDEBUG -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..

		
*64-bit :*
		
**Debug :**
		
		cd build/linux/64/Debug
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_FLAGS:STRING="-g -O0 -Wextra -Wall" -DCMAKE_CXX_FLAGS:STRING="-g -O0 -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..
		
**Release :** 
		
		cd build/linux/64/Release
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_C_FLAGS:STRING="-O3 -DNDEBUG -Wextra -Wall" -DCMAKE_CXX_FLAGS:STRING="-O3 -DNDEBUG -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..
		
**Release With Debug Info:** 
		
		cd build/linux/64/ReleaseWithDebugInfo
		cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="../../../../install/linux" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCMAKE_C_FLAGS:STRING="-O2 -g -DNDEBUG -Wextra -Wall" -DCMAKE_CXX_FLAGS:STRING="-O2 -g -DNDEBUG -std=c++14 -Wextra -Wall" -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ../../../..


**b/Build** : make (or cmake)

*32-bit* : (in build/linux/32/Debug,Release)
		
		make
		
*64-bit* : (in build/linux/64/Debug,Release)
		
		make
		
**c/Install** : 

*32 and 64-bit* :

		make install
