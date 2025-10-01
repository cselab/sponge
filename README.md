# sponge

This repository contains the source code and data for a paper submission. The code performs CFD simulations of flow past a cylinder or sphere using the [Basilisk](http://basilisk.fr) framework.

## Main Code

The main simulation code is located in the `/cylinder` directory. It is built upon the Basilisk CFD solver.

## Directory Structure

- **/cylinder**: Contains the main simulation source code.
  - `cylinder.c`: The main program for the fluid simulation.
  - `centered.h`: Incompressible Navier-Stokes solver implementation.
  - `Makefile`: Used to build the simulation executable.
  - `README.md`: Provides instructions for installing dependencies and building the code.
  - **/deploy**: Contains scripts and pre-processed files for running simulations on different systems.

- **/dump**: Contains utilities for handling simulation data dumps.
  - `dump2xdmf.c`: Converts simulation dump files to XDMF format for visualization.
  - `stl2dump.c`: Generates a Basilisk dump file from an STL geometry file to initialize a simulation domain.

- **/geom**: Contains various scripts for geometry processing.

- **/stl**: Contains utilities for creating and manipulating STL files (e.g., `cylinder.py` to generate a cylinder geometry).

## How to Run

1.  **Install Dependencies:** The main dependency is the Basilisk CFD framework. Follow the instructions in `/cylinder/README.md` to install it.

2.  **Build:** Navigate to the `/cylinder` directory and use the `Makefile` to build the executable:
    ```bash
    cd cylinder
    make
    ```

3.  **Run Simulation:** The scripts in the `/cylinder/deploy` directory provide examples of how to run the simulations with different parameters. For example, you can use `run.sh` to start a simulation from a pre-generated dump file.
    ```bash
    cd cylinder/deploy
    ./run.sh
    ```