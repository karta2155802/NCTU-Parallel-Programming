/* *********************************************************************
* DESCRIPTION :
* Serial Concurrent Wave Equation - C Version
* This program implements the concurrent wave equation
******************************************************************** */
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>
# define MAXPOINTS 1000000
# define MAXSTEPS 1000000
# define MINPOINTS 20
# define PI 3.14159265

# define blockSize 1024
void check_param ( void );
void init_line ( void );
void printfinal ( void );
int nsteps , /* number of time steps */
tpoints , /* total points along string */
rcode ; /* generic return code */
float values[MAXPOINTS+2], /* values at time t */
*values_d; 
/* *********************************************************************
* Checks input values from parameters
******************************************************************** */
void check_param ( void )
{
	char tchar [20];
	/* check number of points , number of iterations */
	while (( tpoints < MINPOINTS ) || ( tpoints > MAXPOINTS ) ) {
		printf ( " Enter number of points along vibrating string [% d -% d ]: ", MINPOINTS , MAXPOINTS );
		scanf ( " %s " , tchar ) ;
		tpoints = atoi ( tchar ) ;
		if (( tpoints < MINPOINTS ) || ( tpoints > MAXPOINTS ) )
			printf ( " Invalid . Please enter value between % d and % d \n " ,	MINPOINTS , MAXPOINTS );
	}
	while (( nsteps < 1) || ( nsteps > MAXSTEPS ) ) {
		printf ( " Enter number of time steps [1 -% d ]: " , MAXSTEPS );
		scanf ( " %s " , tchar ) ;
		nsteps = atoi ( tchar ) ;
		if (( nsteps < 1) || ( nsteps > MAXSTEPS ) )
			printf ( " Invalid . Please enter value between 1 and % d \n ", MAXSTEPS );
	}
	printf ( " Using points = %d , steps = %d \n " , tpoints , nsteps );
}
/* *********************************************************************
*initialization
******************************************************************** */
__global__ void init_line (int tpoints, float *values_d, float *oldval)
{    
	int i = 1 + blockIdx.x * blockSize + threadIdx.x;
	float x = (float) (i - 1) / (tpoints - 1);
	values_d[i] = __sinf(2.0 * PI * x);
	oldval[i] = values_d[i];
}
/* *********************************************************************
* merge three function
******************************************************************** */
__global__ void merge (int __tpoints, int __nsteps, float *__values_d)
{
	float oldval, newval, value;
    float dtime, c, dx, tau, sqtau;
	int i = 1 + blockIdx.x * blockSize + threadIdx.x;
	dtime = 0.3;
    c = 1.0;
    dx = 1.0;
    tau = (c * dtime / dx);
    sqtau = tau * tau;

    float x = (float) (i - 1) / (__tpoints - 1);
	value = __sinf(2.0 * PI * x);
	oldval = value;
	
	if (i <= __tpoints) {
		for (int j = 1; j <= __nsteps; j++) {
			if ((i == 1) || (i == __tpoints ))
				newval = 0;
			else 
				newval = (2.0 * value) - oldval + (sqtau * -2.0 * value);
			oldval = value;
			value = newval;			
		}
		__values_d[i] = value;
	}	
}
/* *********************************************************************
* Print final results
******************************************************************** */
void printfinal ()
{
	int i ;
	for ( i = 1; i <= tpoints ; i ++) {
		printf("%6.4f ", values[i]);
		if (i % 10 == 0)
			printf("\n");
	}
}
/* *********************************************************************
* Main program
******************************************************************** */
int main ( int argc , char * argv [])
{
	sscanf ( argv [1] , "%d" ,& tpoints );
	sscanf ( argv [2] , "%d" ,& nsteps );
	
    int numBlocks = tpoints / blockSize +  !(tpoints % blockSize == 0);
    cudaMalloc((void**) &values_d, sizeof(float) * (tpoints + 1));

	check_param ();
	printf("Initializing points on the line...\n");
	printf("Updating all points for all time steps...\n");
	
	merge<<<numBlocks, blockSize>>>(tpoints, nsteps, values_d);
	cudaMemcpy(values, values_d, sizeof(float) * (tpoints + 1), cudaMemcpyDeviceToHost);
	
	printf("Printing final results...\n");
	printfinal () ;
	printf("\nDone.\n\n");
	return 0;
}