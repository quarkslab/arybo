#!/bin/bash
# Script used to extract the part of boost needed

rm -rf tmp boost
mkdir tmp
bcp --unix-lines --boost=/usr/include type_traits/function_traits.hpp integer/static_log2.hpp iterator/iterator_facade.hpp tmp
mv tmp/boost .
rm -rf tmp
