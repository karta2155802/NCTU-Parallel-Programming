#include <iostream>
#include <time.h>
#include <fstream>
#include <math.h>
#include <string>
#include <sys/time.h>

#define size 864860
using namespace std;
double *x = new double[size];
double *y = new double[size];

unsigned long long int iteration;
int core;

struct timeval Start , End;

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
    

    
    //gradient descent
    double b0 = 0;
    double b1 = 0;
    double alpha_b0 = 0.01;
    double alpha_b1 = 0.0000001;
    gettimeofday (&Start , NULL);
    for(int i=0;i<iteration;i++)
    {
        double err_b0 = 0;
        double err_b1 = 0;
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
        /*double MSE,sum;
        for(int k=0;k<size;k++)
        {
            sum += pow(b0 + b1 * x[k] - y[k],2);
        }
        sum /= size;
        MSE = sqrt(sum);*/
        
        cout<<"err_b0="<<err_b0<<" err_b1="<<err_b1<<" b0="<<b0<<" b1="<<b1<<"\n";
        //cout<<" MSE="<<MSE<<"\n";        
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
    cout<<(( double)End.tv_sec - (double)Start.tv_sec ) + (( double)End.tv_usec - (double)Start.tv_usec )/1000000.0<<" s\n";
}
