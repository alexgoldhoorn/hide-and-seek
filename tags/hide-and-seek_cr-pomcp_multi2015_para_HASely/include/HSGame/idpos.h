#ifndef IDPOS_H
#define IDPOS_H
#include "HSGame/pos.h"

/*!
 * \brief The IDPos struct A positions with (person/object) ID. The ID refers to the person/object, and is therefore
 * also when copying, but not when comparing
 */
struct IDPos : public Pos {

    IDPos(int row_,int col_, int id);
    IDPos(unsigned int row_, unsigned int col_, int id); //AG150122
    IDPos(double row_,double col_, int id);
    IDPos(const IDPos& p);
    IDPos(const Pos& p, int id);

    IDPos(const std::vector<double>& vec, int id);

    IDPos(int id);

    IDPos();

#ifdef USE_QT
    IDPos(const QPoint& p, int id);
#endif

    virtual ~IDPos();

    /*!
     * \brief toString return position as Pos.toString, adding the ID before between < and >: "<{id}>r{row}c{col}"
     * \return
     */
    virtual std::string toString() const;

    /*!
     * \brief equalsWithID return true if passed position and ID are the same
     * \param p
     * \return
     */
    virtual bool equalsWithID(const IDPos& p) const;

    /*!
     * \brief equalsWithID return true if passed position and ID are the same
     * \param row
     * \param col
     * \param id
     * \return
     */
    virtual bool equalsWithID(double row, double col, int id) const;

    /*!
     * \brief equalsIntWithID return true if passed position (using int values only) and ID are the same
     * \param p
     * \return
     */
    virtual bool equalsIntWithID(const IDPos& p) const;

    /*!
     * \brief equalsIntWithID return true if passed position (using int values only) and ID are the same
     * \param rows
     * \param cols
     * \param id
     * \return
     */
    virtual bool equalsIntWithID(int rows, int cols, int id) const;


    virtual IDPos& operator = (const IDPos& pos);

    virtual IDPos& operator = (const Pos& pos);

    /*!
     * \brief set int values
     * \param row_
     * \param col_
     * \param id
     */
    void set(int row_, int col_, int id);

    /*!
     * \brief set unsigned int values
     * \param row_
     * \param col_
     * \param id
     */
    void set(unsigned int row_, unsigned int col_, int id);

    /*!
     * \brief set set double values
     * \param row_
     * \param col_
     * \param id
     */
    void set(double row_, double col_, int id);

    /*!
     * \brief set get position from pos, keep same ID
     * \param pos
     */
    void set(const Pos& pos);

    /*!
     * \brief getID returns the id (refers to the object/person)
     * \return
     */
    int id() const;

#ifdef USE_QT
    virtual void readPosFromStream(QDataStream& in, bool useDouble);

    virtual void writePostoStream(QDataStream& out, bool useDouble);
#endif


protected:
    int _id;
};

#endif // IDPOS_H
