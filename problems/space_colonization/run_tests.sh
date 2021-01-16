#!/bin/bash

test_details=("1,0,0", "1,0,0", "2,0,0", "10,0,0", "10000,0,0", "10001,0,0", "10003,11,12")

for (( i = 1 ; i <= ${#test_details[@]} ; i++ ))
do

    # Get details for this test
    inc_i=$i-1
    IFS="," read -r -a data <<< "${test_details[$inc_i]}"
    iterations="${data[0]}"
    source_planet_x="${data[1]}"
    source_planet_y="${data[2]}"

    for process_count in 1 2 4
    do

        # Running the test with a different number of processes
        command="mpirun -np "$process_count" executables/homework data/in/test_"$i".txt \
            data/prod/"$process_count"/test_"$i".txt "$iterations" "$source_planet_x" "$source_planet_y
        echo "[+] Running command:" $command
        eval $command

        # Check if files are different
        ./executables/compare data/out/test_$i.txt  data/prod/$process_count/test_$i.txt 

    done

done