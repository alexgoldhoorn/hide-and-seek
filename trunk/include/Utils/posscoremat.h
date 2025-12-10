#ifndef POSSCOREMAT_H
#define POSSCOREMAT_H

#include <string>
#include <eigen3/Eigen/Dense>

// Code from Ely
/*!
 * \brief The PosScoreMat class that reads a file with per position a score:
 *
 * nr-of-lines
 * [row1,col1][score1]
 * [row2,col2][score2]
 * ...
 *
 *
 */
class PosScoreMat {
public:
    PosScoreMat(std::string matFile);

    //! load scores when size is known
    void loadScores(int rows, int cols);

    /*!
     * \brief getScore returns the score for location [r,c]
     * \param r
     * \param c
     * \return
     */
    double getScore(int r, int c);

private:
    void get_score();

    //matrix file
    std::string _matFile;

    //! Matrix of scores. matrix_map_scores_(px,py)=value_score
    Eigen::MatrixXd _matrix_map_scores;

    int _map_size_x;

    int _map_size_y;

};

#endif // POSSCOREMAT_H
