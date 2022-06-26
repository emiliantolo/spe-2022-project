
# SPE 2022 project

## Structure

- **ns-3-dev/scratch/**
  - **project_*.cc** - simulation source files
- **project_*/**
  - **sem_simulations_py** - simulations runs script
  - **count.sh** - pcap reading and retransmission count
  - **retry_count.sh** - execute counts for every run
  - **stats.py** - generate stats from runs
  - **throughput.png** - throughput graphs
  - **retry.png** - retransmission graphs
  - **run.sh** - script for running simulation and obtain stats
- **run_all.sh** - script for running all independent runs for every configuration and generate statistics

## Run

Tested with ns3 3.33-dev, Ubuntu 20.04.4 LTS WSL2

### Cloning

Need to add ns-3-dev release in *ns-3-dev* directory

    git clone https://github.com/emiliantolo/spe-2022-project.git

### Run simulation

    cd ns-3-dev
    ./waf --run "scratch/project_hidden.cc --rtscts=true --verbose=false --datarate=10"

### Run campaign

Run with *--clean* option for cleaning generated independent runs

    cd project_hidden
    chmod +x run.sh
    ./run.sh

### Run all campaigns and clean

    chmod +x run_all.sh
    ./run_all.sh
