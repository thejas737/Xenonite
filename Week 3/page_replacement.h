#ifndef PR_H
#define PR_H
#define MAX_REFERENCES 1000
#define MAX_FRAMES 100
typedef enum { FIFO=1, LRU, OPTIMAL } Algorithm;
typedef struct { int hits,faults,frame_count,reference_count; double hit_ratio,fault_ratio; int frames[MAX_REFERENCES][MAX_FRAMES]; int hit[MAX_REFERENCES]; } SimulationResult;
const char *algorithm_name(Algorithm);
int simulate(const int[],int,int,Algorithm,SimulationResult*);
void print_simulation(const int[],const SimulationResult*,Algorithm);
void print_comparison(const SimulationResult[],int);
#endif
