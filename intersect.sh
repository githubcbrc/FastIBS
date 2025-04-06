#!/bin/bash

# Function to display help message
function display_help {
    echo "Usage: $0 <KMCDB1Path> <KMCDB2Path>"
    echo
    echo "This script runs a Singularity container to compute the intersection count."
    echo "  example:        bash $0 data/ECOLI data/G123"
}

# Check if the correct number of arguments are provided
if [ "$#" -ne 2 ]; then
    display_help
    exit 1
fi

# Assign arguments to variables
KMCDB1=$1
KMCDB2=$2

# Fixed values
project_dir=$(pwd)
singularity_image="fastibs.sif"

if [ ! -f "$singularity_image" ]; then
    echo "Error: Singularity image '$singularity_image' does not exist."
    exit 1
fi

# Run the Singularity container with the bound directories and the specified DB paths
singularity exec --bind ${project_dir}:/project ${singularity_image} /project/bin/KDBIntersect ${KMCDB2} ${KMCDB1}
