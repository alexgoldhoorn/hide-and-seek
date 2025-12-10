#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <cstdlib>

// g++ testRand.cpp -std=c++11 -o testRand
std::random_device __randomDevice;
std::mt19937 __randomGenerator(__randomDevice());
std::uniform_real_distribution<> __uniformRealDistr(0,1);
std::uniform_int_distribution<> __uniformIntDistr(0,100);
std::normal_distribution<double> __gausDistribution(0,1);

using namespace std;


void initRandomizer() {
    //TODO: not necessary (if linux..)
    //srand (time(NULL));//TODO more random (use ms/ns or /dev/random)
}

int random(int max)
{
    decltype(__uniformIntDistr.param()) new_range (0, max);
    __uniformIntDistr.param(new_range);
    return __uniformIntDistr(__randomGenerator);
}

int random(int min, int max)
{
    decltype(__uniformIntDistr.param()) new_range (min, max);
    __uniformIntDistr.param(new_range);
    return __uniformIntDistr(__randomGenerator);
}

int randomOld(int max)
{
    return rand() % (max+1);
}

double randomDouble(double min, double max)
{
    decltype(__uniformRealDistr.param()) new_range (min, max);
    __uniformRealDistr.param(new_range);
    return __uniformRealDistr(__randomGenerator);
}

int main(int argc, char *argv[])
{
double mean=0,std=1;
if (argc>1) {
	std = atof(argv[1]);
	if (argc>2) {
		mean=atof(argv[2]);
	}
}
std::cout << "Normal distribution - mean="<<mean<<", std="<<std<<std::endl;
    std::map<int, int> hist;
    
    decltype(__gausDistribution.param()) newParams (mean, std);
    __gausDistribution.param(newParams);
	int total = 10000;
	int cnt1Std = 0;

    for(int n=0; n<total; ++n) {
	double r = __gausDistribution(__randomGenerator);
        ++hist[(int)(r)];
	if (r>=mean-std && r<=mean+std) cnt1Std++;
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(3)
                  << p.first << ' ' << std::string(p.second/50, '*') << '\n';
    }
	std::cout << std::endl<< cnt1Std <<"/"<<total<<": "<<(100.0*cnt1Std/total)<<"% in 1st stddev"<<std::endl;
}

int main4() {
	int max = 9;
	int c=0,cOld=0;
	for (int i=0;i<1000;i++) {
		if (random(max)==max) c++;
		if (randomOld(max)==max) cOld++;
	}
	cout << "count: "<<c<<"; count old: "<<cOld<<endl;
	
}

int main3()
{
std::cout << "Double (0,5):"<<std::endl;
    std::map<int, int> hist;
    for(int n=0; n<10000; ++n) {
        ++hist[(int)(randomDouble(0,5))];
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(3)
                  << p.first << ' ' << std::string(p.second/50, '*') << '\n';
    }

std::cout <<std::endl << "Int (0,5):"<<std::endl;
    hist.clear();
    for(int n=0; n<10000; ++n) {
        ++hist[random(0,5)];
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(3)
                  << p.first << ' ' << std::string(p.second/50, '*') << '\n';
    }

}

int main2()
{
    std::random_device rd;
    std::mt19937 gen(rd());
 
    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> d(0,5);
 
    std::map<int, int> hist;
    for(int n=0; n<10000; ++n) {
        ++hist[std::round(d(gen))];
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(3)
                  << p.first << ' ' << std::string(p.second/50, '*') << '\n';
    }


    std::uniform_real_distribution<> dis(1, 2);
    for (int n = 0; n < 10; ++n) {
        std::cout << dis(gen) << ' ';
    }
    std::cout << '\n';

//std::uniform_real_distribution<> dis(1, 2);
decltype(dis.param()) new_range (0, 1);
	dis.param(new_range);
    for (int n = 0; n < 10; ++n) {
        std::cout << dis(gen) << ' ';
    }
    std::cout << '\n';

}
