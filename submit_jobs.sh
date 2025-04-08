#!/bin/bash

# Function to display help message
display_help() {
    cat << EOF
Usage: $0 <accessions_file> <data_path> <window_size>

This script generates a SLURM job script and submits it for each accession.

Arguments:
  accessions_file File containing accessions for the jobs
  data_path       Path to the data directory on the host
  window_size     Window size for the FastIBS program

Example:  bash $0 accessions.txt ../FastIBSData/ 50000
EOF
}

# Check if the correct number of arguments are provided
if [ "$#" -ne 3 ]; then
    display_help
    exit 1
fi

# Assign arguments to variables
accessions_file=$1
data_path=$2
window_size=$3

# Check if the accessions file exists
if [ ! -f "$accessions_file" ]; then
    echo "Error: Accessions file '$accessions_file' does not exist."
    exit 1
fi

# Loop over each accession in the file
while IFS= read -r accession
do
    # Generate a SLURM job script
    job_script="job_${accession}.sh"
    cat << EOF > $job_script
#!/bin/bash
#SBATCH --exclusive
#SBATCH --job-name=FastIBS_${accession}
#SBATCH --output=FastIBS_${accession}.out
#SBATCH --error=FastIBS_${accession}.err
#SBATCH --time=24:00:00
#SBATCH --mem=256G
#SBATCH --cpus-per-task=50
bash run.sh ${accession} ${data_path} ${window_size}
EOF

    # Submit the job - uncomment below to submit
    #sbatch $job_script
done < "$accessions_file"
