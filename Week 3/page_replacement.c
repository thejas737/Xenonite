#include "page_replacement.h"
#include <string.h>
const char *algorithm_name(Algorithm a){return a==FIFO?"FIFO":a==LRU?"LRU":a==OPTIMAL?"Optimal":"Unknown";}
static int find(const int *f,int n,int p){for(int i=0;i<n;i++)if(f[i]==p)return i;return -1;}
static int optimal(const int *p,int n,int from,const int *f,int nf){int v=0,far=-1;for(int i=0;i<nf;i++){int next=n+1;for(int j=from;j<n;j++)if(p[j]==f[i]){next=j;break;}if(next>far){far=next;v=i;}}return v;}
int simulate(const int *p,int n,int nf,Algorithm a,SimulationResult *r){
 if(!p||!r||n<1||n>MAX_REFERENCES||nf<1||nf>MAX_FRAMES||a<FIFO||a>OPTIMAL)return 0;
 memset(r,0,sizeof(*r));r->frame_count=nf;r->reference_count=n;
 int f[MAX_FRAMES],last[MAX_FRAMES],loaded=0,next_fifo=0;
 for(int j=0;j<nf;j++){f[j]=-1;last[j]=-1;}
 for(int i=0;i<n;i++){int slot=find(f,nf,p[i]);
  if(slot>=0){r->hits++;r->hit[i]=1;if(a==LRU)last[slot]=i;}
  else{r->faults++;r->hit[i]=0;
   if(loaded<nf)slot=loaded++;
   else if(a==FIFO){slot=next_fifo;next_fifo=(next_fifo+1)%nf;}
   else if(a==LRU){slot=0;for(int j=1;j<nf;j++)if(last[j]<last[slot])slot=j;}
   else slot=optimal(p,n,i+1,f,nf);
   f[slot]=p[i];if(a==LRU)last[slot]=i;
  }
  for(int j=0;j<nf;j++)r->frames[i][j]=f[j];
 }
 r->hit_ratio=100.0*r->hits/n;r->fault_ratio=100.0*r->faults/n;return 1;
}
