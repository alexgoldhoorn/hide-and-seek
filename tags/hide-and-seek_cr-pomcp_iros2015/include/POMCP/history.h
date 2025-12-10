#ifndef HISTORY_H
#define HISTORY_H

#include <vector>
#include "POMCP/state.h"

namespace pomcp {

/*!
 * \brief The History class the action-observation history
 */
class History {
public:

    /*!
     * \brief The HistoryItem struct contains an action and observation
     */
    struct HistoryItem {
        HistoryItem() : action(-1), observation(NULL) {}
        HistoryItem(int act, State* obs) : action(act), observation(obs) {}

        int action;
        State* observation;
    };

    //History()
    ~History();

    /*!
     * \brief add add an observation and action
     * \param action
     * \param obs
     */
    void add(int action, State* obs);

    /*!
     * \brief getLength get number of history items
     * \return
     */
    std::size_t getLength();

    /*!
     * \brief size size of history
     * \return
     */
    std::size_t size();

    /*!
     * \brief operator [] get item
     * \param t
     * \return
     */
    HistoryItem& operator[](int t);


    /*!
     * \brief getLast get last item
     * \return
     */
    HistoryItem& getLast();

    /*!
     * \brief truncate truncate history size
     * \param size
     */
    void truncate(std::size_t size);

protected:
    std::vector<HistoryItem> _historyVector;
};
}

#endif // HISTORY_H
