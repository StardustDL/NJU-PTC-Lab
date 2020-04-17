#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
BOLD=$(tput bold)
NORMAL=$(tput sgr0)
PYTHON='python3'

cd $(dirname $0)

RUN=$1
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

report_error(){
  echo -e "${RED}${BOLD}test [$(basename $fcmm)]" "$1" "${NC}${NORMAL}"
}

for fcmm in ./*.cmm; do
  cp $fcmm ./workdir/a.cmm
  cp ${fcmm%.cmm}.json ./workdir/a.json

  $RUN ./workdir/a.cmm --semantics> ./workdir/a.out 2>&1;

  if $PYTHON ./check.py; then
    echo test [$(basename $fcmm)] matched
  else
    report_error "mismatch"
    CODE=-1
    continue
  fi
done

exit $CODE