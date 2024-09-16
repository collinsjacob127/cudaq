#!/bin/bash

# Loop through all items in the current directory
for file in *; do
  # Check if it's a file or directory
  if [ -e "$file" ]; then
    # Run 'du -hs' on the file or directory and print the output
    du -hs "$file"
  fi
done

