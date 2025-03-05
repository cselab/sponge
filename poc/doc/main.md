Sergey/ Prof. Koumoutsakos - Section about Sponge CFD simulations.

Inspired by the recent results [Falcucci, Giacomo, et al. "Extreme
flow simulations reveal skeletal adaptations of deep-sea sponges."
Nature 595.7868 (2021): 537-541.], we are integrating direct numerical
simulation CFD (computational fluid dynamics) into the project of the
deep-sea glass sponge. The realistic living conditions correspond to
the range of Reynolds numbers between 100 and 2000. Our preliminary
estimations lead to the conclusion that such simulations are feasible
but require state-of-the-art high-performance computing techniques. To
optimize the setup, we use the Basilisk flow solver; however, we plan
to switch to our in-house open-source code, CubismAMR.

CubismAMR is an adaptive version of the Cubism library [Rossinelli,
D., Hejazialhosseini, B., Hadjidoukas, P., Bekas, C., Curioni, A.,
Bertsch, A., ... & Koumoutsakos, P. (2013, November). 11 PFLOP/s
simulations of cloud cavitation collapse. In Proceedings of the
International Conference on High-Performance Computing, Networking,
Storage, and Analysis (pp. 1-13).]. It partitions the simulation
domain into cubic blocks of uniform resolution that are distributed to
multiple compute nodes for cache-optimized parallelism [30]. CubismAMR
organizes these blocks in an octree data structure (for
three-dimensional simulations) or a quadtree data structure (for
two-dimensional simulations), allowing for Adaptive Mesh Refinement in
different regions. CubismAMR is a C++ library for distributed
simulations with block-structured grids and Adaptive Mesh
Refinement. It proposes a numerical method to solve the incompressible
Navier-Stokes equations and introduces a novel approach for solving
the pressure Poisson equation on an adaptively refined
grid. Validation and verification results for the method have been
conducted, specifically for the flow past an impulsively started
cylinder [Chatzimanolakis, M., Weber, P., Wermelinger, F., &
Koumoutsakos, P. (2022). CubismAMR--A C++ library for Distributed
Block-Structured Adaptive Mesh Refinement. arXiv preprint
arXiv:2206.07345.].

Basilisk, a successor of Gerris, is an open-source CFD solver built in
the C programming language. It solves partial differential equations
based on the Finite Volume Method (FVM) discretization
scheme. Basilisk combines adaptive refinement (Adaptive Mesh
Refinement (AMR)) and the Multigrid iterative method, making it more
feasible for simulating two-phase flows [Fuster, D., & Popinet,
S. (2018). Investigation of an implicit solver for the simulation of
bubble oscillations using Basilisk.].

We wrote a C program to simulating fluid flow around an obstacle of a
custom shapes custom shapes provided as STL files. The program uses
MPI for parallelism, and `vtkHyperTreeGrid` file format for output.
To confirm our setup's accuracy, we simulated the flow around a
circular cylinder. Our aim was to ensure that the drag coefficients we
calculated closely matched those reported in the
literature. Additionally, we conducted a grid convergence study and
fine-tuned the solver parameters to enhance the performance specific
to the simulation. The initial performance analysis of the code was
also carried out to assess its scaling capabilities. We also wrote a
Python script which uses the ParaView library to create a
visualization of HyperTreeGrid data along with associated 3D
geometries. The visualization script also runs in parallel.

Sergey/ Prof. Koumoutsakos - Section about Sponge CFD simulations.

We are currently developing a multi-object, mixed integer-continuous
algorithm based on an extended version of CMA-ES. Additionally, we've
established a system that orchestrates the efficient execution of
parallel Computational Fluid Dynamics (CFD) simulations on a remote
cluster. To achieve this, we leverage the FirecREST API, a tool
developed by the Swiss National Supercomputing Centre (CSCS) at ETH
Zurich.

FirecREST offers a REST API, defining a set of functions accessible
through the HTTP/REST protocol architecture. This interface enables
developers to interact with various services. Calls made to the REST
API are processed by a services gateway, which then translates these
requests into the appropriate infrastructure demands. Notable services
provided by FirecREST include authentication and authorization, system
status monitoring, file system access, data movement, accounting
information, and more.
