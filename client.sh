#!/bin/bash

exec 3<>/dev/tcp/127.0.0.1/28772
echo "connected"
while read -r line
do  
    echo -n "$line" >&3
    #reponse=$(head -z <&3)
    reponse=$(head -z <&3)
    echo "Query executed :"
    echo "$reponse"
    
done

exec 3<&-
exec 3>&-