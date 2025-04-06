#!/bin/bash

# Read config values
source config.sh

# Check if data directory argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <data_dir>"
    exit 1
fi

data_dir=$1

echo "##############################"
echo "Starting container ${CONTAINER_NAME}"

# Check if the container exists and remove it if it does
if docker container ls -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container ${CONTAINER_NAME} exists, removing..."
    docker container stop ${CONTAINER_NAME} >/dev/null 2>&1
    docker container rm -f ${CONTAINER_NAME} >/dev/null 2>&1
    echo "Container ${CONTAINER_NAME} removed!"
fi

# Get the absolute path of the project directory
project_dir=$(realpath ..)

echo "project at ${project_dir}, data volume at ${data_dir}"

# Run a new container instance
# You may use --user $(id -u):$(id -g) to run under user id instead of root
container_id=$(docker run -d --name ${CONTAINER_NAME} -it -v ${project_dir}:/project -v ${data_dir}:/project/data -p 8087:8087 ${IMAGE_NAME})
echo "New container instance ${CONTAINER_NAME} is running with ID: ${container_id}"

echo "##############################"
