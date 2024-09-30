#!/bin/bash

# This scripts lets you check which minimum GLIBC version an executable requires.
# Simply run './glibc-check.sh path/to/your/binary'
#
# You can set `VER_LIMIT` however low you want, although I (fasterthanlime)
# feel like `2.13` is a good target (For reference, Ubuntu 12.04 has GLIBC 2.15)
# VER_LIMIT="${VER_LIMIT:-2.29}"
VER_LIMIT="${VER_LIMIT:-2.34}"

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
  case $1 in
    -m|--max)
      MAX_ONLY_FLAG=1
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

BINARY="$1"

# Version comparison function in bash
vercomp() {
  if [[ $1 == "$2" ]]; then
    return 0
  fi
  local i ver1 ver2
  IFS="." read -ra ver1 <<<"$1"
  IFS="." read -ra ver2 <<<"$2"
  # fill empty fields in ver1 with zeros
  for ((i = ${#ver1[@]}; i < ${#ver2[@]}; i++)); do
    ver1[i]=0
  done
  for ((i = 0; i < ${#ver1[@]}; i++)); do
    if [[ -z ${ver2[i]} ]]; then
      # fill empty fields in ver2 with zeros
      ver2[i]=0
    fi
    if ((10#${ver1[i]} > 10#${ver2[i]})); then
      return 1
    fi
    if ((10#${ver1[i]} < 10#${ver2[i]})); then
      return 2
    fi
  done
  return 0
}

IFS="
"

# This sets VERS to be a list of all the GLIBC version numbers sorted alphabetically
# (or so it appears.)
VERS=$(objdump -T "$BINARY" | grep GLIBC | sed 's/.*GLIBC_\([.0-9]*\).*/\1/g' | sort -u)

MAX_VER="0.0"

for VER in $VERS; do
  vercomp "$VER" "$MAX_VER"
  COMP=$?
  if [[ $COMP -eq 1 ]]; then
    MAX_VER=${VER}
  fi

  vercomp "$VER" "$VER_LIMIT"
  COMP=$?
  if [[ $COMP -eq 1 ]]; then
    echo "Error! ${BINARY} requests GLIBC ${VER}, which is higher than target ${VER_LIMIT}"
    echo "Affected symbols:"
    objdump -T "$BINARY" | grep -F "GLIBC_${VER}"
    echo "Looking for symbols in libraries..."
    for LIBRARY in $(ldd "$BINARY" | cut -d ' ' -f 3); do
      echo "$LIBRARY"
      objdump -T "$LIBRARY" | grep -F "GLIBC_${VER}"
    done
    exit 27
  else
    if [ -z "$MAX_ONLY_FLAG" ]; then
      echo "Found version ${VER}"
    fi
  fi
done

if [ -n "$MAX_ONLY_FLAG" ]; then
  echo $MAX_VER
fi
