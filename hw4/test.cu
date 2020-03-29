#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

void check_param(void);
void init_line(void);
void update (void);
void printfinal (void);

int nsteps,                 	/* number of time steps */
    tpoints, 	     		/* total points along string */
    rcode;                  	/* generic return code */

float *hostVal;
float *devOldVal, *devNewVal;

/**********************************************************************
 *	Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
    char tchar[20];

    /* check number of points, number of iterations */
    while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
        printf("Enter number of points along vibrating string [%d-%d]: "
                ,MINPOINTS, MAXPOINTS);
        scanf("%s", tchar);
        tpoints = atoi(tchar);
        if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
            printf("Invalid. Please enter value between %d and %d\n", 
                    MINPOINTS, MAXPOINTS);
    }
    while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
        printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
        scanf("%s", tchar);
        nsteps = atoi(tchar);
        if ((nsteps < 1) || (nsteps > MAXSTEPS))
            printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
    }

    printf("Using points = %d, steps = %d\n", tpoints, nsteps);

}

/**********************************************************************
 *     Initialize points on line
 *********************************************************************/
__global__ 
void init_line(int __tpoints, float* __oldval, float* __newval)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    /* Calculate initial values based on sine curve */
    for (int i = index; i < __tpoints; i+=stride) {
        float x = (float)i / (__tpoints-1);
        __oldval[i] = __newval[i] = __sinf(2.0 * PI * x);
    } 
}

/**********************************************************************
 *      Calculate new values using wave equation
 *********************************************************************/
__device__
float do_math(float __newval, float __oldval)
{
    float dtime, c, dx, tau, sqtau;

    dtime = 0.3;
    c = 1.0;
    dx = 1.0;
    tau = (c * dtime / dx);
    sqtau = tau * tau;
    return (2.0 * __newval) - __oldval + (sqtau *  (-2.0)*__newval);
}

/**********************************************************************
 *     Update all values along line a specified number of times
 *********************************************************************/
__global__
void update(int __tpoints, int __nsteps, float* __oldval, float* __newval)
{
    float __localval;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    /* Update values for each time step */
    for (int i = 0; i< __nsteps; i++) {
        /* Update points along line for this time step */
        for (int j = index; j < __tpoints; j+=stride) {
            /* global endpoints */
            if ((j == 0) || (j  == (__tpoints-1)))
                __newval[j] = 0.0;
            else
                __localval = do_math(__newval[j], __oldval[j]);
                __oldval[j] = __newval[j];
                __newval[j] = __localval;
        }
    }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
    int i;

    for (i = 0; i < tpoints; i++) {
        printf("%6.4f ", hostVal[i]);
        if ((i+1)%10 == 0)
            printf("\n");
    }
}

/**********************************************************************
 *	Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
    sscanf(argv[1],"%d",&tpoints);
    sscanf(argv[2],"%d",&nsteps);

    int blockSize = 256;
    int numBlocks = (tpoints + blockSize + 1) / blockSize; 

    cudaMalloc(&devOldVal, (MAXPOINTS+2) * sizeof(float));
    cudaMalloc(&devNewVal, (MAXPOINTS+2) * sizeof(float));

    check_param();
    printf("Initializing points on the line...\n");
    init_line<<<numBlocks, blockSize>>>(tpoints, devOldVal, devNewVal);
    printf("Updating all points for all time steps...\n");
    update<<<numBlocks, blockSize>>>(tpoints, nsteps, devOldVal, devNewVal);

    hostVal = (float*) malloc(sizeof(float) * tpoints);

    cudaDeviceSynchronize();

    cudaMemcpy(hostVal, devNewVal, sizeof(float) * tpoints, cudaMemcpyDeviceToHost);

    printf("Printing final results...\n");
    printfinal();
    printf("\nDone.\n\n");

    return 0;
}