# Parallel Computing Learning Repository

This repository covers various parallel programming models, including Pthreads, OpenMP, CUDA and MPI.

## Table of Contents

1. [Pthreads](#pthreads)
2. [OpenMP](#openmp)
3. [CUDA](#cuda)
4. [MPI](#mpi)

## Pthreads

Pthreads, or POSIX threads, is a threading library for parallel programming in C/C++. The following code demonstrates a simple example using Pthreads.

### How to Run

```bash
gcc -o pthreads_example pthreads_example.c -lpthread
./pthreads_example
```

## OpenMP

OpenMP is a widely used API for parallel programming in C, C++, and Fortran. The following code illustrates a basic OpenMP example.

### How to Run

```bash
gcc -o openmp_example openmp_example.c -fopenmp
./openmp_example
```

## CUDA

CUDA is a parallel computing platform and application programming interface model created by NVIDIA. The following code showcases a basic CUDA program.

### How to Run

```bash
nvcc -o cuda_example cuda_example.cu
./cuda_example
```

## MPI

MPI, or Message Passing Interface, is a standard for parallel programming in distributed memory systems. The provided code demonstrates a simple MPI example.

### How to Run

```bash
mpicc -o mpi_example mpi_example.c
mpirun -np 4 ./mpi_example
```

Feel free to explore and modify the provided code examples to enhance your understanding of parallel computing. Happy learning!