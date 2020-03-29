#include <iostream>
#include <time.h>
#include <fstream>
#include <math.h>
#include <string>
#include <pthread.h>
#include <sys/time.h>

#define size 864860
using namespace std;
double *x = new double[size];
double *y = new double[size];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long long int iteration;
unsigned long long int data_per_thread;
int core;
double global_errb0, global_errb1;
double b0 = 0, b1 = 0;
double sum=0;
struct timeval Start , End;


void* MeanSquareError(void* arg)
{
    int id = *(int* )arg;
    double local_sum = 0;
    for(int i=id * data_per_thread;i<(id + 1)*data_per_thread;i++)
    {
        local_sum += pow(b0 + b1 * x[i] - y[i], 2);
    }
    pthread_mutex_lock(&lock);
    sum+=local_sum;
    pthread_mutex_unlock(&lock);
    //pthread_exit(NULL);
}

void* GradientDescent(void* arg)
{
    int id = *(int* )arg;
    double local_errb0 = 0, local_errb1 = 0;
    for(int i=id * data_per_thread;i<(id + 1)*data_per_thread;i++)
    {
        double p = b0 + b1 * x[i];
        local_errb0 += p - y[i];
        local_errb1 += (p - y[i]) * x[i];
    }
    pthread_mutex_lock(&lock);
    global_errb0 += local_errb0;
    global_errb1 += local_errb1;
    pthread_mutex_unlock(&lock);
    //pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if ( argc != 3) {
        cout<<"usage: ./[file] [iteration] [core number]"<<endl;
        exit (-1);
    }
    iteration = atof(argv[1]);
    core = atoi(argv[2]);
    data_per_thread = size / core;
    
    //dataset
    ifstream file("dataset2.csv");
    if(!file.is_open()) cout<<"Error open file"<<endl;
    string tmp1,tmp2;
    for(int i=0;i<size;i++)
    {
        getline(file,tmp1,',');
        getline(file,tmp2,'\n');
        x[i] = atof(tmp1.c_str());
        y[i] = atof(tmp2.c_str());
    }

    pthread_t *thread = new pthread_t[core];
    //gradient descent
    double alpha_b0 = 0.01;
    double alpha_b1 = 0.0000001;
    //fstream file1;
    //file1.open("result.csv",ios::out);
    int para[core];
    
    for(int i=0;i<core;i++)
    {
        para[i]=i;
    }
    gettimeofday (&Start , NULL);
    for(int i=0;i<iteration;i++)
    {
        pthread_t *thread = new pthread_t[core];
        sum = 0;
        global_errb0 = 0;
        global_errb1 = 0;
        for(int j=1;j<core;j++)
        {
            pthread_create(&thread[j], NULL, GradientDescent, &para[j]);
        }
        GradientDescent(&para[0]);
        for(int j=1;j<core;j++)
        {
            pthread_join(thread[j], NULL);
        } 

        global_errb0 /= size;
        global_errb1 /= size;
        b0 = b0 - alpha_b0 * global_errb0;
        b1 = b1 - alpha_b1 * global_errb1;

        /*for(int j=1;j<core;j++)
        {
            para[j] = j; 
            pthread_create(&thread[j], NULL, MeanSquareError, &para[j]);
        }
        MeanSquareError(&para[0]);
        for(int j=1;j<core;j++)
        {
            pthread_join(thread[j], NULL);
        }
        sum /= size;
        double MSE = sqrt(sum);*/
        cout<<"global_errb0="<<global_errb0<<" global_errb1="<<global_errb1<<" b0="<<b0<<" b1="<<b1<<"\n";
        //cout<<"MSE="<<MSE<<"\n";
        //file1<<i<<","<<MSE<<"\n";
        free(thread);
    }
    gettimeofday (&End, NULL);
    double MSE,sum;
    for(int k=0;k<size;k++)
    {
        sum += pow(b0 + b1 * x[k] - y[k],2);
    }
    sum /= size;
    MSE = sqrt(sum);
    cout<<"MSE="<<MSE<<"\n";
    
    //file1.close();
    cout<<(( double)End.tv_sec - (double)Start.tv_sec ) + (( double)End.tv_usec - (double)Start.tv_usec )/1000000.0<<" s\n";
}
