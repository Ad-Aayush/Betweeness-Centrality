all: Coarse CoarseOmp FineOmp FineThreadPool MediumOmp MediumThreadPool MediumChunk Sequential

Coarse: coarse.cpp
	g++ -o coarse coarse.cpp -fopenmp

CoarseOmp: coarseOmp.cpp
	g++ -o coarseomp coarseOmp.cpp -fopenmp

FineOmp: fineOmp.cpp
	g++ -o fineomp fineOmp.cpp -fopenmp

FineThreadPool: fineThreadPool.cpp
	g++ -o finethreadpool fineThreadPool.cpp -fopenmp

MediumOmp: mediumOmp.cpp
	g++ -o mediumomp mediumOmp.cpp -fopenmp

MediumThreadPool: mediumThreadPool.cpp
	g++ -o mediumthreadpool mediumThreadPool.cpp -fopenmp

MediumChunk: mediumChunk.cpp
	g++ -o mediumchunk mediumChunk.cpp -fopenmp

Sequential: sequential.cpp
	g++ -o sequential sequential.cpp -fopenmp

clean:
	rm coarse coarseomp fineomp finethreadpool mediumomp mediumthreadpool mediumchunk sequential
