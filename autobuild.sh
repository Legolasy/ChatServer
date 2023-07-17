#!/bin/bash
set -x

rm -rf `pwd`/build/*
cd `pwd`/ &&
   cmake . &&
   make
