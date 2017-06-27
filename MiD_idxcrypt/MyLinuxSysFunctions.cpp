/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef __linux__

#include "MyLinuxSysFunctions.h"

#include <vector>
#include <stdlib.h>

/*  Define a type of objects that hold 2 different object/structure values in 1 object;
*  In this case, it will hold a dev_t and a ino_t structure;
*  The combination of the two identify uniquely every file;
*  Type : file_id;
*  dev_t : structure that holds device ID of device containing file;
*  ino_t : structure that holds file serial number;
*  */
typedef std::pair<dev_t, ino_t> file_id;

DIR * opendirat(int fd_dir, const char * dirname) {
    // Open directory
    int fd;
    if ((fd = openat(fd_dir, dirname, O_RDONLY | O_NONBLOCK | O_DIRECTORY)) <= 0) return nullptr;

    // Create directory handle
    DIR *result;
    if ((result = fdopendir(fd)) == nullptr) close(fd);
    return result;
}

/*
 * RecursiveWorkingDirectoryLookup implementation using opendirat and file descriptors
 * Doesn't use opendir; Uses opendirat instead
 */
int RecursiveDirectoryLookup(DIR * current_dir, file_id current_id, file_id root_id, std::vector<std::string> & path_components)
{
    bool found = false;
    struct stat stat_buf{};
    int fd = 0;

    if (current_id != root_id) {    // we've not reached the root "/" yet
        
        // We're sure dirfd(current_dir) is > 0 since we've already checked in the last pass
        
        DIR * parent_dir = opendirat(dirfd(current_dir), ".."); // Contains info about all files (entries) in parent directory
        closedir(current_dir);   // closes fd associated with current_dir

        if (nullptr != parent_dir) { // opendirat OK
               
            // update fd to parent directory
            if ((fd = dirfd(parent_dir)) > 0){  // dirfd OK; this file desc can only be closed using closedir of its dir

                dirent * entry = {};             // to collect these files info (i.e. names)

                while ((entry = readdir(parent_dir)) != nullptr) { // as long as we didn't go in circle, and there are (still) files/entries within

                    // we collect these entries info (except . and ..) to find the dir we were in before opendir("..")
                    if (0 != strcmp(".", entry->d_name) && 0 != strcmp("..", entry->d_name)) {

                        int child_fd = openat(fd, entry->d_name, O_RDONLY | O_NONBLOCK | O_DIRECTORY);

                        if (child_fd <= 0){ // openat error
                            if (errno != ENOTDIR && errno != EACCES){
                                std::cerr << "An error occured when attempting to open a component within path (RecursiveWorkingDirectoryLookup - child openat). Error code : " << errno << ".\n";
                                closedir(parent_dir);
                                return 1;
                            }
                            /* else continue, because current_dir is either not a directory
                             * or the permission was denied, which is not possible if it was
                             * the directory we're looking for (which has read + execution permission)
                             */ 
                            else continue;
                        }

                        // else, openat OK
                        if (0 == fstat(child_fd, &stat_buf)) // fstat OK
                        {
                            close(child_fd);
                            file_id child_id(stat_buf.st_dev, stat_buf.st_ino);
                            if (current_id == child_id) { // we found our directory, add the name to the list and break loop
                                path_components.push_back(entry->d_name);
                                found = true;
                                break;
                            }
                        }
                        // else, fstat error
                        else{
                            close(child_fd);
                            closedir(parent_dir);
                            std::cerr << "An error occured when attempting to populate stat struct with info about a component within path (RecursiveWorkingDirectoryLookup - fstat). Error code : " << errno << ".\n";
                            return 1;
                        }
                    }
                }

                if (found == true) { // we found our directory

                    /* now we move to the next one, we update the current dir id
                     * we update the current dir id with info from fd parent_fd
                     */
                    if (0 == fstat(fd, &stat_buf)){
                        current_id = file_id(stat_buf.st_dev, stat_buf.st_ino); // current_id now points to ..
                    }
                    else { // fstat error
                        closedir(parent_dir);
                        std::cerr << "An error occured while attempting to update stat struct with current dir info (RecursiveWorkingDirectoryLookup - fstat). Error code : " << errno << ".\n";
                        return 1;
                    }
                }
                else { // error, our directory not found
                    std::cerr << "Couldn't find a component of the path (RecursiveWorkingDirectoryLookup)...\n";
                    closedir(parent_dir);
                    return 1;
                }
                return (RecursiveDirectoryLookup(parent_dir, current_id, root_id, path_components));
            }
            else{ // dirfd error
                closedir(parent_dir);
                std::cerr << "An error occured while attempting to get the file descriptor of the parent directory. (RecursiveWorkingDirectoryLookup - dirfd). Error code : " << errno << ".\n";
                return 1;
                
            }
        }
        else {  //  opendirat error
            std::cerr << "An error occured when attempting to retrieve list of directories within input path's parent directory (RecursiveWorkingDirectoryLookup - opendir). Error code : " << errno << ".\n";
            return 1;
        }
    }
    if (current_id == root_id) { //Unless they're equal, we failed above
        closedir(current_dir);
        return 0;
    }
}

/*
 * RecursiveWorkingDirectoryLookup implementation using opendir and up_path which gets prepended
 * with /.. in each pass
 * Uses opendir
 */
int RecursiveDirectoryLookup(std::string & up_path, file_id current_id, file_id root_id, std::vector<std::string> & path_components)
{
    bool found = false;
    struct stat stat_buf{};
    int fd = 0;

    if (current_id != root_id) {    // we've not reached the root "/" yet

        DIR * dir = opendir(up_path.data()); // Contains info about all files (entries) in parent directory      
        
        if (nullptr == dir) { // opendir error
            std::cerr << "An error occured when attempting to retrieve list of directories within input path's parent directory (RecursiveWorkingDirectoryLookup - opendir). Error code : " << errno << ".\n";
            return 1;
        }
        dirent * entry = {};             // to collect these files info (i.e. names)
        
        up_path += "/";

        while ((entry = readdir(dir)) != nullptr) { // as long as we didn't go in circle, and there are (still) files/entries within
            
            // we collect these entries info (except . and ..) to find the dir we were in before opendir("..")
            if (0 != strcmp(".", entry->d_name) && 0 != strcmp("..", entry->d_name)) {
                
                DIR * current_dir = opendir((up_path+entry->d_name).data());
                
                if (nullptr == current_dir){ // opendir error
                    if (errno != ENOTDIR && errno != EACCES){
                        std::cerr << "An error occured when attempting to open a component within path (RecursiveWorkingDirectoryLookup - current opendir). Error code : " << errno << ".\n";
                        closedir(dir);
                        return 1;
                    }
                    /* else continue, because current_dir is either not a directory
                     * or the permission was denied, which is not possible if it was
                     * the directory we're looking for (which has read + execution permission)
                     */ 
                    else continue;
                }
                
                // else, opendir OK
                if ((fd = dirfd(current_dir)) > 0 && 0 == fstat(fd, &stat_buf)) // dirfd and fstat OK
                {
                    close(fd);
                    closedir(current_dir);
                    file_id child_id(stat_buf.st_dev, stat_buf.st_ino);
                    if (current_id == child_id) { // we found our directory, add the name to the list and break loop
                        path_components.push_back(entry->d_name);
                        found = true;
                        break;
                    }
                }
                // else, dirfd or fstat error
                else{
                    if (fd > 0) close(fd);
                    closedir(current_dir);
                    closedir(dir);
                    std::cerr << "An error occured when attempting to populate stat struct with info about a component within path (RecursiveWorkingDirectoryLookup - dirfd, fstat). Error code : " << errno << ".\n";
                    return 1;
                }
            }
        }

        if (found == true) { // we found our directory
            up_path += "..";      
            // now we move to the next one, we update the current dir id
            if ((fd = dirfd(dir)) > 0 && 0 == fstat(fd, &stat_buf))   // we update the current dir id
            {
                current_id = file_id(stat_buf.st_dev, stat_buf.st_ino); // current_id now points to ..
                close(fd);
                closedir(dir);
            }
            else { // fstat or dirfd error
                if (fd > 0) close(fd);
                closedir(dir);
                std::cerr << "An error occured while attempting to update stat struct with current dir info (RecursiveWorkingDirectoryLookup - dirfd, fstat). Error code : " << errno << ".\n";
                return 1;
            }
        }
        else { // error
            std::cerr << "Couldn't find a component of the path (RecursiveWorkingDirectoryLookup)...\n";
            closedir(dir);
            return 1;
        }
        return (RecursiveDirectoryLookup(up_path, current_id, root_id, path_components));
    }
    if (current_id == root_id) { //Unless they're equal, we failed above
        return 0;
    }
}

int GetCurrentWorkingDirectory(std::string & cwdPath)
{
    struct stat stat_buf1{}, stat_buf2{};   /* stat structure which will hold file's unique info */
    int iStatus = 0;
    DIR * parent_dir = nullptr;

    file_id current_id{};
    file_id root_id{};
    
    /* Populate stat struct with file info of the current folder/file */
    if (0 != stat(".", &stat_buf1)) { // error stat
        std::cerr << "An error occured while attempting to populate stat struct with info about current directory. (GetCurrentWorkingDirectory - fstat) Error code : " << errno << ".\n";
        iStatus = 1;
    }
    else { // stat OK
        /* Create the unique file_id object for current directory and store the structures in it */
        current_id = file_id(stat_buf1.st_dev, stat_buf1.st_ino);

        /* Now, for the root, our finish line */

        /* stat structure */
        if (0 != stat("/", &stat_buf2)) {   // error stat
            std::cerr << "An error occured while attempting to populate stat struct with info about root directory / . (GetCurrentWorkingDirectory - stat) Error code : " << errno << ".\n";
            iStatus = 1;
        }
        else { // stat OK
            /* file_id for / */
            root_id = file_id(stat_buf2.st_dev, stat_buf2.st_ino);

            std::vector<std::string> path_components = {};  // will hold path's components
            // std::string up_path = "..";

            if ((parent_dir = opendir(".")) == nullptr){
                std::cerr << "An error occured while attempting to open current directory (GetCurrentWorkingDirectory - opendir parent_dir). Error code : " << errno << ".\n";
                iStatus = 1;
            }
            else {
                if (dirfd(parent_dir) <= 0){
                    std::cerr << "An error occured while attempting to get file descriptor of current directory (GetCurrentWorkingDirectory - dirfd parent_dir). Error code : " << errno << ".\n";
                    closedir(parent_dir);
                    iStatus = 1;
                }
                else{
                    /* Now we start recursing towards parent directory, until we reach "/" = root */
                    iStatus = RecursiveDirectoryLookup(parent_dir, current_id, root_id, path_components);

                    if (iStatus != 0) {
                        std::cerr << "There was an error during the recursive lookup of the current working directory. (GetCurrentWorkingDirectory - RecursiveWorkingDirectoryLookup) Error code : " << errno << ".\n";
                    }
                    else {
                        // Build the path
                        cwdPath = "/";
                        // reverse because path components have been added in reverse order 
                        for (std::vector<std::string>::reverse_iterator i = path_components.rbegin(); i != path_components.rend(); ++i) {
                            cwdPath += *i+"/";
                        }
                        if (cwdPath.size()>1) cwdPath.pop_back(); // to get rid of the trailing '/' in case current working directory != '/' root
                    }
                } 
            }
        }
    }
    return iStatus;
}

int getAbsolutePath(const std::string & relativePath, std::string & absolutePath)
{   
    if (relativePath[0] == '/') {
        absolutePath = relativePath;
        return 0;
    }
    
    std::string pathCopy = relativePath;
    int current_fd = 0;
    DIR * current_dir = nullptr;
    int iStatus = 0;
    
    struct stat stat_buf1{}, stat_buf2{};   /* stat structure which will hold file's unique info */
    file_id current_id{};
    file_id root_id{};
    std::string dir{}, base{};
    
    if (pathCopy.back() != '/') pathCopy.push_back('/');
    
    dirname_base_separator(pathCopy, dir, base);
    
    current_dir = my_opendir(dir);
    
    if (nullptr != current_dir){   // my_open OK
        if ((current_fd = dirfd(current_dir)) > 0){   // dirfd OK
            if (0 != fstat(current_fd, &stat_buf1)) { // error fstat
                std::cerr << "An error occured while attempting to populate stat struct with info about relative path. (getAbsolutePath - fstat) Error code : " << errno << ".\n";
                closedir(current_dir);
                iStatus = 1;
            }
            else { // fstat OK

                /* Create the unique file_id object for current directory and store the structures in it */
                current_id = file_id(stat_buf1.st_dev, stat_buf1.st_ino);

                /* Now, for the root, our finish line */

                /* stat structure */
                if (0 != stat("/", &stat_buf2)) {   // error stat
                    std::cerr << "An error occured while attempting to populate stat struct with info about root directory / . (getAbsolutePath - stat) Error code : " << errno << ".\n";
                    closedir(current_dir);
                    iStatus = 1;
                }
                else { // stat OK
                    /* file_id for / */
                    root_id = file_id(stat_buf2.st_dev, stat_buf2.st_ino);

                    std::vector<std::string> path_components = {};  // will hold path's components
                    
                    /* Now we start recursing starting from parent of relative path, until we reach "/" = root */
                    iStatus = RecursiveDirectoryLookup(current_dir, current_id, root_id, path_components);

                    if (iStatus != 0) {
                        std::cerr << "There was an error during the recursive lookup of the relative path. (GetCurrentWorkingDirectory - RecursiveDirectoryLookup) Error code : " << errno << ".\n";
                    }
                    else {
                        // Build the path
                        absolutePath = "/";

                        // Translate ".."
                        if ((stat_buf1.st_mode & S_IFMT) == S_IFDIR && base==".."){
                            path_components.erase(path_components.begin());
                        }

                        // reverse because path components have been added in reverse order 
                        for (std::vector<std::string>::reverse_iterator i = path_components.rbegin(); i != path_components.rend(); ++i) {
                            absolutePath += *i+"/";
                        }

                        // Add base to path only if it's not ".." or "."
                        if (base != ".." && base!=".") absolutePath += base;
                        
                        if (absolutePath.back() == '/') absolutePath.pop_back();
                    }
                }
            }
        }
        else{   // dirfd error
            std::cerr << "An error occured while attempting to the file descriptor of relative path. (getAbsolutePath - dirfd) Error code : " << errno << ".\n";
            closedir(current_dir);
            iStatus = 1;
        }
    }
        
    else{   // my_opendir error
        std::cerr << "An error occured while trying to get the relative path directory (getAbsolutePath - my_opendir). Last error code : " << errno << ".\n";
        iStatus = 1;
    }
    
    return iStatus;
}

int getAbsolutePath(const std::string && relativePath, std::string & absolutePath)
{
	std::string relativePathCopy = relativePath;
	return (getAbsolutePath(relativePathCopy, absolutePath));
}

void dirname_base_separator(const std::string & path, std::string & dir, std::string & base)
{
    if (path == "." || path == "./") {
        dir = "./";
        base = "";
        return;
    }
    
    if (path == ".." || path == "./.."){
        dir = "..";
        base = "";
        return;
    }

    if (path == "/") {
        dir = "/";
        base = "";
        return;
    }

    std::size_t found2 = 0;
    std::string pathCopy = path;
    
    // Case : path = filename => this function will fail with out_of_boubdary error
    //      =>  if path begins with '/' => dont append ./, otherwise do it
    
    if (pathCopy[0] != '/') pathCopy = "./" + pathCopy; // if relative
    
    // std::string temp{};  for testing
    
    // Possible paths : 
    //      /{succession_of_folders}/, /{succession_of_folders}/filename, 
    //      {relative_subpath}/, {relative_subpath}/filename
    //      /{succession_of_folders}/{combination_of_/_and_./}, /{succession_of_folders}/filename/{combination_of_/_and_./}, 
    //      {relative_subpath}/{combination_of_/_and_./}, {relative_subpath}/filename/{combination_of_/_and_./}
    //      => Either ends with / or doesn't
    found2 = pathCopy.find_last_of('/');
    
    // temp = pathCopy.substr(0, found2 + 1); // for testing
    
    // If path ends with '/' => Get rid of slashes and "./"s at the end of path
    if (found2 == pathCopy.size() - 1){
        do{
            if (pathCopy.size() >= 3 && pathCopy.substr(pathCopy.size()-3, 3) == "/./"){
                pathCopy.pop_back();
                pathCopy.pop_back();
            }
            else if (pathCopy[pathCopy.size()-1] == '/') pathCopy.pop_back();
            
            // found2 = pathCopy.size();  // for testing
            
        }while(pathCopy[pathCopy.size()-1] == '/');
    }

    // Case : pathCopy only contains succession of "/"s and "./"s
    //		=> Result of striping will be "", so we look at path's first char
    if (pathCopy == "") {
        if (path[0] == '/') {
            //pathCopy = "/";
            dir = "/";
            base = "";
            return;
        }
        if (path[0] == '.') {
            //pathCopy = "./";
            dir = "./";
            base = "";
            return;
        }
    }
    
    found2 = pathCopy.find_last_of('/');
    
    dir = pathCopy.substr(0, found2+1);
    base = pathCopy.substr(found2+1);
    
}

/*
 * N.B : current_dir should be initialized with nullptr
 */
DIR * RecursiveOpendir(DIR * current_dir, std::string & path)
{
    // First time calling RecursiveOpendir with a path that can be dealt with using opendir
    if (current_dir == nullptr && path.size() <= pathmax){
        return (opendir(path.data()));
    }
    if (current_dir == nullptr && path.size() == pathmax+1 && path.back() == '/'){
        return (opendir(path.substr(0, pathmax).data()));
    }
    
    __int64 size = 0;
    int iStatus = 0;
    std::size_t found1 = 0, found2 = 0, found3 = 0;
    int end = 0;
    DIR * new_dir = nullptr;
    
    std::string temp{}; // for testing

    // Case : last block only ////// => size will be 0 and we'll prepend / => size will become 1 > 0 which we dont want
    //      => add current_dir==nullptr before prepending '/' (first call only)
    if (current_dir == nullptr && path.back() != '/') path.push_back('/');
    size = path.size();
    
    if (size > 0){
        // temp = path.substr(found1, MY_MAX_PATH);   //for testing
        
        /* 
         * Since normally, path_max (i.e. 256) > name_max (i.e. 255), 
         * the case of the following substring not including a '/' will never happen 
         * because of the push_back
         */
        found2 = path.substr(found1, pathmax).find_last_of('/');   // if size < 256, substr will cut at size
        
        /*
        * Case :   path = /file1/file2 or /file1/file2/ with file1 length = 255 
        *      =>  found2 = 0 since the last '/' found is at the beginning
        *      =>  the substring we feed opendir will be empty 
        *      =>  make its size = 1 (with end) to opendir to '/' then make (end = 0) and continue
        *      N.B : this may only happen when current_dir=nullptr (at the start)
        */
        if (found2 == 0) end = 1;

        found3 = found2;
        
        // temp = path.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Get rid of extra slashes and "./" in the beginning of the substring we feed to the opendir 
         * Only when not firt chunk (current_dir != nullptr), otherwie opendir will deal with them
         * Necessary in the middle of the path if there is a //// for example => opendir will opendir /
         */
        if (current_dir != nullptr){
            while(path.substr(found1,found2 + end + 1).substr(0,2) == "./"){
                found1 += 2;
                found2 -=2;
            }
            while (path.substr(found1,found2 + end + 1)[0] == '/'){
                found1++;
                found2--;
            }
        }
         
        // temp = path.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Case : pathCopy.substr(found1, found2 + end + 1) contained //////.//././././ or //// or ././././
         *      => Getting rid of these extra slashes and ./ leads to an empty substring
         *      => opendir empty : undefined behaviour?
         *      => don't opendir (do nothing)
         * May only happen when not first chunk (current_dir != nullptr)
         */
        if (path.substr(found1,found2 + end + 1) == ""){
            new_dir = current_dir;
            iStatus = 0;
        }
        else{
            if (current_dir == nullptr){   // 1st time calling RecursiveOpendir
                if (nullptr != (current_dir = opendir(path.substr(found1,found2 + end + 1).data()))) // opendir OK
                {
                    if ((dirfd(current_dir)) > 0){  // dirfd OK, preparing for next pass opendirat
                        new_dir = current_dir;
                        iStatus = 0;
                    }
                    else{   // dirfd error
                        iStatus = 1;
                        closedir(current_dir);
                        std::cerr << "An error occured. RecursiveOpendir - 1st dirfd. Error code : " << errno << ".\n";
                    }
                } 
                else{   // opendir error
                    std::cerr << "An error occured. RecursiveOpendir - 1st opendir. Error code : " << errno << ".\n";
                    iStatus = 1;   
                } 
            }
            else{
                if (nullptr != (new_dir = opendirat(dirfd(current_dir), path.substr(found1,found2 + end + 1).data()))){ // opendirat OK
                    closedir(current_dir);
                    if ((dirfd(new_dir)) > 0) iStatus = 0;  // dirfd OK, preparing for next pass opendirat
                    else{   // dirfd error
                        iStatus = 1;
                        closedir(new_dir);
                        std::cerr << "An error occured. RecursiveOpendir - 2nd dirfd. Error code : " << errno << ".\n";
                    }
                }
                else{   // opendirat error
                    iStatus = 1;
                    closedir(current_dir);
                    std::cerr << "An error occured. RecursiveOpendir - opendirat. Error code : " << errno << ".\n";
                }
            }
        }
         
        if (iStatus == 0){
            found1 += found2 + 1;    // +1 to avoid "/" for next chdir to not start from "/" = root
            size -= found3 + 1;
            temp = path.substr(found1, size);   // because no const in function
            return (RecursiveOpendir(new_dir, temp));
        }
        else return nullptr;
    }
    
    return current_dir;
}

int my_chdir(const std::string & path)
{
    std::string pathCopy = path;
    std::size_t found1 = 0, found2 = 0, found3 = 0;
    __int64 size;     // path length in bytes
    int iStatus = 0;
    int current_fd;
    int end = 0;
    std::string cwd{};
  
    // Get the current working directory to chdir back in case of an error
    current_fd = open(".", O_RDONLY);  // better with O_SEARCH in case cwd is not readable
    
    // std::string temp{};   //for testing
    
    /* 
     * Case :   path = /file or /file/ or /succession_of_files or /succession_of_files/ with total length <= MY_MAX_PATH 
     *      =>  Call chdir
     */
    if (pathCopy.size() <= pathmax) return (chdir(pathCopy.data()));
    
    /* 
     * Case :   path = /file/ with file length = 255 or /succession_of_files/ with succession_of_files length = 255 
     *      =>  total length = 257 > MY_MAX_PATH
     *      =>  Strip the trailing '/' and chdir (new total length = MY_MAX_PATH)
     */
    if (pathCopy.size() == pathmax+1 && pathCopy.back() == '/') return (chdir(pathCopy.substr(0, pathmax).data()));   
    
    /* 
     * N.B.1    In all what follows, path length is > MY_MAX_PATH
     * N.B.2    Usually : PATH_MAX > NAME_MAX. Here, MY_MAX_PATH = 256 > NAME_MAX
     * Case :   path = (long_path)/file with file length < MY_MAX_PATH or (long_path)/succession_of_files with succession_of_files length < MY_MAX_PATH
     *      =>  algo reaches first char of file (or succession_of_files) and needs to find last '/' to create substring for chdir
     *      =>  push_back('/')
     */
    if (pathCopy.back() != '/') pathCopy.push_back('/');
    size = pathCopy.size();
    
    do{
        // temp = pathCopy.substr(found1, MY_MAX_PATH);   //for testing
        
        /* 
         * Since normally, path_max (i.e. 256) > name_max (i.e. 255), 
         * the case of the following substring not including a '/' will never happen 
         * because of the push_back
         */
        found2 = pathCopy.substr(found1, pathmax).find_last_of('/');   // if size < 256, substr will cut at size
        
        /*
        * Case :   path = /file1/file2 or /file1/file2/ with file1 length = 255 
        *      =>  found2 = 0 since the last '/' found is at the beginning
        *      =>  the substring we feed chdir will be empty 
        *      =>  make its size = 1 (with end) to chdir to '/' then make (end = 0) and continue
        *      N.B : this can only happen when found1 = 0 (at the start)
        */
        if (found2 == 0) end = 1;

        found3 = found2;
        
        // temp = pathCopy.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Get rid of extra slashes and "./" in the beginning of the substring we feed to the chdir 
         * Unless it is in the start of the path, chdir will deal with them
         */
        if (found1 != 0){
            while(pathCopy.substr(found1,found2 + end + 1).substr(0,2) == "./"){
                found1 += 2;
                found2 -=2;
            }
            while (pathCopy.substr(found1,found2 + end + 1)[0] == '/'){
                found1++;
                found2--;
            }
        }
         
        // temp = pathCopy.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Case : pathCopy.substr(found1, found2 + end + 1) contained //////.//././././ or //// or ././././
         *      => Getting rid of these extra slashes and ./ leads to an empty substring
         *      => chdir to empty : undefined behaviour?
         *      => don't chdir, set iStatus to 0
         */
        if (pathCopy.substr(found1,found2 + end + 1) == "") iStatus = 0;
        else
            iStatus = chdir(pathCopy.substr(found1,found2 + end + 1).data()); // including trailing '/'
        
        end = 0;
        
        if (0 != iStatus){
            std::cerr << "An error occured while changing directory. (my_chdir - chdir) Error code : " << errno << ".\n";
            if (current_fd > 0) fchdir(current_fd);
            else std::cerr << "Warning! Current working directory may have been altered!\n";
            break;
        }
        
        // GetCurrentWorkingDirectory(cwd);  //for testing
        
        found1 += found2 + 1;    // +1 to avoid "/" for next chdir to not start from "/" = root
        size -= found3 + 1;
    }while(size > 0);
    
    return iStatus;
}

int my_chdir(const std::string && path)
{
    std::string pathCopy = path;
    return (my_chdir(pathCopy));
}

/*
 * N.B : All path components should have Read AND Search permission ON.
 *       Otherwise, this implementation shall fail
 */
DIR * my_opendir(const std::string & path)
{
    DIR * current_dir = nullptr;
    std::string pathCopy = path;
    return (RecursiveOpendir(current_dir, pathCopy));
}
DIR * my_opendir(const std::string && path)
{
    std::string pathCopy = path;
    return (my_opendir(pathCopy));
}

int RecursiveOpen(int current_fd, std::string & path, int oflag)
{
    // First time calling RecursiveOpen with a path that can be dealt with using open
    if (current_fd == 0 && path.size() <= pathmax){
        return (open(path.data(), oflag));
    }
    if (current_fd == 0 && path.size() == pathmax+1 && path.back() == '/'){
        return (open(path.substr(0, pathmax).data(), oflag));
    }
    
    __int64 size = 0;
    int iStatus = 0;
    std::size_t found1 = 0, found2 = 0, found3 = 0;
    int end = 0;
    int new_fd = 0;
    
     std::string temp{}; // for testing
     
    // Case : last block only ////// => size will be 0 and we'll prepend / => size will become 1 > 0 which we dont want
    //      => add condition of current_fd=0 before prepending '/' so that it does it only at the beginning (first call)
    if (current_fd == 0 && path.back() != '/') path.push_back('/');

    size = path.size();
    
    if (size > 0){
         temp = path.substr(found1, pathmax);   //for testing
        
        /* 
         * Since normally, path_max (i.e. 256) > name_max (i.e. 255), 
         * the case of the following substring not including a '/' will never happen 
         * because of the push_back
         */
        found2 = path.substr(found1, pathmax).find_last_of('/');   // if size < 256, substr will cut at size
        
        /*
        * Case :   path = /file1/file2 or /file1/file2/ with file1 length = 255 
        *      =>  found2 = 0 since the last '/' found is at the beginning
        *      =>  the substring we feed opendir will be empty 
        *      =>  make its size = 1 (with end) to open to '/' then make (end = 0) and continue
        *      N.B : this may only happen when current_fd=0 (at the start)
        */
        if (found2 == 0) end = 1;

        found3 = found2;
        
         temp = path.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Get rid of extra slashes and "./" in the beginning of the substring we feed to the open
         * Only when not firt chunk (current_fd != 0), otherwie open will deal with them
         * Necessary in the middle of the path if there is a //// for example => open will open /
         */
        if (current_fd != 0){
            while(path.substr(found1,found2 + end + 1).substr(0,2) == "./"){
                found1 += 2;
                found2 -=2;
            }
            while (path.substr(found1,found2 + end + 1)[0] == '/'){
                found1++;
                found2--;
            }
        }
         
         temp = path.substr(found1,found2 + end + 1);    //for testing
         
        /*
         * Case : pathCopy.substr(found1, found2 + end + 1) contained //////.//././././ or //// or ././././
         *      => Getting rid of these extra slashes and ./ leads to an empty substring
         *      => open empty : undefined behaviour?
         *      => don't open (do nothing)
         * May only happen when not first chunk (current_fd != 0)
         */
        if (path.substr(found1,found2 + end + 1) == ""){
            new_fd = current_fd;
            iStatus = 0;
        }
        else{
            if (current_fd == 0){   // 1st time calling RecursiveOpen
                if (0 < (current_fd = open(path.substr(found1,found2 + end + 1).data(), O_RDONLY))) // open OK
                {
                    new_fd = current_fd;
                    iStatus = 0;
                } 
                else{   // open error
                    std::cerr << "An error occured. RecursiveOpen - 1st open. Error code : " << errno << ".\n";
                    iStatus = 1;   
                } 
            }
            else{
                // Case : pathCopy contains filename/ or {bunch of folders}/filename/
                //      open will fail because it will try to open it as a directory (assumes it is)
                //      because of the last '/' altough it is a file (error code 20 : not directory)
                //      => we dont pop last char of pathCopy if it is '/', but rather call open with substring
                //      with found2 decremented
                if (size <= 256){
                    found2--;
                }
                if (0 < (new_fd = openat(current_fd, path.substr(found1,found2 + end + 1).data(), size <= 256 ? oflag : O_RDONLY))){ // openat OK
                    close(current_fd);
                    iStatus = 0;
                }
                else{   // opendirat error
                    iStatus = 1;
                    close(current_fd);
                    std::cerr << "An error occured. RecursiveOpen - openat. Error code : " << errno << ".\n";
                }
            }
        }
         
        if (iStatus == 0){
            found1 += found2 + 1;    // +1 to avoid "/" for next chdir to not start from "/" = root
            size -= found3 + 1;
            temp = path.substr(found1, size);   // because no const in function
            return (RecursiveOpen(new_fd, temp, oflag));
        }
        else return 0;
    }
    
    return current_fd;
}

/*
 * N.B : All path components should have Read AND Search permission ON.
 *       Last file/dir should have the permission defined by oflag
 *       Otherwise, this implementation shall fail
 *       (No O_SEARCH flag yet in GNU/Linux)
 */
int my_open(const std::string & path, int oflag)
{
    int current_fd = 0;
    std::string pathCopy = path;
    return (RecursiveOpen(current_fd, pathCopy, oflag));
}
int my_open(const std::string && path, int oflag)
{
    std::string pathCopy = path;
    return (my_open(pathCopy, oflag));
}

/*
 * Uses chdir
 */
int my_mkdir(const std::string & path, __mode_t mode)
{
    std::string cwdPath{}; // to get back to current directory after chdir
    int iStatus = 0;
    int current_fd;
    int ret = 1;    // return value is 1 if unsuccessful
    
    if (path.size() <= pathmax) return (mkdir(path.data(), mode)); // no need to chdir
    
    if (path.size() == pathmax+1 && path.back() == '/')
        return(mkdir(path.substr(0, pathmax).data(), mode));
    
    // Get the current working directory to chdir back in case of an error
    current_fd = open(".", O_RDONLY);  // better with O_SEARCH

    /* First, separate dirname and base */

    std::string dir{}, base{};
    dirname_base_separator(path, dir, base);

    /* Chdir to dirname, and if successful, mkdir base */
    iStatus = my_chdir(dir);
    if (0 == iStatus) ret = mkdir(base.data(), mode);
    else {  // my_chdir error
        std::cerr << "An error occured while attempting to change directory. (my_mkdir - my_chdir) Last error code : " << errno << ".\n";
        if (current_fd > 0) fchdir(current_fd);
        else std::cerr << "Warning! Current working directory may have been altered!\n";
    }
    // Chdir back to precedent cwd
    if (current_fd > 0) fchdir(current_fd);
    else std::cerr << "WARNING! Current working directory might have been altered!\n";
    
    return ret;
}

int my_mkdir(const std::string && path, __mode_t mode)
{
    std::string pathCopy = path;
    return (my_mkdir(pathCopy, mode));
}


#endif
