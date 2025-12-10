#ifndef GENERIC_H
#define GENERIC_H

#include <string>


// From APPL 0.95 (http://bigbird.comp.nus.edu.sg/pmwiki/farm/appl/index.php?n=Main.Download)
// Core/Const.h

#undef FOREACH
#define FOREACH(type,elt,collection) \
    for (std::vector<type>::const_iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)

#undef FOREACHnc
#define FOREACHnc(type,elt,collection) \
    for (std::vector<type>::iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)

#undef LISTFOREACH
#define LISTFOREACH(type,elt,collection) \
    for (std::list<type>::const_iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)


#undef FOR
#define FOR(i,n) \
    for (unsigned int i=0, __n = (n); i<__n; i++)

//AG121009: for loop with signed i
#undef FORs
#define FORs(i,n) \
    for (int i=0, __n = (n); i<__n; i++)



/*! Copies a <tt> const char* </tt> variable to \c char* variable.
  \param[in] chrs  The char array to copy to char*
  \return new char* array
  */
char* charConstToCharArr(const char* chrs);


/*! Convert the character \c c to a number.
  Note that this only works correctly from 0 and higher, because it simply subtract the '0' character: <tt> c - '0' </tt>.
  \param c the character
  \return number
  */
char convertTextToNumChar(char c);


/*! Allocate a dynamic array of 2 dimensions.
  \param nRows number of rows
  \param nCols number of cols
  \return a pointer to the array of arrays.
  */
template <typename T>
T **AllocateDynamicArray( int nRows, int nCols) {
      T **dynamicArray;

      dynamicArray = new T*[nRows];
      for( int i = 0 ; i < nRows ; i++ )
            dynamicArray[i] = new T [nCols];

      return dynamicArray;
}


/*! Delete a dynamic array of 2 dimensions, as created by AllocateDynamicArray.
  \param dArray dynamic array
  */
template <typename T>
void FreeDynamicArray(T** dArray) {
    delete [] *dArray;
    delete [] dArray;
}


//ag130227: allocate / free matrices with 4 dimensions
/*!
 * Allocate dynamic array with 4 dimensions.
 */
template <typename T>
T ****AllocateDynamicArray( int n1, int n2, int n3, int n4) {
    T ****dynamicArray;

    dynamicArray = new T***[n1];
    for( int i1 = 0 ; i1 < n1 ; i1++ ) {
        dynamicArray[i1] = new T**[n2];
        for( int i2 = 0 ; i2 < n2 ; i2++ ) {
            dynamicArray[i1][i2] = new T*[n3];
            for( int i3 = 0 ; i3 < n3 ; i3++ ) {
                dynamicArray[i1][i2][i3] = new T [n4];
            }
        }
    }

    return dynamicArray;
}

/*!
 * Free dynamic array with 4 dimensions.
 */
template <typename T>
void FreeDynamicArray(T**** dArray) {
    delete [] ***dArray;
    delete [] **dArray;
    delete [] *dArray;
    delete [] dArray;
}


/*!
 * \brief currentTimeStamp return the current time/date as a string
 * \return
 */
std::string currentTimeStamp();



/*!
 * \brief fileExists return if the file exists (and is not a directory)
 * \param file
 * \return
 */
bool fileExists(const char* file);

/*!
 * \brief intRandomizer initializes randomizer
*/
void initRandomizer();

/*!
 * \brief random random int between 0 and max (excl.)
 * \param max
 * \return
 */
int random(int max);

/*!
 * \brief random random int between min and max (excl.)
 * \param min
 * \param max
 * \return
 */
int random(int min, int max);

/*!
 * \brief randomDouble random double between min and max (excl.)
 * \param min
 * \param max
 * \return
 */
double randomDouble(double min, double max);




#endif // GENERIC_H
