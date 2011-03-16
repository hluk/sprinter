#!/bin/sh
FILE="$PWD"

while [ -d "$FILE" ]
do
    FILE=$FILE/`(echo ..; /bin/ls -p "$FILE") |
        sprinter -t"Edit file" -l"EDIT:" -g 400,300` ||
            exit 1
done

gvim "$FILE"|| gedit "$FILE"

