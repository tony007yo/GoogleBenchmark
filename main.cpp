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

/*
void measure_time_begin(struct timespec *start)
{
    clock_gettime(CLOCK_MONOTONIC, start);
}


double measureTimeEnd(struct timespec *start, struct timespec *end)
{
    clock_gettime(CLOCK_MONOTONIC, end);
    double tdiff = (end->tv_sec - start->tv_sec)
                    + 1e-9*(end->tv_nsec - start->tv_nsec);
                    printf("%lf\n", tdiff);
    return tdiff;
}
*/
#define allocBuf(buf, type, len) \
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


struct s1Hot {
    int a;
    int b;
};


struct s1Cold {
    int c;
    int d;
};

void example1PeelingOrig(struct s1 *buf1Orig, int len, int nTimes)
{
    for (int j = 0; j < nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf1Orig[i].a = buf1Orig[i].b;
        }
}

void example1PeelingOpt(struct s1Hot *buf1Hot, int len, int nTimes)
{
    for (int j = 0; j <nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf1Hot[i].a = buf1Hot[i].b;
        }
}

void Example1PeelingOrig(benchmark::State& state) {
    struct s1 *buf1Orig = NULL;
    allocBuf(buf1Orig, struct s1, state.range(0));

    for (auto s : state) {
        example1PeelingOrig(buf1Orig, state.range(0), state.range(1));
    }

    free(buf1Orig);
}

BENCHMARK(Example1PeelingOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example1PeelingOpt(benchmark::State& state) {
    struct s1Hot *buf1Hot = NULL;
    allocBuf(buf1Hot, struct s1Hot, state.range(0));

    for (auto s : state) {
        example1PeelingOpt(buf1Hot, state.range(0), state.range(1));
    }

    free(buf1Hot);
}

BENCHMARK(Example1PeelingOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example1Peeling(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 1 (peeling)\n");
#ifdef RUN_ORIG
    struct s1 *buf1Orig = NULL;
    allocBuf(buf1Orig, struct s1, len);
    
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf1Orig[i].a = buf1Orig[i].b;
        }
        
    printf("Before:\t");
    double origTime = measureTimeEnd(&start, &end);
    free(buf1Orig);
    
#endif

#ifdef RUN_OPT
    struct s1Hot *buf1Hot = NULL;;
    allocBuf(buf1Hot, struct s1Hot, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j <nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf1Hot[i].a = buf1Hot[i].b;
        }
        
#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    free(buf1Hot);
#endif
}
*/

struct s2 {
    int a;	// Hot field
    int b;	// Hot field
    int c;	// Cold field
    int d;	// Cold field
    int e;	// Cold field
    int f;	// Cold field
    struct s2 *p1;	//pointer to self
};


struct s2Hot {
    int a;	// Hot field
    int b;	// Hot field
    struct s2Hot *p1;	//pointer to self
    struct s2Cold *p2;	//pointer to Cold fiels
};


struct s2Cold {
    int c;	// Cold field
    int d;	// Cold field
    int e;	// Cold field
    int f;	// Cold field
};

void example2SplittingOrig(struct s2 *buf2Orig, int len, int nTimes)
{   
    for (int j = 0; j <nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf2Orig[i].a = buf2Orig[i].b;
        }
}

void example2SplittingOpt(struct s2Hot *buf2Hot, int len, int nTimes)
{ 
    for (int j = 0; j < nTimes; j++)
        for(int i = 0; i < len; i++) {
            buf2Hot[i].a = buf2Hot[i].b;
        }
}

void Example2SplittingOrig(benchmark::State& state) {
    struct s2 *buf2Orig = NULL;
    allocBuf(buf2Orig, struct s2, state.range(0));
    
    for (auto s : state) {
        example2SplittingOrig(buf2Orig, state.range(0), state.range(1));
    }

    free(buf2Orig);
}

BENCHMARK(Example2SplittingOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example2SplittingOpt(benchmark::State& state) {
    struct s2Hot *buf2Hot = NULL;
    allocBuf(buf2Hot, struct s2Hot, state.range(0));
    
    for (auto s : state) {
        example2SplittingOpt(buf2Hot, state.range(0), state.range(1));
    }
    
    free(buf2Hot);
}

BENCHMARK(Example2SplittingOpt)->ArgPair(EX12_LEN, EX12_NTIMES);


/*
void example2Splitting(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 2 (splitting)\n");
#ifdef RUN_ORIG
    struct s2 *buf2Orig = NULL;
    allocBuf(buf2Orig, struct s2, len);
    
    measure_time_begin(&start);
    
    for (int j = 0; j <nTimes; j++)
        for (int i = 0; i < len; i++) {
            buf2Orig[i].a = buf2Orig[i].b;
        }
    
    printf("Before:\t");
    double origTime = measureTimeEnd(&start, &end);
    free(buf2Orig);
#endif

#ifdef RUN_OPT
    struct s2Hot *buf2Hot = NULL;
    allocBuf(buf2Hot, struct s2Hot, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++)
        for(int i = 0; i < len; i++) {
            buf2Hot[i].a = buf2Hot[i].b;
        }
#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    free(buf2Hot);
#endif
}*/

struct s3 {
    int a;	//4 bytes
    int b;	//4 bytes
    int data2[S3_DATA_LEN];	//N*4 bytes
    int data1[S3_DATA_LEN];	//N*4 bytes
};


struct s3Opt {
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

void example3FieldReorderingOrig(struct s3 *buf3Orig, int len, int nTimes)
{
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf3Orig[i].a = buf3Orig[i].b;
            process(buf3Orig[i].data1);
        }
    
        for (int i = 0; i < len; i++) {
            process(buf3Orig[i].data1);
            process(buf3Orig[i].data2);
        }
    }
}

void example3FieldReorderingOpt(struct s3Opt *buf3Opt, int len, int nTimes)
{
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf3Opt[i].a = buf3Opt[i].b;
            process(buf3Opt[i].data1);
        }
    
        for (int i = 0; i< len; i++) {
            process(buf3Opt[i].data1);
            process(buf3Opt[i].data2);
        }
    }
}

void Example3ReorderingOrig(benchmark::State& state) {
    struct s3 *buf3Orig = NULL;
    allocBuf(buf3Orig, struct s3, state.range(0));
    
    for (auto s : state) {
        example3FieldReorderingOrig(buf3Orig, state.range(0), state.range(1));
    }
    
    free(buf3Orig);
}

BENCHMARK(Example3ReorderingOrig)->ArgPair(EX3_LEN, EX3_NTIMES);

void Example3ReorderingOpt(benchmark::State& state) {
    struct s3Opt *buf3Opt = NULL;
    allocBuf(buf3Opt, struct s3Opt, state.range(0));
    
    for (auto s : state) {
        example3FieldReorderingOpt(buf3Opt, state.range(0), state.range(1));
    }

    free(buf3Opt);
}

BENCHMARK(Example3ReorderingOpt)->ArgPair(EX3_LEN, EX3_NTIMES);

/*
void example3FieldReordering(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 3 (field reordering)\n");
#ifdef RUN_ORIG
    struct s3 *buf3Orig = NULL;
    allocBuf(buf3Orig, struct s3, len);
    
    printf("Before:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf3Orig[i].a = buf3Orig[i].b;
            process(buf3Orig[i].data1);
        }
    
        for (int i = 0; i < len; i++) {
            process(buf3Orig[i].data1);
            process(buf3Orig[i].data2);
        }
    }
    
    double origTime = measureTimeEnd(&start, &end);
    free(buf3Orig);
#endif

#ifdef RUN_OPT
    struct s3Opt *buf3Opt = NULL;
    allocBuf(buf3Opt, struct s3Opt, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf3Opt[i].a = buf3Opt[i].b;
            process(buf3Opt[i].data1);
        }
    
        for (int i = 0; i< len; i++) {
            process(buf3Opt[i].data1);
            process(buf3Opt[i].data2);
        }
    }

#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    
    free(buf3Opt);
#endif
}*/


struct s4 {
    char a;
    //	padding 3 bytes
    int b;
    char c;
    //	padding 3 bytes
};

struct s4Opt {
    int b;
    char a;
    char c;
    //	padding 2 bytes
};

void example4EliminationHolesOrig(struct s4 *buf4Orig, int len, int nTimes)
{ 
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf4Orig[i].a++;
        }
    }
}

void example4EliminationHolesOpt(struct s4Opt *buf4Opt, int len, int nTimes)
{
    for (int j = 0; j < nTimes; j++) {
        for(int i = 0; i < len; i++) {
            buf4Opt[i].a++;
        }
    }
}

void Example4EliminationHolesOrig(benchmark::State& state) {
    struct s4 *buf4Orig = NULL;
    allocBuf(buf4Orig, struct s4, state.range(0));
    
    for (auto s : state) {
        example4EliminationHolesOrig(buf4Orig, state.range(0), state.range(1));
    }

    free(buf4Orig);
}

BENCHMARK(Example4EliminationHolesOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example4EliminationHolesOpt(benchmark::State& state) {
    struct s4Opt *buf4Opt = NULL;
    allocBuf(buf4Opt, struct s4Opt, state.range(0));
    
    for (auto s : state) {
        example4EliminationHolesOpt(buf4Opt, state.range(0), state.range(1));
    }

    free(buf4Opt);
}

BENCHMARK(Example4EliminationHolesOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example4EliminationHoles(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 4 (eliminating holes)\n");
    
#ifdef RUN_ORIG
    struct s4 *buf4Orig = NULL;
    allocBuf(buf4Orig, struct s4, len);
    
    printf("Before:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf4Orig[i].a++;
        }
    }
    
    double origTime = measureTimeEnd(&start, &end);
    free(buf4Orig);
#endif

#ifdef RUN_OPT
    struct s4Opt *buf4Opt = NULL;
    allocBuf(buf4Opt, struct s4Opt, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++) {
        for(int i = 0; i < len; i++) {
            buf4Opt[i].a++;
        }
    }
    
#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    free(buf4Opt);
#endif
}*/

struct s5 {
    int a;
    int b;
    int c;
};

struct s5Opt {
    int *a;
    int *b;
    int *c;
};

void example5AoSSoAOrig(struct s5 *buf5Orig, int len, int nTimes)
{
    for (int j = 0; j <nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf5Orig[i].a++;
        }
    }
}

void example5AoSSoAOpt(struct s5Opt *s5Opt, int len, int nTimes)
{
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            s5Opt->a[i]++;
        }
    }
}

void Example5AoSSoAOrig(benchmark::State& state) {
    struct s5 *buf5Orig = NULL;
    allocBuf(buf5Orig, struct s5, state.range(0));
    
    for (auto s : state) {
        example5AoSSoAOrig(buf5Orig, state.range(0), state.range(1));
    }
    
    free(buf5Orig);
}

BENCHMARK(Example5AoSSoAOrig)->ArgPair(EX12_LEN, EX12_NTIMES);

void Example5AoSSoAOpt(benchmark::State& state) {
    struct s5Opt* buf5Opt = NULL;
    allocBuf(buf5Opt, struct s5Opt, 1);
    allocBuf(buf5Opt->a, int, state.range(0));
    allocBuf(buf5Opt->b, int, state.range(0));
    allocBuf(buf5Opt->c, int, state.range(0));
    
    for (auto s : state) {
        example5AoSSoAOpt(buf5Opt, state.range(0), state.range(1));
    }
    
    free(buf5Opt->a);
    free(buf5Opt->b);
    free(buf5Opt->c);
    free(buf5Opt);
}

BENCHMARK(Example5AoSSoAOpt)->ArgPair(EX12_LEN, EX12_NTIMES);

/*
void example5AoSSoA(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 5 (Aos to SoA)\n");

#ifdef RUN_ORIG
    struct s5 *buf5Orig = NULL;
    allocBuf(buf5Orig, struct s5, len);
    
    printf("Before:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j <nTimes; j++) {
        for (int i = 0; i < len; i++) {
            buf5Orig[i].a++;
        }
    }
    
    double origTime = measureTimeEnd(&start, &end);
    free(buf5Orig);
#endif
    
#ifdef RUN_OPT
    struct s5Opt s5Opt = {.a = NULL, .b = NULL, .c = NULL};
    allocBuf(s5Opt.a, int, len);
    allocBuf(s5Opt.b, int, len);
    allocBuf(s5Opt.c, int, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j < nTimes; j++) {
        for (int i = 0; i < len; i++) {
            s5Opt.a[i]++;
        }
    }
    
#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    free(s5Opt.a);
    free(s5Opt.b);
    free(s5Opt.c);
#endif
}*/

struct s6 {
    int a;
    int b;
    int c;
};

void example6StructArrayCopyOrig(struct s6 *buf6Orig, int len, int nTimes)
{
    for (int j = 0; j <nTimes; j++) {
        //Cant split struct besause they are accessed together
        for (int i = 0; i < len; i++) {
            buf6Orig[i].a++;
            buf6Orig[i].b++;
            buf6Orig[i].c++;
        }
        //Here access only field b many Times
        for (int i = 0; i < len; i++) {
            int sum = 0;
            for (int k = 0; k < len; k++) {
                sum += buf6Orig[k].b;
                buf6Orig[k].b++;
            }
            buf6Orig[i].c = sum;
        }
    }
}

void example6StructArrayCopyOpt(struct s6 *buf6Opt, int len, int nTimes)
{    
    for (int j = 0; j <nTimes; j++) {
        //Cant split struct besause they are accessed together
        for (int i = 0; i < len; i++) {
            buf6Opt[i].a++;
            buf6Opt[i].b++;
            buf6Opt[i].c++;
        }
        
        //Copy field b to temporal array
        int *tmp_b = NULL;
        allocBuf(tmp_b, int, len);
        for (int i = 0; i < len; i++) {
            tmp_b[i] = buf6Opt[i].b;
        }
        
        //Here acess only field b many timespec
        for (int i = 0; i < len; i++) {
            int sum = 0;
            for (int k = 0; k < len; k++) {
                sum += tmp_b[k];
                tmp_b[k]++;
            }
            buf6Opt[i].c = sum;
        }
        
        free(tmp_b);
    }
}

void Example6StructArrayCopyOrig(benchmark::State& state) {
    struct s6 *buf6Orig = NULL;
    allocBuf(buf6Orig, struct s6, state.range(0));
    
    for (auto s : state) {
        example6StructArrayCopyOrig(buf6Orig, state.range(0), state.range(1));
    }

    free(buf6Orig);
}

BENCHMARK(Example6StructArrayCopyOrig)->ArgPair(EX6_LEN, EX6_NTIMES);

void Example6StructArrayCopyOpt(benchmark::State& state) {
    //After field copying
    struct s6 *buf6Opt = NULL;
    //Here buf6Opt is the same as buf6Orig intentionally
    allocBuf(buf6Opt, struct s6, state.range(0));

    for (auto s : state) {
        example6StructArrayCopyOpt(buf6Opt, state.range(0), state.range(1));
    }
    

    free(buf6Opt);
}

BENCHMARK(Example6StructArrayCopyOpt)->ArgPair(EX6_LEN, EX6_NTIMES);

// TO DO: Check buf6Orig in RUN_OPT
/*
void example6StructArrayCopy(int len, int nTimes)
{
    struct timespec start, end;
    printf("\nExample 6 (Copy structure field)\n");
    
#ifdef RUN_ORIG
    struct s6 *buf6Orig = NULL;
    allocBuf(buf6Orig, struct s6, len);
    
    printf("Before:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j <nTimes; j++) {
        //Cant split struct besause they are accessed together
        for (int i = 0; i < len; i++) {
            buf6Orig[i].a++;
            buf6Orig[i].b++;
            buf6Orig[i].c++;
        }
        //Here access only field b many Times
        for (int i = 0; i < len; i++) {
            int sum = 0;
            for (int k = 0; k < len; k++) {
                sum += buf6Orig[k].b;
                buf6Orig[k].b++;
            }
            buf6Orig[i].c = sum;
        }
    }
    
    double origTime = measureTimeEnd(&start, &end);
    free(buf6Orig);
#endif

#ifdef RUN_OPT
    //After field copying
    struct s6 *buf6Opt = NULL;
    //Here buf6Opt is the same as buf6Orig intentionally
    allocBuf(buf6Opt, struct s6, len);
    
    printf("After:\t");
    measure_time_begin(&start);
    
    for (int j = 0; j <nTimes; j++) {
        //Cant split struct besause they are accessed together
        for (int i = 0; i < len; i++) {
            buf6Orig[i].a++;
            buf6Orig[i].b++;
            buf6Orig[i].c++;
        }
        
        //COpy field b to temporal array
        int *tmp_b = NULL;
        allocBuf(tmp_b, int, len);
        for (int i = 0; i < len; i++) {
            tmp_b[i] = buf6Opt[i].b;
        }
        
        //Here acess only field b many timespec
        for (int i = 0; i < len; i++) {
            int sum = 0;
            for (int k = 0; k < len; k++) {
                sum += tmp_b[k];
                tmp_b[k]++;
            }
            buf6Opt[i].c = sum;
        }
        
        free(tmp_b);
    }
    
#ifdef RUN_ORIG
    double optTime = measureTimeEnd(&start, &end);
    printf("Speedup: %.2lf\n", origTime / optTime);
#else
    measureTimeEnd(&start, &end);
#endif
    free(buf6Opt);
#endif
}*/

BENCHMARK_MAIN();

/*int main() {
    example1Peeling(EX12_LEN, EX12_NTIMES);
    example2Splitting(EX12_LEN, EX12_NTIMES);
    example3FieldReordering(EX3_LEN, EX3_NTIMES);
    example4EliminationHoles(EX12_LEN, EX12_NTIMES);
    example5AoSSoA(EX12_LEN, EX12_NTIMES);
    example6StructArrayCopy(EX6_LEN, EX6_NTIMES);
    
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
// 	 in which cases Optimization
//   gives best performance gain
//
// Replace all functions with macros?