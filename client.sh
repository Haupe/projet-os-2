#!/bin/bash

exec 3<>/dev/tcp/127.0.0.1/28772
echo "connected"
while true
do
    read -r line
    #if [$line == ""]; then
    #    break
    #fi
    echo "$line" >&3
    read -r reponse <&3
    echo "Query executed"
    echo "Réponse : $reponse"
done

exec 3<&-
exec 3>&-