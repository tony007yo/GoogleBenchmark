#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <benchmark/benchmark.h>

#define RUN_ORIG
#define RUN_OPT

const int EX12_LEN = 2e7;
const int EX12_NTIMES = 10;

const int EX3_LEN = 1e6;
const int EX3_NTIMES = 20;
const int S3_DATA_LEN = 5;

const int EX6_LEN = 5e3;
const int EX6_NTIMES = 10;


int dummy(int n) {return n;}


void measure_time_begin(struct timespec *start)
{
	clock_gettime(CLOCK_MONOTONIC, start);
}


double measure_time_end(struct timespec *start, struct timespec *end)
{
	clock_gettime(CLOCK_MONOTONIC, end);
	double tdiff = (end->tv_sec - start->tv_sec)
					+ 1e-9*(end->tv_nsec - start->tv_nsec);
					printf("%lf\n", tdiff);
	return tdiff;
}

#define alloc_buf(buf, type, len) \
	buf = (type*) malloc(sizeof(type) * len); \
	if (buf == NULL) { \
		fprintf(stderr, "malloc() failed for %s: %s\n", #buf, strerror(errno)); \
		exit(1); \
	} \
	
	
struct s1 {
	int a;
	int b;
	int c;
	int d;
};


struct s1_hot {
	int a;
	int b;
};


struct s1_cold {
	int c;
	int d;
};

void example1_peeling_orig(struct s1 *buf1_orig, int len, int ntimes)
{
	for (int j = 0; j < ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf1_orig[i].a = buf1_orig[i].b;
		}
}

void example1_peeling_opt(struct s1_hot *buf1_hot, int len, int ntimes)
{
	for (int j = 0; j <ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf1_hot[i].a = buf1_hot[i].b;
		}
}

void Example1PeelingOrig(benchmark::State& state) {
	struct s1 *buf1_orig = NULL;
	alloc_buf(buf1_orig, struct s1, state.range(0));

    for (auto _ : state) {
		example1_peeling_orig(buf1_orig, state.range(0), state.range(1));
    }

	free(buf1_orig);
}

BENCHMARK(Example1PeelingOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example1PeelingOpt(benchmark::State& state) {
	struct s1_hot *buf1_hot = NULL;
	alloc_buf(buf1_hot, struct s1_hot, state.range(0));

    for (auto _ : state) {
		example1_peeling_opt(buf1_hot, state.range(0), state.range(1));
    }

	free(buf1_hot);
}

BENCHMARK(Example1PeelingOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example1_peeling(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 1 (peeling)\n");
#ifdef RUN_ORIG
    struct s1 *buf1_orig = NULL;
	alloc_buf(buf1_orig, struct s1, len);
	
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf1_orig[i].a = buf1_orig[i].b;
		}
		
	printf("Before:\t");
	double orig_time = measure_time_end(&start, &end);
	free(buf1_orig);
	
#endif

#ifdef RUN_OPT
	struct s1_hot *buf1_hot = NULL;;
	alloc_buf(buf1_hot, struct s1_hot, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j <ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf1_hot[i].a = buf1_hot[i].b;
		}
		
#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	free(buf1_hot);
#endif
}
*/

struct s2 {
	int a;	// hot field
	int b;	// hot field
	int c;	// cold field
	int d;	// cold field
	int e;	// cold field
	int f;	// cold field
	struct s2 *p1;	//pointer to self
};


struct s2_hot {
	int a;	// hot field
	int b;	// hot field
	struct s2_hot *p1;	//pointer to self
	struct s2_cold *p2;	//pointer to cold fiels
};


struct s2_cold {
	int c;	// cold field
	int d;	// cold field
	int e;	// cold field
	int f;	// cold field
};

void example2_splitting_orig(int len, int ntimes)
{
	struct s2 *buf2_orig = NULL;
	alloc_buf(buf2_orig, struct s2, len);
	
	for (int j = 0; j <ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf2_orig[i].a = buf2_orig[i].b;
		}
	
	free(buf2_orig);
}

void example2_splitting_opt(int len, int ntimes)
{
	struct s2_hot *buf2_hot = NULL;
	alloc_buf(buf2_hot, struct s2_hot, len);
	
	for (int j = 0; j < ntimes; j++)
		for(int i = 0; i < len; i++) {
			buf2_hot[i].a = buf2_hot[i].b;
		}

	free(buf2_hot);
}

void Example2SplittingOrig(benchmark::State& state) {
    for (auto _ : state) {
		example2_splitting_orig(state.range(0), state.range(1));
    }
}

BENCHMARK(Example2SplittingOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example2SplittingOpt(benchmark::State& state) {
    for (auto _ : state) {
		example2_splitting_opt(state.range(0), state.range(1));
    }
}

BENCHMARK(Example2SplittingOpt)->ArgPair(EX12_LEN, EX12_NTIMES);


/*
void example2_splitting(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 2 (splitting)\n");
#ifdef RUN_ORIG
	struct s2 *buf2_orig = NULL;
	alloc_buf(buf2_orig, struct s2, len);
	
	measure_time_begin(&start);
	
	for (int j = 0; j <ntimes; j++)
		for (int i = 0; i < len; i++) {
			buf2_orig[i].a = buf2_orig[i].b;
		}
	
	printf("Before:\t");
	double orig_time = measure_time_end(&start, &end);
	free(buf2_orig);
#endif

#ifdef RUN_OPT
	struct s2_hot *buf2_hot = NULL;
	alloc_buf(buf2_hot, struct s2_hot, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++)
		for(int i = 0; i < len; i++) {
			buf2_hot[i].a = buf2_hot[i].b;
		}
#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	free(buf2_hot);
#endif
}*/

struct s3 {
	int a;	//4 bytes
	int b;	//4 bytes
	int data2[S3_DATA_LEN];	//N*4 bytes
	int data1[S3_DATA_LEN];	//N*4 bytes
};


struct s3_opt {
	int a;	//4 bytes
	int b;	//4 bytes
	int data1[S3_DATA_LEN];	//N*4 bytes
	int data2[S3_DATA_LEN];	//N*4 bytes
};


void process(int *data)
{
	int sum = 0;
	
	for (int i = 0; i < S3_DATA_LEN; ++i) {
		sum += data[i];
	}
	dummy(sum);
}

void example3_field_reordering_orig(int len, int ntimes)
{
	struct s3 *buf3_orig = NULL;
	alloc_buf(buf3_orig, struct s3, len);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf3_orig[i].a = buf3_orig[i].b;
			process(buf3_orig[i].data1);
		}
	
		for (int i = 0; i < len; i++) {
			process(buf3_orig[i].data1);
			process(buf3_orig[i].data2);
		}
	}

	free(buf3_orig);
}

void example3_field_reordering_opt(int len, int ntimes)
{
	struct s3_opt *buf3_opt = NULL;
	alloc_buf(buf3_opt, struct s3_opt, len);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf3_opt[i].a = buf3_opt[i].b;
			process(buf3_opt[i].data1);
		}
	
		for (int i = 0; i< len; i++) {
			process(buf3_opt[i].data1);
			process(buf3_opt[i].data2);
		}
	}

	free(buf3_opt);
}

void Example3ReorderingOrig(benchmark::State& state) {
    for (auto _ : state) {
		example3_field_reordering_orig(state.range(0), state.range(1));
    }
}

BENCHMARK(Example3ReorderingOrig)->ArgPair(EX3_LEN, EX3_NTIMES);

void Example3ReorderingOpt(benchmark::State& state) {
    for (auto _ : state) {
		example3_field_reordering_opt(state.range(0), state.range(1));
    }
}

BENCHMARK(Example3ReorderingOpt)->ArgPair(EX3_LEN, EX3_NTIMES);

/*
void example3_field_reordering(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 3 (field reordering)\n");
#ifdef RUN_ORIG
	struct s3 *buf3_orig = NULL;
	alloc_buf(buf3_orig, struct s3, len);
	
	printf("Before:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf3_orig[i].a = buf3_orig[i].b;
			process(buf3_orig[i].data1);
		}
	
		for (int i = 0; i < len; i++) {
			process(buf3_orig[i].data1);
			process(buf3_orig[i].data2);
		}
	}
	
	double orig_time = measure_time_end(&start, &end);
	free(buf3_orig);
#endif

#ifdef RUN_OPT
	struct s3_opt *buf3_opt = NULL;
	alloc_buf(buf3_opt, struct s3_opt, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf3_opt[i].a = buf3_opt[i].b;
			process(buf3_opt[i].data1);
		}
	
		for (int i = 0; i< len; i++) {
			process(buf3_opt[i].data1);
			process(buf3_opt[i].data2);
		}
	}

#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	
	free(buf3_opt);
#endif
}*/


struct s4 {
	char a;
	//	padding 3 bytes
	int b;
	char c;
	//	padding 3 bytes
};

struct s4_opt {
	int b;
	char a;
	char c;
	//	padding 2 bytes
};

void example4_elimination_holes_orig(int len, int ntimes)
{
	struct s4 *buf4_orig = NULL;
	alloc_buf(buf4_orig, struct s4, len);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf4_orig[i].a++;
		}
	}

	free(buf4_orig);
}

void example4_elimination_holes_opt(int len, int ntimes)
{
	struct s4_opt *buf4_opt = NULL;
	alloc_buf(buf4_opt, struct s4_opt, len);
	
	for (int j = 0; j < ntimes; j++) {
		for(int i = 0; i < len; i++) {
			buf4_opt[i].a++;
		}
	}

	free(buf4_opt);
}

void Example4EliminationHolesOrig(benchmark::State& state) {
    for (auto _ : state) {
		example4_elimination_holes_orig(state.range(0), state.range(1));
    }
}

BENCHMARK(Example4EliminationHolesOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example4EliminationHolesOpt(benchmark::State& state) {
    for (auto _ : state) {
		example4_elimination_holes_opt(state.range(0), state.range(1));
    }
}

BENCHMARK(Example4EliminationHolesOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example4_elimination_holes(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 4 (eliminating holes)\n");
	
#ifdef RUN_ORIG
	struct s4 *buf4_orig = NULL;
	alloc_buf(buf4_orig, struct s4, len);
	
	printf("Before:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf4_orig[i].a++;
		}
	}
	
	double orig_time = measure_time_end(&start, &end);
	free(buf4_orig);
#endif

#ifdef RUN_OPT
	struct s4_opt *buf4_opt = NULL;
	alloc_buf(buf4_opt, struct s4_opt, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++) {
		for(int i = 0; i < len; i++) {
			buf4_opt[i].a++;
		}
	}
	
#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	free(buf4_opt);
#endif
}*/

struct s5 {
	int a;
	int b;
	int c;
};

struct s5_opt {
	int *a;
	int *b;
	int *c;
};

void example5_AoS_SoA_orig(int len, int ntimes)
{
	struct s5 *buf5_orig = NULL;
	alloc_buf(buf5_orig, struct s5, len);
	
	for (int j = 0; j <ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf5_orig[i].a++;
		}
	}
	
	free(buf5_orig);
}

void example5_AoS_SoA_opt(int len, int ntimes)
{
	struct s5_opt s5_opt = {.a = NULL, .b = NULL, .c = NULL};
	alloc_buf(s5_opt.a, int, len);
	alloc_buf(s5_opt.b, int, len);
	alloc_buf(s5_opt.c, int, len);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			s5_opt.a[i]++;
        }
    }

	free(s5_opt.a);
	free(s5_opt.b);
	free(s5_opt.c);
}

void Example5AoSSoAOrig(benchmark::State& state) {
    for (auto _ : state) {
		example5_AoS_SoA_orig(state.range(0), state.range(1));
    }
}

BENCHMARK(Example5AoSSoAOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example5AoSSoAOpt(benchmark::State& state) {
    for (auto _ : state) {
		example5_AoS_SoA_opt(state.range(0), state.range(1));
    }
}

BENCHMARK(Example5AoSSoAOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example5_AoS_SoA(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 5 (Aos to SoA)\n");

#ifdef RUN_ORIG
	struct s5 *buf5_orig = NULL;
	alloc_buf(buf5_orig, struct s5, len);
	
	printf("Before:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j <ntimes; j++) {
		for (int i = 0; i < len; i++) {
			buf5_orig[i].a++;
		}
	}
	
	double orig_time = measure_time_end(&start, &end);
	free(buf5_orig);
#endif
	
#ifdef RUN_OPT
	struct s5_opt s5_opt = {.a = NULL, .b = NULL, .c = NULL};
	alloc_buf(s5_opt.a, int, len);
	alloc_buf(s5_opt.b, int, len);
	alloc_buf(s5_opt.c, int, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j < ntimes; j++) {
		for (int i = 0; i < len; i++) {
			s5_opt.a[i]++;
        }
    }
    
#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	free(s5_opt.a);
	free(s5_opt.b);
	free(s5_opt.c);
#endif
}*/

struct s6 {
	int a;
	int b;
	int c;
};

void example6_struct_array_copy_orig(int len, int ntimes)
{
	struct s6 *buf6_orig = NULL;
	alloc_buf(buf6_orig, struct s6, len);
	
	for (int j = 0; j <ntimes; j++) {
		//Cant split struct besause they are accessed together
		for (int i = 0; i < len; i++) {
			buf6_orig[i].a++;
			buf6_orig[i].b++;
			buf6_orig[i].c++;
		}
		//Here access only field b many times
		for (int i = 0; i < len; i++) {
            int sum = 0;
			for (int k = 0; k < len; k++) {
				sum += buf6_orig[k].b;
				buf6_orig[k].b++;
			}
			buf6_orig[i].c = sum;
		}
	}

	free(buf6_orig);
}

void example6_struct_array_copy_opt(int len, int ntimes)
{
	//After field copying
	struct s6 *buf6_opt = NULL;
	//Here buf6_opt is the same as buf6_orig intentionally
	alloc_buf(buf6_opt, struct s6, len);
	
	for (int j = 0; j <ntimes; j++) {
		//Cant split struct besause they are accessed together
		for (int i = 0; i < len; i++) {
			buf6_opt[i].a++;
			buf6_opt[i].b++;
			buf6_opt[i].c++;
		}
		
		//Copy field b to temporal array
		int *tmp_b = NULL;
		alloc_buf(tmp_b, int, len);
		for (int i = 0; i < len; i++) {
			tmp_b[i] = buf6_opt[i].b;
		}
		
		//Here acess only field b many timespec
		for (int i = 0; i < len; i++) {
			int sum = 0;
			for (int k = 0; k < len; k++) {
				sum += tmp_b[k];
				tmp_b[k]++;
			}
			buf6_opt[i].c = sum;
		}
		
		free(tmp_b);
	}

	free(buf6_opt);
}

void Example6StructArrayCopyOrig(benchmark::State& state) {
    for (auto _ : state) {
		example6_struct_array_copy_orig(state.range(0), state.range(1));
    }
}

BENCHMARK(Example6StructArrayCopyOrig)->ArgPair(EX6_LEN, EX6_NTIMES);

void Example6StructArrayCopyOpt(benchmark::State& state) {
    for (auto _ : state) {
		example6_struct_array_copy_opt(state.range(0), state.range(1));
    }
}

BENCHMARK(Example6StructArrayCopyOpt)->ArgPair(EX6_LEN, EX6_NTIMES);

// TO DO: Check buf6_orig in RUN_OPT
/*
void example6_struct_array_copy(int len, int ntimes)
{
	struct timespec start, end;
	printf("\nExample 6 (Copy structure field)\n");
	
#ifdef RUN_ORIG
	struct s6 *buf6_orig = NULL;
	alloc_buf(buf6_orig, struct s6, len);
	
	printf("Before:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j <ntimes; j++) {
		//Cant split struct besause they are accessed together
		for (int i = 0; i < len; i++) {
			buf6_orig[i].a++;
			buf6_orig[i].b++;
			buf6_orig[i].c++;
		}
		//Here access only field b many times
		for (int i = 0; i < len; i++) {
            int sum = 0;
			for (int k = 0; k < len; k++) {
				sum += buf6_orig[k].b;
				buf6_orig[k].b++;
			}
			buf6_orig[i].c = sum;
		}
	}
	
	double orig_time = measure_time_end(&start, &end);
	free(buf6_orig);
#endif

#ifdef RUN_OPT
	//After field copying
	struct s6 *buf6_opt = NULL;
	//Here buf6_opt is the same as buf6_orig intentionally
	alloc_buf(buf6_opt, struct s6, len);
	
	printf("After:\t");
	measure_time_begin(&start);
	
	for (int j = 0; j <ntimes; j++) {
		//Cant split struct besause they are accessed together
		for (int i = 0; i < len; i++) {
			buf6_orig[i].a++;
			buf6_orig[i].b++;
			buf6_orig[i].c++;
		}
		
		//COpy field b to temporal array
		int *tmp_b = NULL;
		alloc_buf(tmp_b, int, len);
		for (int i = 0; i < len; i++) {
			tmp_b[i] = buf6_opt[i].b;
		}
		
		//Here acess only field b many timespec
		for (int i = 0; i < len; i++) {
			int sum = 0;
			for (int k = 0; k < len; k++) {
				sum += tmp_b[k];
				tmp_b[k]++;
			}
			buf6_opt[i].c = sum;
		}
		
		free(tmp_b);
	}
	
#ifdef RUN_ORIG
	double opt_time = measure_time_end(&start, &end);
	printf("Speedup: %.2lf\n", orig_time / opt_time);
#else
	measure_time_end(&start, &end);
#endif
	free(buf6_opt);
#endif
}*/

BENCHMARK_MAIN();

/*int main() {
	example1_peeling(EX12_LEN, EX12_NTIMES);
	example2_splitting(EX12_LEN, EX12_NTIMES);
	example3_field_reordering(EX3_LEN, EX3_NTIMES);
	example4_elimination_holes(EX12_LEN, EX12_NTIMES);
	example5_AoS_SoA(EX12_LEN, EX12_NTIMES);
	example6_struct_array_copy(EX6_LEN, EX6_NTIMES);
	
	return 0 ;
}*/

//Plan TODO
//1.Prepare
//	--Warm-up cache
//	--Huge-pages (?)
//	--Affinity
//	--Priority
//
//2. Statistical processing
//	--Standard deviation
//	--Confidence intervals
//	--Quartiles
//
//3. Investigate examples
// 	 in which cases optimization
//   gives best performance gain
//
// Replace all functions with macros?