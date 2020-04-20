#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
CC='gcc'

cd $(dirname $0)

if ! [ -z $1 ]
then
  rm ./workdir/saved_binary.sh 2> /dev/null
fi

RUN=$1

if [ -e ./workdir/saved_binary.sh ]
then
  source ./workdir/saved_binary.sh
fi

CODE=0

if [ -z $RUN ]
then
  echo "Usage: $0 path_to_parser_binary"
  rm ./workdir/saved_binary.sh 2> /dev/null
  exit 0
fi

mkdir -p ./workdir

echo "RUN=$RUN" > ./workdir/saved_binary.sh

cp $RUN/* ./workdir
rm -f ./workdir/main.o
cp ./main.c ./workdir
cp ./Makefile ./workdir
cp ./unittest.h ./workdir

report_error(){
  echo -e "${RED}module [$(basename $fcmm)]" "$1" "${NC}"
  CODE=-1
}

for fcmm in ./test_*.c; do
  echo Testing module [$(basename $fcmm)]

  cp $fcmm ./workdir/test.c

  rm -f ./workdir/test

  make -C ./workdir -s

  ec=$?
  if [[ $ec -eq 0 ]]; then
    true
  elif [[ $ec -eq 1 ]]; then
    true
  else
    report_error "Compile Error"
    continue
  fi

  if ./workdir/test; then
    echo module [$(basename $fcmm)] passed
  else
    report_error "FAILED"
  fi
done

exit $CODE