#ifndef GETCWD_H
#define GETCWD_H

#ifdef __linux__

#include <fcntl.h>      // open (pathname, mode), O_RDONLY
#include <unistd.h>     // chdir(pathname), fchdir(file_descriptor) and close(file_descriptor)
#include <dirent.h>     // DIR*, opendir(dirname), dirent*, readdir(DIR*), closedir(DIR*)
#include <sys/stat.h>   // stat(pathname, stat_struct)), stat struct, fstat(file_descriptor, stat_struct)

#include <utility>      // std::pair (bits/stl_pair.h)
#include <vector>       // vector
#include <string>       // string
#include <cstring>      // strcmp

int GetCurrentWorkingDirectory(std::string & cwdPath);

#endif

#endif /* GETCWD_H */