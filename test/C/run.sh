#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
PYTHON='python'
CC='gcc'

cd $(dirname $0)

if ! [ -z $1 ]
then
  rm ./workdir/saved_binary.sh 2> /dev/null
fi

RUN=$1
CODE=0

if [ -e ./workdir/saved_binary.sh ]
then
  source ./workdir/saved_binary.sh
fi

if [ -z $RUN ]
then
  echo "Usage: $0 path_to_parser_binary"
  rm ./workdir/saved_binary.sh 2> /dev/null
  exit 0
fi

if ! [ -x $RUN ]
then
  echo "Error: file \"$RUN\" is not executable"
  rm ./workdir/saved_binary.sh 2> /dev/null
  exit 0
fi

mkdir -p ./workdir

echo "RUN=$RUN" > ./workdir/saved_binary.sh

report_error(){
  echo -e "${RED}test [$(basename $fcmm)]" "$1" "${NC}"
  CODE=-1
}

for fcmm in ./*.cmm; do
  cp $fcmm ./workdir/a.cmm
  cp ${fcmm%.cmm}.in ./workdir/a.in
  cp template.c ./workdir/template.c

  if timeout --help > /dev/null 2>&1; then #if has `timeout` command
    timeout 5 $RUN ./workdir/a.cmm ./workdir/a.ir --ir
    ec=$?
    if [[ $ec -eq 124 ]]; then
      report_error "Time Limit Error when compile"
      continue
    elif [[ $ec -eq 0 ]]; then
      true
    elif [[ $ec -eq 1 ]]; then
      true
    else
      report_error "Runtime Error when compile"
      continue
    fi
  else
    $RUN ./workdir/a.cmm ./workdir/a.ir --ir
    ec=$?
    if [[ $ec -eq 0 ]]; then
      true
    elif [[ $ec -eq 1 ]]; then
      true
    else
      report_error "Runtime Error when compile"
      continue
    fi
  fi

  if timeout --help > /dev/null 2>&1; then #if has `timeout` command
    timeout 5 $CC ./workdir/template.c -o ./workdir/std.out
    ec=$?
    if [[ $ec -eq 124 ]]; then
      report_error "Time Limit Error when compile by GCC"
      continue
    elif [[ $ec -eq 0 ]]; then
      ./workdir/std.out < ./workdir/a.in > ./workdir/ans.out
    else
      report_error "Runtime Error when compile by GCC"
      continue
    fi
  else
    $CC ./workdir/template.c -o ./workdir/std.out
    ec=$?
    if [[ $ec -eq 0 ]]; then
      ./workdir/std.out < ./workdir/a.in > ./workdir/ans.out
    else
      report_error "Runtime Error when compile by GCC"
      continue
    fi
  fi

  if timeout --help > /dev/null 2>&1; then #if has `timeout` command
    timeout 8 $PYTHON ./irsim.py ./workdir/a.ir < ./workdir/a.in > ./workdir/a.out
    ec=$?
    if [[ $ec -eq 124 ]]; then
      report_error "Time Limit Error when execute IR"
      continue
    elif [[ $ec -eq 0 ]]; then
      true
    else
      report_error "Runtime Error when execute IR"
      continue
    fi
  else
    $PYTHON ./irsim.py ./workdir/a.ir < ./workdir/a.in > ./workdir/a.out
    ec=$?
    if [[ $ec -eq 0 ]]; then
      true
    else
      report_error "Runtime Error when execute IR"
      continue
    fi
  fi

  if diff ./workdir/ans.out ./workdir/a.out --strip-trailing-cr > /dev/null; then
    echo test [$(basename $fcmm)] matched
  else
    diff ./workdir/ans.out ./workdir/a.out --strip-trailing-cr | head -10
    report_error "mismatch"
  fi
done

exit $CODE