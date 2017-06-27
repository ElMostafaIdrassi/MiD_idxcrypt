#ifdef __linux__

#include "GetCWD.h"

/* Credits go to Insane Coding's article : http://insanecoding.blogspot.fr/2007/11/pathmax-simply-isnt.html */

/* General idea (not thread safe because of usage of chdir) */

/* 1-Start from "." (initial directory)
* 2-Gather info about it
* 3-Gather info about the root "/" (root directory)
* 4-Chdir to ".." (containing directory)
* 5-Gather all the entries of it
* 6-Find the directory we were in before chdir by comparing each entry of the
* containing directory to the initial directory
* 7-Once found, get the name
* 8-Check if we reached root directory
* 9-If yes, form the path
* 10-If no, set initial directory to the current directory, go to 4 (recursively)
*/

/*  Define a type of objects that hold 2 different object/structure values in 1 object;
*  In this case, it will hold a dev_t and a ino_t structure;
*  The combination of the two identify uniquely every file;
*  Type : file_id;
*  dev_t : structure that holds device ID of device containing file;
*  ino_t : structure that holds file serial number;
*  */
typedef std::pair<dev_t, ino_t> file_id;

int RecursiveWorkingDirectoryLookup(file_id current_id, file_id root_id, std::vector<std::string> & path_components)
{
    bool found = false;
    struct stat stat_buf;

    if (current_id != root_id) {

        if (0 != chdir("..")){  // i.e. now current working directory is /home/user/NetBeansProjects
            return 1;
        }  

        DIR* dir = opendir("."); // Contains info about all directories under /home/user/NetBeansProjects
        dirent *entry = {}; // to collect these directories info (names...)
        
        if (nullptr == dir) { // error
            return 1;
        }

        while ((entry = readdir(dir)) != nullptr) { // as long as we didn't go in circle, and there are directories
            // we collect these directories info (except . and ..) to find the dir we were in before chdir("..")
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (0 != lstat(entry->d_name, &stat_buf)) { // error
                    closedir(dir);
                    return 1;
                }
                file_id child_id(stat_buf.st_dev, stat_buf.st_ino);
                if (current_id == child_id) { // we found our directory, add the name to the list and break loop
                    path_components.push_back(entry->d_name);
                    found = true;
                    break;
                }
            }
        }
        closedir(dir);

        if (found == true) { // we found our directory
            if (0 == stat(".", &stat_buf)) { // now we move to the next one
                current_id = file_id(stat_buf.st_dev, stat_buf.st_ino); // current_id now points to ..
            }
            else { // error
                return 1;
            }
        }
        else { // error
            return 1;
        }
        return (RecursiveWorkingDirectoryLookup(current_id, root_id, path_components));
    }
    if (current_id == root_id) { //Unless they're equal, we failed above
        return 0;
    }
    return 1;   // error
}

int GetCurrentWorkingDirectory(std::string & cwdPath)
{
    int initial_fd = 0; /* Will hold the current working directory file descriptor */
    struct stat stat_buf1{}, stat_buf2{}; /* stat structure which will hold file's unique info */
    int iStatus = 0;

    file_id current_id{};
    file_id root_id{};

    /*  Open the current directory and retrieve the file descriptor (>0 number) */
    if ((initial_fd = open(".", O_RDONLY)) > 0) { // no error, i.e. /home/user/NetBeansProjects/test_dirent

        /* Populate stat struct with file info of the file "referenced" by file descriptor */
        if (0 == fstat(initial_fd, &stat_buf1)) { // no error
            /* Create the file_id object for current working directory and store the structures in it */
            current_id = file_id(stat_buf1.st_dev, stat_buf1.st_ino);
        }
        else { // error
            close(initial_fd);
            iStatus = 1;
        }
    }
    else { // error
        close(initial_fd);
        iStatus = 1;
    }

    /* Now, for the root, our finish line */

    /* stat structure */
    if (0 == stat("/", &stat_buf2)) {
        /* file_id for / */
        root_id = file_id(stat_buf2.st_dev, stat_buf2.st_ino);
    }
    else { // error
        close(initial_fd);
        iStatus = 1;
    }
    
    if (0 == iStatus){

        std::vector<std::string> path_components = {};

        // Now we start recursing towards root each iteration, until we reach "/"
        int iStatus = RecursiveWorkingDirectoryLookup(current_id, root_id, path_components);

        if (iStatus == 0) {
            //Build the path
            cwdPath = "/";
            for (std::vector<std::string>::reverse_iterator i = path_components.rbegin(); i != path_components.rend(); ++i) {
                cwdPath += *i+"/";
                //cwdPath.push_back('/');
            }
            fchdir(initial_fd);
        }
    }

    close(initial_fd);

    return iStatus;
}

#endif
