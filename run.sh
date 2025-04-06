#!/bin/bash

# Function to display help message
function display_help {
    echo "Usage: $0 <KMC_dir> <data_dir> <window_size>"
    echo
    echo "This script runs a Singularity container with specified directories and parameters."
    echo
    echo "Arguments:"
    echo "  KMC_accession   Accession of the KMC set (folder name)"
    echo "  data_dir        Path to the data directory on the host (where kmc_sets and reference folders reside)"
    echo "  window_size     Window size for the FastIBS program"
    echo "  example:        bash $0 ECOLI FastIBSData 50000"
}

# Check if the correct number of arguments are provided
if [ "$#" -ne 3 ]; then
    display_help
    exit 1
fi

# Assign arguments to variables
KMC_dir=$1
data_dir=$2
window_size=$3

# Fixed values
project_dir=$(pwd)
singularity_image="fastibs.sif"
source_path="/project/data/kmc_sets/${KMC_dir}"
reference_path="/project/data/reference"
results_folder="/project/data/FastIBS_runs"

# Check if the directories and the Singularity image exist
if [ ! -d "$data_dir" ]; then
    echo "Error: Data directory '$data_dir' does not exist."
    exit 1
fi

if [ ! -f "$singularity_image" ]; then
    echo "Error: Singularity image '$singularity_image' does not exist."
    exit 1
fi

# Run the Singularity container with the bound directories and the new parameters
singularity exec --bind ${project_dir}:/project,${data_dir}:/project/data ${singularity_image} /project/bin/fastibs ${source_path} ${reference_path} ${results_folder} ${window_size}
