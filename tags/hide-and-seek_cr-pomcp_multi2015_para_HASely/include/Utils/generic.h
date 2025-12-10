#ifndef GENERIC_H
#define GENERIC_H

#include <string>

#ifdef USE_QT
#include <QString>
#endif

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
    for (std::size_t i=0, __n = (n); i<__n; i++)

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
T **AllocateDynamicArray( unsigned int nRows, unsigned int nCols) {
      T **dynamicArray;

      dynamicArray = new T*[nRows];
      for( unsigned int i = 0 ; i < nRows ; i++ )
            dynamicArray[i] = new T [nCols];

      return dynamicArray;
}


/*! Delete a dynamic array of 2 dimensions, as created by AllocateDynamicArray.
  \param dArray dynamic array
  */
template <typename T>
void FreeDynamicArray(T** dArray, unsigned int nRows) {
    //delete [] *dArray; //AG140501: FIX, this was not deleting the whole matrix!
    for( unsigned int i = 0 ; i < nRows ; i++ )
        delete [] dArray[i];

    delete [] dArray;
}

/*!
 * Allocate dynamic array with 3 dimensions.
 */
template <typename T>
T ***AllocateDynamicArray( unsigned int n1, unsigned int n2, unsigned int na) {
    T ***dynamicArray;

    dynamicArray = new T**[n1];
    for( unsigned int i1 = 0 ; i1 < n1 ; i1++ ) {
        dynamicArray[i1] = new T*[n2];
        for( unsigned int i2 = 0 ; i2 < n2 ; i2++ ) {
            dynamicArray[i1][i2] = new T [na];
        }
    }

    return dynamicArray;
}

/*!
 * Free dynamic array with 3 dimensions.
 */
template <typename T>
void FreeDynamicArray(T*** dArray, unsigned int n1, unsigned int n2) {
    //delete [] **dArray;
    //delete [] *dArray; //AG140501: FIX, this was not deleting the whole matrix!
    for( unsigned int i1 = 0 ; i1 < n1 ; i1++ ) {
        for( unsigned int i2 = 0 ; i2 < n2 ; i2++ ) {
            delete [] dArray[i1][i2];
        }
        delete [] dArray[i1];
    }

    delete [] dArray;
}


//ag130227: allocate / free matrices with 4 dimensions
/*!
 * Allocate dynamic array with 4 dimensions.
 */
template <typename T>
T ****AllocateDynamicArray( unsigned int n1, unsigned int n2, unsigned int n3, unsigned int n4) {
    T ****dynamicArray;

    dynamicArray = new T***[n1];
    for( unsigned int i1 = 0 ; i1 < n1 ; i1++ ) {
        dynamicArray[i1] = new T**[n2];
        for( unsigned int i2 = 0 ; i2 < n2 ; i2++ ) {
            dynamicArray[i1][i2] = new T*[n3];
            for( unsigned int i3 = 0 ; i3 < n3 ; i3++ ) {
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
void FreeDynamicArray(T**** dArray, unsigned int n1, unsigned int n2, unsigned int n3) {
    //delete [] ***dArray; //AG140501: FIX, this was not deleting the whole matrix!
    //delete [] **dArray;
    //delete [] *dArray;

    for( unsigned int i1 = 0 ; i1 < n1 ; i1++ ) {
        for( unsigned int i2 = 0 ; i2 < n2 ; i2++ ) {
            for( unsigned int i3 = 0 ; i3 < n3 ; i3++ ) {
                delete [] dArray[i1][i2][i3];
            }
            delete [] dArray[i1][i2];
        }
        delete [] dArray[i1];
    }

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
 * \brief random random int between 0 and max (incl.)
 * \param max
 * \return
 */
int random(int max);

/*!
 * \brief random random int between min and max (incl.)
 * \param min
 * \param max
 * \return
 */
int random(int min, int max);

/*!
 * \brief randomDouble random double between min and max (incl.)
 * \param min
 * \param max
 * \return
 */
double randomDouble(double min=0, double max=1.0);


#ifdef USE_QT
/*!
 * \brief qstringToBool returns true if string equals: "true"/"yes"/"1" and false if "false"/"no"/"0"
 * ignoring case. The parameter ok=true if it equals on any of the previous values, otherwise false.
 * \param str the string
 * \param ok is the string value a correct boolean or no
 * \return
 */
bool qstringToBool(QString str, bool* ok=NULL);

/*!
 * \brief argsToQString generate a string of the list of parameters
 * \param argc
 * \param argv
 * \return
 */
QString argsToQString(int argc, char *argv[]);
#endif

//AG140908
/*!
 * \brief sqrd square
 * \param x
 * \return
 */
double sqrd(double x);



#endif // GENERIC_H
