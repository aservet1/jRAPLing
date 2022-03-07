#!/bin/bash

sudo -v

# runs through my sample Demo.java drivers to make sure the output is as expected

printf "\nhello w0rld\n\n";

runDriver=""

for driverArgs in \
	'ArchSpec' \
	'EnergyMonitor' \
	'AsyncEnergyMonitor'
do
	echo ~~~ $driverArgs ~~~
	sudo java -cp 'src/java/target/jRAPL-1.0.jar' jRAPL.Demo $driverArgs

	echo =============================================
done
