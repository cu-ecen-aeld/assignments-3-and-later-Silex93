#!/bin/bash
#A Script to search a directory for a string
#Author: Daniel Mendez
#Course : ECEN 5713: Advanced Embedded Software Development

#First check to see the correct number of arguments are provided
if [ $# -ne 2 ]; then
  echo "Usage: $0 <path to search> <string to look for>"
  echo "Script Exiting!"
  exit 1
fi
filesdir=$1
searchstr=$2
#Check to see if the first argument is a Path
if ! [ -d "$filesdir" ]; then
  echo "$filesdir is not a valid directory path"
  exit 1
fi

#Check to see if the second argument is a non-empty string
if [ -z "$searchstr" ]; then
  echo "$searchstr must not be empty"
  exit 1
fi


# Search for the string in files and count the matches and lines
# Count the files with the specified name
line_count=$(grep -r "$searchstr" "$filesdir" | wc -l)
file_count=$(grep -rl "$searchstr" "$filesdir"| wc -l)
#Get the real path incase the user submitted .
filesdir=$(realpath "$filesdir")


echo "The number of files are $file_count and the number of matching lines are $line_count"