#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
PYTHON='python3'

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
  exit 0
fi

if ! [ -x $RUN ]
then
  echo "Error: file \"$RUN\" is not executable"
  exit 0
fi

mkdir -p ./workdir

echo "RUN=$RUN" > ./workdir/saved_binary.sh

report_error(){
  echo -e "${RED}test [$(basename $fcmm)]" "$1" "${NC}"
  CODE=-1
}

for fcmm in ./tests/*.cmm; do
  cp $fcmm ./workdir/a.cmm
  cp ${fcmm%.cmm}.json ./workdir/a.json

  if timeout --help > /dev/null 2>&1; then #if has `timeout` command
    timeout 5 $RUN ./workdir/a.cmm --semantics> ./workdir/a.out 2>&1
    ec=$?
    if [[ $ec -eq 124 ]]; then
      report_error "Time Limit Error"
      continue
    elif [[ $ec -eq 0 ]]; then
      true
    elif [[ $ec -eq 1 ]]; then
      true
    else
      report_error "Runtime Error"
      continue
    fi
  else
    $RUN ./workdir/a.cmm --semantics> ./workdir/a.out 2>&1
    ec=$?
    if [[ $ec -eq 0 ]]; then
      true
    elif [[ $ec -eq 1 ]]; then
      true
    else
      report_error "Runtime Error"
      continue
    fi
  fi

  if $PYTHON ./check.py; then
    echo test [$(basename $fcmm)] matched
  else
    report_error "mismatch"
  fi
done

exit $CODE