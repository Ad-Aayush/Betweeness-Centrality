# Course Project: Parallel & Concurrent Programming

<strong>Authors:</strong> \
Aayush Adlakha (CS22BTECH11001)\
Jash Jhatakia (CS22BTECH11028)

## Directory Structure and File Explanation:
- `Graphs/` : Contains the test graphs 
- `coarse.cpp` : Contains the code for the coarse-grained parallel implementation of the Brandes Algorithm. The outer loop of the Brandes Algorithm is given to each thread.
- `coarseOmp.cpp` : Contains the code for the coarse-grained parallel implementation of the Brandes Algorithm using OpenMP.
- `fineOmp.cpp` : Contains the code for the fine-grained parallel implementation of the Brandes Algorithm using OpenMP. All the neighbours of a BFS node are given to different threads. While performing BFS, the task of exploring the neighbors of
a vertex is parallelized. Each thread does the task of exploring the set of neighbors of the vertex.
- `fineThreadPool.cpp` : Contains the code for the fine-grained parallel implementation of the Brandes Algorithm using ThreadPool. 
- `mediumChunk.cpp` : Contains the code for the medium-grained parallel implementation of the Brandes Algorithm using OpenMP. Here the exploration of a vertex in one level is parallelized and each thread is given a chunk of vertices to explore.
- `mediumOmp.cpp` : Contains the code for the medium-grained parallel implementation of the Brandes Algorithm using OpenMP. 
- `mediumThreadPool.cpp` : Contains the code for the medium-grained parallel implementation of the Brandes Algorithm using ThreadPool.
- `sequential.cpp` : Contains the code for the sequential implementation of the Brandes Algorithm.
- `writeRandomGraphTofile.cpp` : Contains the code to generate a random graph with a given number of vertices and write it to a file.
- `Makefile` : Contains the commands to compile the code.
- `README.md` : Contains the instructions to run the code and basic details about the files.
- `report.pdf` : Contains the report of the project.
- `output.txt` : Contains the output of the code for the test graphs.

## Creating test graphs
To create test graphs with `<number_of_vertices>` nodes, run the following command:
```bash
g++ writeRandomGraphTofile.cpp -o writeRandomGraphTofile
./writeRandomGraphTofile <number_of_vertices>
``` 
This will create a file `graph-<number_of_vertices>.txt` with the test graph, in the `Graphs/` directory.

## Compiling the code
- To compile all the file, run the following command:
```bash
make all
```
- To compile any the `coarse.cpp` file, run the following command:
```bash
make Coarse
```
- To compile any the `coarseOmp.cpp` file, run the following command:
```bash
make CoarseOmp
```
<!-- - To compile any the `fineOmp.cpp` file, run the following command:
```bash
make FineOmp
```
- To compile any the `fineThreadPool.cpp` file, run the following command:
```bash
make FineThreadPool
```
- To compile any the `mediumOmp.cpp` file, run the following command:
```bash
make MediumOmp
```
- To compile any the `mediumThreadPool.cpp` file, run the following command:
```bash
make MediumThreadPool
```
- To compile any the `mediumChunk.cpp` file, run the following command:
```bash
make MediumChunk
```
- To compile any the `sequential.cpp` file, run the following command:
```bash
make Sequential
``` -->
- Similarly, use the `make` command to compile any other file.

## Running the code
After compiling the code, run the following command to run the code:
```bash
./<executable> <graph_file> <number_of_threads>
```

## Output
The output of the code will be stored in the `output.txt` file.
