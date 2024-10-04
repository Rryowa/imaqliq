#!/bin/bash

# Load environment variables from .env file
if [ -f .env ]; then
    set -a
    source .env
    set +a
else
    echo ".env file not found!"
    exit 1
fi

# Function to wait for server to start
wait_for_server() {
    echo "Waiting for server to start..."
    until grep -q "Server is listening on port" "${OUT_LOG}"; do
        sleep 1
    done
    echo "Server started."
}

# Function to send SIGHUP
send_sighup() {
    echo "Sending SIGHUP to ${SERVICE}..."
    sudo supervisorctl signal SIGHUP ${SERVICE}
}

# Function to send SIGTERM
send_sigterm() {
    echo "Sending SIGTERM to ${SERVICE}..."
    sudo supervisorctl signal SIGTERM ${SERVICE}
}

# Function to monitor logs
monitor_logs() {
    echo "Monitoring logs..."
    tail -n 50 -f "${OUT_LOG}" "${ERR_LOG}" &
    TAIL_PID=$!
}

# Start monitoring logs
monitor_logs

# Start the server
make all

# Wait for the server to initialize
wait_for_server

# Send SIGHUP signal
send_sighup

# Wait a moment to observe server behavior
sleep 2

# Send SIGTERM signal to shut down the server
send_sigterm

# Allow some time for graceful shutdown
sleep 2

# Stop monitoring logs
kill $TAIL_PID

echo "Signal handling test completed."