#!/bin/bash

# Read config values
source init/config.sh

# Check if data directory argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <data_dir>"
    exit 1
fi

data_dir=$1

# Navigate to the root directory of the project
cd "$(dirname "${BASH_SOURCE[0]}")"

# Build Docker image and start container
(cd init && bash ../scripts/build_img.sh
bash ../scripts/start_cont.sh ${data_dir})

# Compile the project
docker exec -it ${CONTAINER_NAME} bash -c "cd /project && bash compile.sh"

# Build docker image tar
echo "  ** saving docker image into a tar file **  "
docker save -o ${IMAGE_NAME}.tar ${IMAGE_NAME}:latest

# Build Singulairty image
singularity build ${IMAGE_NAME}.sif docker-archive://${IMAGE_NAME}.tar




