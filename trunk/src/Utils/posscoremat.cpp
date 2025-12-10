#include "Utils/posscoremat.h"

#include <iostream>
#include <fstream>
#include "exceptions.h"

using namespace std;


PosScoreMat::PosScoreMat(std::string matFile) {
    _matFile = matFile;
}

void PosScoreMat::loadScores(int rows, int cols) {
    _map_size_x = rows;
    _map_size_y = cols;
    get_score();
}

void PosScoreMat::get_score(){
    _matrix_map_scores.resize(_map_size_x,_map_size_y);

    int p_pos_x;
    int p_pos_y;
    double score;

     ifstream reader;
     reader.open(_matFile, ifstream::in);
     if (reader.good()) {
         //read first line
         string line;
         getline(reader,line);
         int rows=-1;
         //find first comma, read #rows
         string::size_type i = line.find(",");
         if (i!=string::npos) {
            string cstr = line.substr(0,i);
            rows = atoi(cstr.c_str());
         } else {
            cout <<"ERROR @ set_get_score::readScoreFile: first parameter expected to be the number of rows."<<endl;
             exit(EXIT_FAILURE);
         }
        // cout <<"rows="<<rows<<endl;

         int r = 0;
         while(!reader.eof() && r<rows) // To get all the lines.
         {
             getline(reader,line);
             if (line.size()==0 || line.at(0)=='#') continue; //skip empty line

             string::size_type i = line.find(",");
             if (i!=string::npos) {
                // cout <<"i="<<i<<endl;
                string cstr = line.substr(1,i); // desde 1 a i, para saltar el [
                p_pos_x = atoi(cstr.c_str()); // atoi= convierte string en int
             } else {
                cout <<"ERROR @ set_get_score::readScoreFile: p_pos_x."<<endl;
                exit(EXIT_FAILURE);
             }
            // cout <<"[p_pos_x="<<p_pos_x<<",";
             string::size_type i2 = line.find("]",i+1);
             if (i2!=string::npos) {
                string cstr = line.substr(i+1,i-i2+1);
                p_pos_y = atoi(cstr.c_str());
             } else {
                cout <<"ERROR @ set_get_score::readScoreFile: p_pos_y."<<endl;
                exit(EXIT_FAILURE);
             }
             //cout <<" p_pos_y="<<p_pos_y<<" ]=";

             string::size_type i3 = line.find("]",i2+2);
             if (i3!=string::npos) {
                string rstr = line.substr(i2+2,i3-i2+2); //+2, no +1 para saltar el parentesis.
                score = std::stod(rstr.c_str());
             } else {
                cout <<"ERROR @ set_get_score::readScoreFile: score."<<endl;
                exit(EXIT_FAILURE);
             }
             _matrix_map_scores(p_pos_x,p_pos_y)=score;
             //cout <<" score ="<<score <<endl;

             r++;
         }

    }
    reader.close();

     /*cout<<"Matrix ext. scores: "<<_map_size_x<<"x"<<_map_size_y<<":";
     for(int r=0;r<_map_size_x; r++) {
         for(int c=0;c<_map_size_y; c++) {
             cout <<_matrix_map_scores(r,c)<<",";
         }
         cout<<endl;
     }*/
}


//TODO: check if row/col is consist with x/y

double PosScoreMat::getScore(int r, int c) {
    if (r<0 || r>_map_size_x || c<0 || c>_map_size_y) {
        cout <<"PosScoreMat::getScore: coordinates r"<<r<<"c"<<c<<" out of bounds, size: "<<_map_size_x<<"x"<<_map_size_y<<endl;
        throw CException(_HERE_, "get score row,col out of bounds");
    }
    return _matrix_map_scores(r,c);
}
