#!/bin/bash
#enter input encoding here
FROM_ENCODING="value_here"
#output encoding(UTF-8)
TO_ENCODING="UTF-8"
#convert
CONVERT=" iconv   -t   $TO_ENCODING"
#loop to convert multiple files 
for  file  in  *.h; do
     $CONVERT   "$file"   -o  "${file}"
done
exit 0