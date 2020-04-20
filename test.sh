#!/bin/bash

INFO='\033[0;36m'
FAIL='\033[0;31m'
PASS='\033[0;32m'
RESET='\033[0m'

echo -e "${INFO}Running all files in test...${RESET}"

cd test

FILES=(../test/*.txt)
OUTPUT=()

cd ../bin

for f in "${FILES[@]}"; do
	echo -e "\n${INFO}### Running $f...${RESET}"
  ./cv --no-ui -i $f -o ../ubuntu_data/output.asm
  status=$?
  if [ $status -eq 0 ]; then
    docker run -it --rm -v $PWD/../ubuntu_data:/home/work cv_ubuntu /home/work/build.sh
    status=$?
  fi
  OUTPUT+=($status)
  if [ $status -eq 0 ]; then
    echo -e "${INFO}# ${PASS}PASS${RESET}"
  else
    echo -e "${INFO}# ${FAIL}FAIL${RESET}"
  fi
done

echo -e "\n${INFO}Results"
for ((i=0;i<${#OUTPUT[@]};++i)); do
  if [ "${OUTPUT[i]}" -eq 0 ]; then
    echo -e "${PASS}PASS ${INFO}${FILES[i]}${RESET}"
  else
    echo -e "${FAIL}FAIL ${INFO}${FILES[i]}${RESET}"
  fi
done
