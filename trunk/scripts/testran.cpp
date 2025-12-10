#include <iostream>
#include <random>

using namespace std;

int main(int argc, char** argv)
  {

cout << "GNU compiler version: " <<__VERSION__ << endl ; // " (" << _MACHINEBITS << " bits)" <<endl;//<<__GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<<endl;
    if( __cplusplus == 201103L ) std::cout << "C++11\n" ; 
    else if( __cplusplus == 199711L ) std::cout << "C++98\n" ;
    else std::cout << "pre-standard C++\n" ;

//std::default_random_engine generator;
std::random_device rd;
std::uniform_int_distribution<int> distribution(1,6);

int dice_roll = distribution(rd); //generator); 
cout << "dice: " << dice_roll<<endl;

/*auto dice = std::bind ( distribution, generator );
for(int i=0;i<10;i++) {
int wisdom = dice()+dice()+dice();
cout << "wisd="<<wisdom<<endl<<"dice="<<dice()<<endl;
}*/
  
std::normal_distribution<> dnorm(5,2);
 
cout << "normal: "<<dnorm(rd)<<endl;


  return 0;
  }

