#!/bin/bash
#A Script to search a directory for a string
#Author: Daniel Mendez
#Course : ECEN 5713: Advanced Embedded Software Development

#First check to see the correct number of arguments are provided
if [ $# -ne 2 ]; then
  echo "Usage: $0 <file to write> <string to write in file>"
  echo "Script Exiting!"
  exit 1
fi
writefile=$1
textstr=$2
#Check to see if the first argument is a writefile
if [ -z "$writefile" ]; then
  echo "writefile must not be empty"
  exit 1
fi


#Check to see if the second argument is a non-empty string
if [ -z "$textstr" ]; then
  echo "$textstr must not be empty"
  exit 1
fi


# Check if the given filename exists and is not a directory
if [ ! -e "$writefile" ]; then
  mkdir -p "$(dirname "$writefile")" || { echo "Error creating directories for '$writefile'"; exit 1; }
  touch "$writefile" || { echo "Error creating file '$writefile'"; exit 1; }
elif [ ! -f "$writefile" ]; then
  echo "'$writefile' is not a valid filename"
  exit 1
fi

# Write the text to the file
echo "$textstr" > "$writefile"
echo "Text written to '$writefile'"