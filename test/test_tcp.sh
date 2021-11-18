#! /bin/bash

../tcp_example/tcp_server > /dev/null &
echo $(../tcp_example/tcp_client < ./src/testTcp.dat) > tcp_client_output.dat
DIFF=$(diff tcp_client_output.dat ./src/tcp_client_output.dat)
killall tcp_server

if [ "$DIFF" != "" ]
then
    echo -e "\e[31mTEST: TCP FAILED!\e[0m"
    echo $DIFF > errorlog.log
    exit
fi

echo -e "\e[32mTEST: TCP PASSED!\e[0m"