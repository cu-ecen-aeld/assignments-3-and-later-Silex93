/*****************************************************************************
 * Author: Daniel Mendez
 * Course : ECEN 5713
 * Assingment 2
 *
 *****************************************************************************/
/**
 * @file writer.c
 * @brief Writer Script
 *
 * This project consists of a script that takes two arguments, a file location and a string. Upon executing,
 * the provided string is written to the file.
 *
 * @date 10/09/2023
 * @version 1.0
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>


int main(int argc, char *argv[]){
    //First setup logging

    openlog(NULL,0,LOG_USER);
    int fd;
    //First we check to see if there are the right amount of args

    if(argc != 3){
     //   printf("Usage: %s <file path> <string>\n", argv[0]);
        syslog(LOG_ERR,"Usage: %s <file path> <string>\n", argv[0]);
        return 1;
    }

    char *writefile = argv[1];
    char *textstr = argv[2];

    //Open the file for both reading and writing
    fd = creat(writefile,0644);
    // Check to see if there was an error opening the file
    if(fd == -1){
       // printf("Error opening the file at %s",writefile);
        syslog(LOG_ERR,"Error opening the file at %s",writefile);
        return 1;
    }
    //Now we get the length of our string
    size_t bytes = strlen(textstr);
    //Then we try to write to the file
    syslog(LOG_DEBUG,"Writing %s to %s",textstr,writefile);
    ssize_t res = write(fd,textstr,bytes);

    //Check if the result is not -1
    if(res == -1){
        //printf("Error writing to file");
        syslog(LOG_ERR,"Error writing to file");
        close(fd);
        return -1;
    }
    else{
        //printf("%s written to %s!",textstr,writefile);
        close(fd);
    }
    //printf("Done");
    return 0;
}