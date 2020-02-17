#!/bin/bash

echo "Running all files in test..."

cd test

for f in *_*.txt; do
  echo
	echo "### Running $f..."
	(
	  cd ../bin
	  ./cv -i ../test/$f -o $f.o && echo "Success" || echo "Error"
	)
done
