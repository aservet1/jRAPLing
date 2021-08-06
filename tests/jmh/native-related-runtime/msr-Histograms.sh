#!/bin/bash

if [ -z $1 ]; then
	echo "usage: $0 output_dir"
	exit 1
fi
output_dir=$1
inputfiles=""
echo $inputfiles
./scripts/make_histograms_MSRs.py \
	results/{alejtpad-e15,vaporeon}/readMSR_{DRAM,CORE,PKG}_histogram.data \
	$output_dir
