# Parallelized-Bucket-Sort
Parallelizing Bucket Sort with MPI

Done as an assignment for 491: Parallel Processing at MSU. The code in bucket_sort_skeleton.c was provided to us, the v1, v2, & v3 files are my solutions. These were run on MSU's HPCC to compare run-times.

v1:
1) generate: create the array at the root process
2) bin: root process determines buckets -- which data belongs on which processor
3) distribute: root sends buckets to appropriate processes
4) local sort: each process sorts the data locally using qsort()
5) gather: results are gathered at the root process

v2:
1) generate: create N/P elements randomly on each process
2) bin: each process determines buckets - which of their data belongs on which
processor
3) distribute: each process sends buckets to appropriate processes
4) local sort: each process sorts the data locally using qsort()
5) gather: results are gathered at the root process

v2.1: Different data distribution: random distribution to create unbalanced load on each processor

v3: Pivots with sampling to deal with imbalanced loads:
1) Each process selects S/P elements randomly within its part of array A
2) S samples are gathered at the root process
3) root process sorts samples locally using qsort()
4) root selects and broadcasts pivots = [S_sorted[S/P], S_sorted[2S/P], S_sorted[3S/P] ...]
