#!/bin/sh

INPUT="./input/"
OUTPUT="./output/"


echo "-- nhcustom2 --"

if [ -d $OUTPUT ]; then
	rm -r $OUTPUT
fi

mkdir $OUTPUT

echo "reading the configuration file..."

./nhcustom2 > ./paths.txt

echo "copying files..."
while read res; do

	pd=$(dirname "$res")

	mkdir -p "$OUTPUT$pd"
	cp -r "$INPUT$res" "$OUTPUT$res"

done < ./paths.txt
rm ./paths.txt

echo "done."
