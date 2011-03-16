#!/bin/sh
find `echo $PATH | tr : ' '` \! -type d -executable -printf '%f\n' |
    sprinter -t"RUN" -l"RUN:" -o -m -z 96,16 -g 200 |
        sh

