# FastIBS

**FastIBS** is a high-performance toolkit for computing identity-by-state (IBS) distances and related genomic analyses. Built with an optimized C++ backend, it supports containerized deployment through **Docker** and **Singularity**, ensuring easy setup and portability across environments, especially as most analyses of this type are commonly performed on HPC enviroments.

FastIBS leverages the **KMC API** to implement IBS distance computation in a manner closely aligned with [**IBSpy**](https://github.com/Uauy-Lab/IBSpy), which is currenly the de-facto workflow for this type of analysis. However, unlike IBSpy, which suffers from significant memory bottlenecks that hinder parallelism, FastIBS provides a highly efficient and scalable C++ implementation. Our intial benchmarks show that FastIBS can deliver over **100× speed-up** compared to the base IBSpy pipeline, depending on underlying hardware and level of parallelism.

FastIBS is built on top of [**KMC**](https://github.com/refresh-bio/KMC), a high-performance API for counting k-mers from FASTQ or FASTA files, including gzipped formats. The KMC API enables efficient access to k-mer databases, forming the computational backbone of FastIBS.

---

## 🚀 Installation Guide

### ✅ Requirements

- **Docker**
- **Singularity** (for generating `.sif` images)
- **Bash**

> ⚠️ Note: You do *not* need to install CMake or Ninja on your host system — they are included in the Docker image.

---

### 🔧 Build & Install

#### 1. Clone the repository

```bash
mkdir <project-folder>
cd <project-folder>
git clone https://github.com/githubcbrc/FastIBS.git .
```

### 2. (Optional) Edit Configuration

Edit the `init/config.sh` file if you want to customize the Docker image and container names:

```bash
IMAGE_NAME=fastibs_img        # Docker image name  
CONTAINER_NAME=fastibs_cont   # Docker container name
```

### 3. Run the Installer

Run the installation script:

```bash
bash install.sh
```

This script performs the following steps:

- Builds the Docker image
- Starts the Docker container and mounts your project directory
- Compiles the C++ binaries inside the container
- Saves the Docker image as a `.tar` archive
- Builds a Singularity `.sif` image from the `.tar` archive
- Removes the intermediate `.tar` file


## ⚙️ Project Structure

```
project-root/
│
├── init/
│   ├── config.sh           # Config variables (image/container names)
│   └── Dockerfile          # Docker image setup
│
├── scripts/
│   ├── build_img.sh        # Builds Docker image
│   └── start_cont.sh       # Starts Docker container
│
├── src/                    # C++ source code
│
├── build/                  # CMake build directory (auto-generated)
├── bin/                    # Compiled binaries (auto-generated)
├── compile.sh              # CMake + Ninja build script
└── install.sh              # Top-level installation script
```



## 📦 Output Binaries

After a successful build, the following executables will be available in `/project/bin`:

- `fastibs`
- `fastibsmapper`
- `KDBIntersect`


## 🐳 Using Docker

To manually enter the running container:

```bash
docker exec -it <container_name> bash
```

Or, using variables from the config:

```bash
source init/config.sh
docker exec -it ${CONTAINER_NAME} bash
```

## 🧪 Using Singularity

Once `fastibs.sif` is built, you can test FastIBS tools have been properly installed as follows:

```bash
singularity exec --bind .:/project fastibs.sif /project/bin/fastibs --help
singularity exec --bind .:/project fastibs.sif /project/bin/fastibsmapper --help
singularity exec --bind .:/project fastibs.sif /project/bin/KDBIntersect --help
```

If you want to use the tools do not forget to mount a data volume with a structure similar to the following:

```bash
FastIBSData/ # data volume root
├── FastIBS_runs # results folder (initially empty)
│   ├── ECOLI_v_ecoli_50000.tsv # fastibs result file
│   ├── ECOLI_v_ecoli_genome.txt # fastibs mapper result file
│   └── ECOLI_v_TA1675_50000.tsv
├── kmc_sets # KMC database (A path to your KMC KBs)
│   ├── BW_01002
│   │   ├── BW_01002.res.kmc_pre
│   │   └── BW_01002.res.kmc_suf
│   ├── ECOLI
│   │   ├── ecoli.res.kmc_pre
│   │   └── ecoli.res.kmc_suf
│   └── IG90747
│       ├── IG90747.res.kmc_pre
│       └── IG90747.res.kmc_suf
└── reference # reference database
    ├── ecoli_genome.fasta
    └── TA1675_genome.fasta
```

## 🧹 Clean Build

To remove all build artifacts and recompile from scratch:

```bash
rm -rf build bin
bash compile.sh
```

Or re-run the full installation pipeline:

```bash
bash install.sh
```


## 📄 Output Format: IBS Distance Results


Running the **fastibs** binary without options or with the `--help` option:
```bash
singularity exec --bind .:/project fastibs.sif /project/bin/fastibs --help
```


prints the following.
```bash
FastIBS - IBS Distance Calculator
----------------------------------
Usage:
  /project/bin/fastibs <sourcePath> <referencePath> <resultsFolder> <windowSize>

Arguments:
  <sourcePath>     Path to folder with KMC dataset
                   e.g., /mnt/data/kmc_sets/BW_01002

  <referencePath>  Path to folder with reference genomes (FASTA)
                   e.g., /mnt/data/reference

  <resultsFolder>  Path to folder for storing output results
                   e.g., /mnt/data/FastIBS_runs

  <windowSize>     Length of the sequence window for IBS calculation
                   (integer, e.g., 50000)

Notes:
  - All folders should be located on a mounted data volume.
  - Reference files can be gzip-compressed.

Output:
  A tab-delimited file summarizing IBS distance metrics for each window.
  Columns: seqname, start, end, total_kmers, observed_kmers, variations, kmer_distance
```

Provided a KMC database at `<sourcePath>` , **fastibs** computes IBS distance reports against all references in `<referencePath>`. 
The output of **fastibs** is a tab-delimited table with the following columns:

| **Column Name**     | **Description**                                                                 |
|---------------------|---------------------------------------------------------------------------------|
| `seqname`           | The sequence name (typically the accession or identifier of the reference genome segment). |
| `start`             | Start position (0-based) of the genomic window being analyzed.                  |
| `end`               | End position of the window (non-inclusive).                                     |
| `total_kmers`       | Total number of k-mers generated from the reference window. This is usually fixed and depends on the window size. |
| `observed_kmers`    | Number of k-mers found in the sample that match the reference set for that window. |
| `variations`        | Number of variant sites (positions where the reference and sample differ within the window). |
| `kmer_distance`     | Computed IBS distance metric, often reflecting the number of unique k-mers in the reference that are absent from the sample (or vice versa). |


## Mapping a KMC database to reference sequences

This tool computes a K-mer mapping for the given references, where each nucleotide position in the reference sequences is associated with a count of how many K-mers (of a fixed size, defined by the kmerSize of the KMC source) overlap that position and exist in the source KMC database.

A quick and easy k-mer mapping can be used to detect regions of high conservation/divergence from a given reference. Even though a KMC database does not retain positional information and only records k-mers and their frequencies from reads, the probability of false positives is small as long as sufficiently long k-mers are used (we use a default of 31 for kmerSize).


Running the **fastibsmapper** binary without options or with the `--help` option:
```bash
singularity exec --bind .:/project fastibs.sif /project/bin/fastibsmapper --help
```

prints the following.

```bash
FastIBS - Reference Mapping Tool
---------------------------------
Usage:
  /project/bin/fastibsmapper <sourcePath> <referencePath> <resultsFolder>

Arguments:
  <sourcePath>     Path to folder containing KMC database files
                   (e.g., /mnt/data/kmc_sets/<dataset_name>)

  <referencePath>  Path to folder containing reference genomes in FASTA format
                   (e.g., /mnt/data/reference)

  <resultsFolder>  Destination folder for writing mapping result files
                   (e.g., /mnt/data/FastIBS_runs)

Notes:
  - All input folders should reside on a mounted data volume.
  - The tool scans <referencePath> for .fasta files and processes them against the KMC base.
  - Output filenames follow the format: <KMC_prefix>_v_<reference_stem>.txt
  - Existing output files will be skipped.
  - Errors during processing are logged to log.txt.

Example:
  /project/bin/fastibsmapper /mnt/data/kmc_sets/sample1 /mnt/data/reference /mnt/data/FastIBS_runs

Output: This tool computes a K-mer mapping for the given references, where each nucleotide position in the reference sequences is associated with a count of how many K-mers (of a fixed size, defined by the kmerSize of the KMC source) overlap that position and exist in the source KMC database.
```

## Intersection Size Tool

```bash
K-mer Database Intersection Size Tool
----------------------------------
Usage:
  /project/bin/KDBIntersect <kDB1Path> <kDB2Path>

Arguments:
  <kDB1Path>     Path to the first KMC database directory
                 (e.g., /mnt/data/kmc_sets/db1)

  <kDB2Path>     Path to the second KMC database directory
                 (e.g., /mnt/data/kmc_sets/db2)

Description:
  This tool computes the intersection size between two KMC databases, i.e.,
  how many k-mers from the first database are present in the second.

Notes:
  - The databases must have the same k-mer size.
  - The tool compares the two KMC databases, choosing the smaller database
    for random access to optimize performance.
  - The intersection size is output as an integer, which represents the
    number of common k-mers between the two databases.

Example:
  /project/bin/KDBIntersect /mnt/data/kmc_sets/db1 /mnt/data/kmc_sets/db2
```




