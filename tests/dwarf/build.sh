#!/bin/zsh

if [ ! -e "bin" ] ; then mkdir "bin" ;  fi

clang -g -I ../../src -o bin/test-dwarf main.c
