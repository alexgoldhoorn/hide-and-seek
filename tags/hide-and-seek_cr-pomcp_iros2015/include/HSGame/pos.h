#ifndef POS_H
#define POS_H

#ifdef USE_QT
#include <QPoint>
#include <QDataStream>
#endif

#include <string>
#include <vector>


/*!
 * \brief The Pos struct Position with a row and column. It contains the integer and double value.
 */
struct Pos {
    //AG140109: allow GMap to access the values directly
    //  but: be careful not to write them! (we want to write both int and double values at the same time)
    friend class GMap;

    Pos(int row_,int col_);
    Pos(unsigned int row_, unsigned int col_); //AG150122
    Pos(double row_,double col_);
    Pos(const Pos& p);

    Pos(const std::vector<double>& vec);

    Pos();

#ifdef USE_QT
    Pos(const QPoint& p);
#endif

    virtual ~Pos();

    int row() const;

    int col() const;

    double rowDouble() const;
    double colDouble() const;

    /*!
     * \brief toString return position as "r{row}c{col}", when it are continuous (double) values, the integer row and col are added between [].
     * \return
     */
    virtual std::string toString() const;

    /*!
     * \brief equals return true if passed position is the same
     * \param p
     * \return
     */
    virtual bool equals(const Pos& p) const;

    /*!
     * \brief equals compares double
     * \param rows
     * \param cols
     * \return
     */
    virtual bool equals(double rows, double cols) const;

    /*!
     * \brief equalsInt compare using int
     * \param p
     * \return
     */
    virtual bool equalsInt(const Pos& p) const;

    /*!
     * \brief equalsInt comparing using itn
     * \param rows
     * \param cols
     * \return
     */
    virtual bool equalsInt(int rows, int cols) const;

    /*!
     * \brief set set current pos equals to param pos
     * \param p
     */ //depricated by =  operator
    //void set(const Pos& p);

    virtual Pos& operator = (const Pos& pos);

    /*!
     * \brief set int values
     * \param row_
     * \param col_
     */
    void set(int row_, int col_);

    /*!
     * \brief set unsigned int values
     * \param row_
     * \param col_
     */
    void set(unsigned int row_, unsigned int col_);

    /*!
     * \brief set set double values
     * \param row_
     * \param col_
     */
    void set(double row_, double col_);

    /*!
     * \brief isSet returns if is set (-1 if not set)
     * \return
     */
    bool isSet() const;

    /*!
     * \brief add add double values to row and col
     * \param ar
     * \param ac
     */
    void add(double ar, double ac);

    /*!
     * \brief add add int values to row and col
     * \param ar
     * \param ac
     */
    void add(int ar, int ac);

    /*!
     * \brief clear clear value (-1,-1)
     */
    void clear();

    bool operator <(const Pos& other) const;

    bool operator >(const Pos& other) const;

    virtual bool operator ==(const Pos& other) const;

    virtual bool operator !=(const Pos& other) const;

    /*!
     * \brief getHash
     * \return
     */
    std::string getHash(/*bool useDouble=false*/) const;

    /*!
     * \brief hasDouble does the pos contain real values
     * \return
     */
    bool hasDouble() const;

    /*!
     * \brief convertValuesToInt convert the double values to int (used for observations for ex.)
     */
    void convertValuesToInt();

    /*!
     * \brief toVector generate vector from pos
     * \return
     */
    //void toVector(std::vector<double>& vec);
    std::vector<double> toVector();

    /*!
     * \brief distanceEuc euclidean distance to p
     * \param p
     * \return
     */
    double distanceEuc(const Pos& p) const;

#ifdef USE_QT
    //AG131211: functions to read/write map from stream in GMap to make them equal for all

    /*!
     * \brief readPosFromStream reads a pos from the stream
     * \param in
     * \param useDouble
     */
    virtual void readPosFromStream(QDataStream& in, bool useDouble);

    /*!
     * \brief writePostoStream writes pos to stream
     * \param out
     * \param useDouble
     */
    virtual void writePostoStream(QDataStream& out, bool useDouble) const;
#endif


protected:
    //AG140109: added double values for continuous values
    double _rowDouble;
    double _colDouble;

    //AG140109: row and col have to be private/protected such that
    int _row;
    int _col;

    //use double
    bool _useDouble;

};

#endif // POS_H
