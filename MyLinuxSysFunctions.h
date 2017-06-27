/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifndef MYLINUXSYSFUNCS_H
#define MYLINUXSYSFUNCS_H

#ifdef __linux__

#include <fcntl.h>      // open (pathname, mode), O_RDONLY
#include <unistd.h>     // chdir(pathname), fchdir(file_descriptor) and close(file_descriptor)
#include <dirent.h>     // DIR*, opendir(dirname), dirent*, readdir(DIR*), closedir(DIR*)
#include <sys/stat.h>   // stat(pathname, stat_struct)), stat struct, fstat(file_descriptor, stat_struct)

#include <utility>      // std::pair (bits/stl_pair.h)
#include <string>       // string
#include <cstring>      // strcmp
#include <iostream>
#include <errno.h>

#include <cstdint>		// __int64
typedef int64_t __int64;

static auto pathmax = pathconf("/", _PC_PATH_MAX) + 1; // Need to check if < 0; If it's the case, define a guessed value (256 = POSIX value)

/*
* A lot of system inherited functions that take as an argument a path or a
* filename (i.e. chdir, getcwd, mkdir, open) are bound to a certain limit when
* it comes to the length of the path (or the filename).
* Example :
*      - Limit for a path : PATH_MAX;
*      - Limit for a filename : NAME_MAX;
*
* These limits are either :
*
*      1/Compile-time limits
*              limits that are fixed for all implementations, defined in headers, following some standard
*              limits that are fixed for this particular implementation but may vary for others
*              (compile-time machine, runtime machine)
*		Defined by ISO C, POSIX and/or XSI in <limits.h>
*      2/Runtime limits not associated with a file or directory (sysconf, confstr) => file-system-dependent
*		3/Runtime limits associated with a file or directory (pathconf, fpathconf) => pathname-dependant and file-system-dependent
*
* Example of a problematic situation :
* User enters a path that exceeds his system's PATH_MAX. Applying chdir to
* that path will result in an error (PATH_MAX limit exceeded)
*
* We know that the filesystems that are compatible with Linux kernel (/proc/filesystems) almost all have an undefined length
* for the pathname and a length of filename that doesn't exceed 255 bytes. If the constant PATH_MAX is defined in <limits.h>,
* then we’re all set (in case the compile-time machine = runtime machine). If it’s not the case, then we need to call pathconf.
* The value returned by pathconf is the maximum size of a relative pathname when the first argument is the working directory, so we specify
* the root as the first argument and add 1 to the result. If pathconf indicates that
* PATH_MAX is indeterminate (<0), we have to just guess a value.
*
* Taking all these parameters into consideration, we chose the following set up :
*
*      *Implement our own system-based-look-alike functions when needed and possible
*       (not always possible with the absence of the flag O_SEARCH on GNU/Linux)
*      *These implemented functions will either operate independentally to their system counterparts or
*       call system-based functions with paths + filenames which will not exceed :
*
*          - PATH_MAX = defined in <limits.h> (if compile-time machine = runtime) or with a call to pathconf/fpathconf
*          - NAME_MAX = defined in <limits.h> (if compile-time machine = runtime) or with a call to pathconf/fpathconf
*                          (always < PATH_MAX)
*
* Using this approach, we're sure that in each MAX_PATH-byte chunk of a path, there will be
* at least 1 filename, and that the call to system inherited functions (i.e. chdir) wont
* fail because of the length of the path
*/

/* int GetCurrentWorkingDirectory(std::string &)
* An alternative implementation of getcwd() with no path length limitation
*
* Credits go to Insane Coding's article : http://insanecoding.blogspot.fr/2007/11/pathmax-simply-isnt.html

General idea (thread-safe)

* 1-Start from "." [initial director]
* 2-Gather info about it
* 3-Gather info about the root "/" (root directory)
* 4-Opendir/opendirat  ".." [parent directory of the initial directory]
* 5-Gather all of its entries
* 6-Find the directory we were in before opendir by comparing each entry of the
*   parent directory to the initial directory
* 7-Once found, get the name
* 8-Check if we've reached root directory
* 9-If yes, form the path
* 10-If no, set [initial directory] to the current directory, go to 4 (recursively)

* This implementation is thread-safe since it doesn't use chdir
* As it is the case for the system getcwd function, this function shall fail if :
* A component of the path doesn't have read OR search permissions
* However, the path limitation is no longer appliable.
* This implementation makes use of opendir(). Also, it uses fstat using the file descriptor
* returned by dirfd.
*
* Concerning access errors, opendir requires the search permission to be on for all
* components of the path prefix, and read permission only for the dirname itself.
* Also, fstat requires only search permission on all path prefix components.
* Therefore, we're good, knowing that getcwd will only work if all path's components
* have read + search permission on.
*
*/
int GetCurrentWorkingDirectory(std::string & cwdPath);

/*
* If dirname is relative, it is opened relatively to
* the directory "pointed to" by fd_dir using open and O_RDONLY flag (OK with opendir permissions)
* Permissions are OK.
*  - opendir needs search permission for all components of prefix path
*    and read for only the directory name
*  - openat requires the directory itself to have mode's permission (here read => OK)
*    and all components of prefix path to be searchable (OK)
*  - fails if fd_dir references a non-directory file and dirname is not absolute (same as openat)
*/
DIR * opendirat(int fd_dir, const char * dirname);

/*
* Separates parent directory from base name
* Doesn't translate ".." or "./" (Separation as it is)
* Removes extra slashes and "./"s from the end of the path
*
* dir : contains last /
* base : doesn't contain last /
*/
void dirname_base_separator(const std::string & path, std::string & dir, std::string & base);

/*
* int getAbsolutePath(std::string & , std::string & )
* Transforms a relative path in to an absolute one
* Thread-safe
* Shall fail if : Read or search permission was denied for a component of relativePath
* (just like realpath())
* No path limitation, although path length shouldn't exceed the max positive value of an __int64,
* in bytes, which is unlikely
*
* In case we want to use chdir in this implementation :
* To get back to current directory after chdir
*  - we can use open with O_SEARCH flag if available to get a file descr
*    (O_RDONLY can give errors if cwd is not readable) - O_SEARCH not available
*    under Linux
*  - opendir may give errors too if cwd is not readable
*  - GetCurrentWorkingDirectory needs read + search permission on every component as well
* Solution : we don't use chdir at all
*
* Basic idea : (No need for chdir)
*
* -Separate dirname from base
* -Recursive lookup on dirname
* -if base = "..", strip last path component
* -form path
* -add base
*
* Not compatible with symlinks
*
* absolutePath doesn't contain last /
*/
int getAbsolutePath(const std::string & relativePath, std::string & absolutePath);
int getAbsolutePath(const std::string && relativePath, std::string & absolutePath);


/*
* TO REIMPLEMENT ONCE O_SEARCH FLAG IS AVAILABLE UNDER LINUX
*
* int my_chdir(std::string &)
* An alternative implementation of chdir() with no path length limitation
* However, path length shouldn't exceed the max positive value of an __int64,
* in bytes, which is unlikely
* Uses chdir on chunks of MAX_PATH bytes of the pathname
* N.B : if 1 chdir fails, the current working directory may be altered
* (We use open with O_RDONLY flag to get back to cwd,
* until we have O_SEARCH flag on Linux, this is the best we can get)
*
* Not suitable everywhere (cwd might be altered if open cannot open cwd in RDONLY)
*
* All path components should have search permission on
*/
int my_chdir(const std::string & path);
int my_chdir(const std::string && path);

/*
* TO REIMPLEMENT ONCE O_SEARCH FLAG IS AVAILABLE UNDER LINUX
*
* DIR * my_opendir(std::string & );
* An alternative implementation of opendir() with no path length limitation
* However, path length shouldn't exceed the max positive value of an __int64,
* in bytes, which is unlikely
*
* N.B : In linux, no path limitation for opendir, so this is useless? (see man 2/3 opendir)
*
* Uses opendir on chunks of 256 bytes of path, therefore :
* WARNING : This implementation requires that :
*          ** All path components should have search AND read permission on
* Use this implementation when you need to use opendir on paths that exceed
* PATH_MAX AND when 'All path components have search AND read permission'
* (until we have O_SEARCH flag on Linux, this is the best we can get)
* i.e. realpath alternative implementation (getAbsolutePath)
*
* Not suitable everywhere (fails if one path component doesn't have read permission on)
*/
DIR * my_opendir(const std::string & path);
DIR * my_opendir(const std::string && path);

/*
* TO REIMPLEMENT ONCE O_SEARCH FLAG IS AVAILABLE UNDER LINUX
*
* int my_open(std::string & , int )
* An alternative implementation of open() with no path length limitation
* Uses open()
* This implementation cuts the path into chunks of 256 bytes at max, and opens
* them one after the other using the flag O_RDONLY (since O_SEARCH is not available
* in GNU/Linux, only UNIX).
* Therefore, this implementation requires that all path components exist and
* have search AND read permission, and last file should have permission of the oflag
* (until we have O_SEARCH flag on Linux, this is the best we can get)
* (Useful for realpath alternative implementation)
*
* Not suitable everywhere (fails if some path component doesn't have read permission on)
*/
int my_open(const std::string & path, int oflag);
int my_open(const std::string && path, int oflag);

/*
* TO REIMPLEMENT ONCE O_SEARCH FLAG IS AVAILABLE UNDER LINUX
*
* int my_mkdir(std::string & , __mode_t )
* An alternative implementation of mkdir() with no path length limitation
* Uses mkdir, open and fchdir
* (We use open with O_RDONLY flag to get back to cwd,
* until we have O_SEARCH flag on Linux, this is the best we can get)
*
* If 1 chdir fails, the cwd may be altered
*
* Not suitable everywhere (cwd might be altered if open cannot open cwd in RDONLY)
*/
int my_mkdir(const std::string & path, __mode_t mode);
int my_mkdir(const std::string && path, __mode_t mode);


#endif /* __linux__ */

#endif /* MYLINUXSYSFUNCS_H */