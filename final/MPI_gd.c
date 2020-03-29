#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define size 864860

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Status status;
    int MPIrank, MPIsize;
    MPI_Comm_size(MPI_COMM_WORLD, &MPIsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &MPIrank);

    unsigned long long int iteration;
    unsigned long long int data_per_thread;
    int core;

    /*if ( argc != 3) {
        printf("usage: ./[file] [iteration] [core number]\n");
        exit (-1);
    }*/
    iteration = atof(argv[1]);
    core = atoi(argv[2]);
    data_per_thread = size / core;

    if(MPIrank == 0) printf("%llu,%d,%llu",iteration,core,data_per_thread);
    
    //dataset
    FILE *fp;
    fp = fopen("dataset2.csv","r");

    double *x = (double *)malloc(sizeof(double)*size);
    double *y = (double *)malloc(sizeof(double)*size);

    char line[20];
    char *result = NULL;
    for(int i=0;i<size;i++)
    {
        fgets(line,20,fp);
        result = strtok(line,",");
        x[i] = atof(result);
        result = strtok(NULL,",");
        y[i] = atof(result);
        result = strtok(NULL,",");
    }    

    //gradient descent
    double b0 = 0;
    double b1 = 0;
    double alpha_b0 = 0.01;
    double alpha_b1 = 0.0000001;
    for(int i=0;i<iteration;i++)
    {
        double err_b0 = 0;
        double err_b1 = 0;
        for(int j = MPIrank * data_per_thread ; j < (MPIrank + 1) * data_per_thread ; j++)
        {
            double p = b0 + b1 * x[j];
            err_b0 += p - y[j];
            err_b1 += (p - y[j]) * x[j];
        }

        //send data
        if(MPIrank != 0)
        {
            MPI_Send(&err_b0,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
            MPI_Send(&err_b1,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD);
            MPI_Recv(&b0,1,MPI_DOUBLE,0,2,MPI_COMM_WORLD,&status);
            MPI_Recv(&b1,1,MPI_DOUBLE,0,3,MPI_COMM_WORLD,&status);
        }
        else //receive data
        {
            double other_err_b0, other_err_b1;
            for(int source = 1; source < MPIsize; source++)
            {
                MPI_Recv(&other_err_b0,1,MPI_DOUBLE,source,0,MPI_COMM_WORLD,&status);
                MPI_Recv(&other_err_b1,1,MPI_DOUBLE,source,1,MPI_COMM_WORLD,&status);
                err_b0 += other_err_b0;
                err_b1 += other_err_b1;
            }
            err_b0 /= size;
            err_b1 /= size;
            b0 = b0 - alpha_b0 * err_b0;
            b1 = b1 - alpha_b1 *err_b1;
            for(int source = 1; source < MPIsize; source++)
            {
                MPI_Send(&b0,1,MPI_DOUBLE,source,2,MPI_COMM_WORLD);
                MPI_Send(&b1,1,MPI_DOUBLE,source,3,MPI_COMM_WORLD);
            }

        }

        //MSE
        double MSE,sum;
        sum = 0;
        for(int k = MPIrank * data_per_thread ; k < (MPIrank + 1) * data_per_thread ; k++)
        {
            sum += pow(b0 + b1 * x[k] - y[k],2);
        }

        //send data
        if(MPIrank != 0)
        {
            MPI_Send(&sum,1,MPI_DOUBLE,0,4,MPI_COMM_WORLD);
        }
        else //receive data
        {
            double other_sum;
            for(int source = 1; source < MPIsize; source++)
            {
                MPI_Recv(&other_sum,1,MPI_DOUBLE,source,4,MPI_COMM_WORLD,&status);
                sum += other_sum;
            }

            sum /= size;
            MSE = sqrt(sum);
        
            printf("err_b0=%lf err_b1=%lf b0=%lf b1=%lf MSE=%lf\n",err_b0,err_b1,b0,b1,MSE);
        }

    }

    MPI_Finalize();
    return 0;
}
