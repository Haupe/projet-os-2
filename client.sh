#!/bin/bash

exec 3<>/dev/tcp/10.0.2.15/28772

while true
do
    read -r line
    if ["$line" == "exit"]; then
        break
    fi
    echo "$line" >&3
    read -r reponse <&3
    echo "Réponse : $reponse"
done

exec 3<&-
exec 3>&-