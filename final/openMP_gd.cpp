#include <iostream>
#include <time.h>
#include <fstream>
#include <math.h>
#include <string>
#include <omp.h>
#include <sys/time.h>

#define size 864860
using namespace std;
double *x = new double[size];
double *y = new double[size];

unsigned long long int iteration;
int core;

int main(int argc, char **argv)
{
    if ( argc != 3) {
        cout<<"usage: ./[file] [iteration] [core number]"<<endl;
        exit (-1);
    }
    iteration = atof(argv[1]);
    core = atoi(argv[2]);
    
    //dataset
    ifstream file("dataset2.csv");
    if(!file.is_open()) cout<<"Error open file"<<endl;
    string tmp1,tmp2;
    for(int i=0;i<size;i++)
    {
        getline(file,tmp1,',');
        getline(file,tmp2,'\n');
        x[i]=atof(tmp1.c_str());
        y[i]=atof(tmp2.c_str());
    }
    file.close();

    omp_set_num_threads(core);

    //gradient descent
    double b0 = 0;
    double b1 = 0;
    double alpha_b0 = 0.01;
    double alpha_b1 = 0.0000001;
    for(int i=0;i<iteration;i++)
    {
        double err_b0 = 0;
        double err_b1 = 0;
        #pragma omp parallel for reduction (+:err_b0, err_b1) 
        for(int j=0;j<size;j++)
        {
            double p = b0 + b1 * x[j];
            err_b0 += p - y[j];
            err_b1 += (p - y[j]) * x[j];
        }
        err_b0 /= size;
        err_b1 /= size;
        b0 = b0 - alpha_b0 * err_b0;
        b1 = b1 - alpha_b1 *err_b1;

        //MSE
        double MSE,sum;
        #pragma omp parallel for reduction (+:sum) 
        for(int k=0;k<size;k++)
        {
            sum += pow(b0 + b1 * x[k] - y[k],2);
        }
        sum /= size;
        MSE = sqrt(sum);
        
        cout<<"err_b0="<<err_b0<<" err_b1="<<err_b1<<" b0="<<b0<<" b1="<<b1<<" MSE="<<MSE<<"\n";
    }
}
