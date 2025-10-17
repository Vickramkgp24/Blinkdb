#!/bin/bash

# TCP Server details
HOST="127.0.0.1"
PORT=9002

# Define test parameters
requests=(10000 100000 1000000)
clients=(10 100 1000)

# Ensure the TCP server is running
if ! nc -z $HOST $PORT; then
    echo "Error: TCP server is not running on $HOST:$PORT"
    exit 1
fi

# Run benchmark for each combination of requests and clients
for req in "${requests[@]}"; do
    for cli in "${clients[@]}"; do
        echo "Running benchmark with $req requests and $cli clients..."
        result_file="result_${req}_${cli}.txt"
        redis-benchmark -h $HOST -p $PORT -t set,get -n $req -c $cli > "$result_file"
        echo "Results saved to $result_file"
    done
done

echo "Benchmarking completed!"
