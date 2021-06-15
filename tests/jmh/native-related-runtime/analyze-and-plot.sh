#!/bin/bash

if [ $1 != 'JavaSide' ] && [ $1 != 'CSide' ] && [ $1 != 'readMSR' ]
then
	echo "invalid option: $1"
	echo "must be JavaSide, CSide, or readMSR"
	exit 2
fi

python3 make_histogram.py $1_*_histogram.data
