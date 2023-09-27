#!/bin/bash

if [ $# -lt 3 ]; then
  echo "Usage: $0 [--query|--loan] file1 [file2 file3 ...]"
  exit 1
fi

option=""
files=()

count=0

for arg in "$@"; do
  if [ "$arg" == "--query" ] || [ "$arg" == "--loan" ]; then
    option="${arg#--}"
  elif [ -f "$arg" ]; then
    files+=("$arg")
  else
    echo "Argument not valid [did you input a [--query|--loan] option?]"
    echo "Did you input valid files?"
  fi
done

for file in "${files[@]}"; do
  result=0
  while read -r key value; do
    if [ "$key" == "$option" ] && [[ "$value" =~ ^[0-9]+$ ]]; then
      result=$((result+value))
    fi
  done < "$file"
  count=$((count+result))
  echo "$file $result"
done

if [ "$option" == "query" ]; then
  echo "QUERY $count"
else
  echo "LOAN $count"
fi
