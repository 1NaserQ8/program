
#include <math.h>
#include "benchmark.hpp"

/***********************************************************************/

namespace par = pasl::sched::native;



static long par_double(const long* source, long* dest, long lo, long hi) {
  long n=hi-lo; 
  if ( n==0);
  else if(n==1)
  	dest[lo]=source[lo]*2;
   {
  long mid=(lo+hi)/2;
  par::fork2([&] { par_double(source,dest,lo,mid); },
             [&] { par_double(source,dest,mid,hi); });
}

/*---------------------------------------------------------------------*/

int main() {
	n=5;
  long xs[n] = 0;
  long ys[n];

 

  result = par_double(xs,ys,n);
  for(long i=0;i<n;i++)
   std::cout <<ys[i]<<" "
 
  return 0;
}

/***********************************************************************/
