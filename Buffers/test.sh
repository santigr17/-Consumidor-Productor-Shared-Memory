#!/bin/bash

# some older test, doesn't work and complains and I get this message on command line: "QApplication::qAppName: Please instantiate the QApplication object first"
# I also can't enter text after command executes
#echo "Hello World!"
#exec konsole --noclose -e cat ~/.aliases
NEW_UUID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 5 | head -n 1)
pro1wdir=$(pwd)
# for i in 1 2 3 4
# do
# done
# opens terminal but then I can't control terminal afterwards
# make sample
# sleep 2
xterm -hold -e "cd $pro1wdir && ./iniciador \"$NEW_UUID\" 4 " &
sleep 2
xterm -hold -e "cd $pro1wdir && ./productor \"$NEW_UUID\" "&
sleep 2
xterm -hold -e "cd $pro1wdir && ./consumidor \"$NEW_UUID\" \"automatico\" 3 "&
sleep 2
xterm -hold -e "cd $pro1wdir && ./consumidor \"$NEW_UUID\" \"manual\" "&


# didn't do anything
#exit 0

# didn't do anything except make me type exit an extra time where I executed my shell script
#$SHELL