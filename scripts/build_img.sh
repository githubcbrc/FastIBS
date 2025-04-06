#!/bin/bash

# Read config values
source config.sh

# Print section separator
echo "##############################"

# Build Docker image
echo "Building Docker image ..."
docker build -t ${IMAGE_NAME} . #--no-cache

# Print section separator
echo "##############################"

