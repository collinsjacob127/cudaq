#!/bin/bash

# Loop through all items (including hidden ones) in the current directory
for file in * .*; do
  # Check if it's a file or directory and ignore '.' and '..'
  if [ -e "$file" ] && [ "$file" != "." ] && [ "$file" != ".." ]; then
    # Run 'du -hs' on the file or directory and print the output
    du -hs "$file"
  fi
done
