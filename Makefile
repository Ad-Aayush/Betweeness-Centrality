all: coarse coarseOmp fineOmp fineThreadPool mediumOmp mediumThreadPool mediumChunk sequential

coarse: Src_Prjt-CS22BTECH11001-coarse.cpp
	g++ -o coarse Src_Prjt-CS22BTECH11001-coarse.cpp -fopenmp

coarseOmp: Src_Prjt-CS22BTECH11001-coarseOmp.cpp
	g++ -o coarseOmp Src_Prjt-CS22BTECH11001-coarseOmp.cpp -fopenmp

fineOmp: Src_Prjt-CS22BTECH11001-fineOmp.cpp
	g++ -o fineOmp Src_Prjt-CS22BTECH11001-fineOmp.cpp -fopenmp

fineThreadPool: Src_Prjt-CS22BTECH11001-fineThreadPool.cpp
	g++ -o fineThreadPool Src_Prjt-CS22BTECH11001-fineThreadPool.cpp -fopenmp

mediumOmp: Src_Prjt-CS22BTECH11001-mediumOmp.cpp
	g++ -o mediumOmp Src_Prjt-CS22BTECH11001-mediumOmp.cpp -fopenmp

mediumThreadPool: Src_Prjt-CS22BTECH11001-mediumThreadPool.cpp
	g++ -o mediumThreadPool Src_Prjt-CS22BTECH11001-mediumThreadPool.cpp -fopenmp

mediumChunk: Src_Prjt-CS22BTECH11001-mediumChunk.cpp
	g++ -o mediumChunk Src_Prjt-CS22BTECH11001-mediumChunk.cpp -fopenmp

sequential: Src_Prjt-CS22BTECH11001-sequential.cpp
	g++ -o sequential Src_Prjt-CS22BTECH11001-sequential.cpp -fopenmp

clean:
	rm -f coarse coarseOmp fineOmp fineThreadPool mediumOmp mediumThreadPool mediumChunk sequential
