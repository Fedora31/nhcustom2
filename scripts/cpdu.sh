#!/bin/dash

# This script replaces updates of entries in file "new" with
# updates and dates from the entries in file "old". Entries
# not found in both entries are prepended with the string
# "REMAINDER ".
# 
# It was created because nhschema cannot find the name of
# the updates as this information doesn't appear in the item
# schema, and the dates are frequently a couple of days off.
# So this allows to reuse the dates and updates from an older
# database.

while read line; do

	hat=$(echo "$line" | awk -F ";" '{print $1}')

	if $(grep -qi "^$hat;" old) ;then

		ohat=$(grep -i "^$hat;" old)
		odate=$(echo "$ohat" | awk -F ';' '{print $4}')
		oupdate=$(echo "$ohat" | awk -F ';' '{print $5}')

		echo "$line" | awk -F ';' '{printf("%s;%s;%s;%s;%s;%s\n", $1, $2, $3, "'"$odate"'", "'"$oupdate"'", $6)}'
	else
		echo "REMAINDER $line"
	fi

done < new

