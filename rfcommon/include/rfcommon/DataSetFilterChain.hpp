#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class DataSet;
class DataSetFilter;
class DataSetFilterChainData;

extern template class RFCOMMON_TEMPLATE_API Reference<DataSetFilter>;
extern template class RFCOMMON_TEMPLATE_API SmallVector<Reference<DataSetFilter>, 8>;

class RFCOMMON_PUBLIC_API DataSetFilterChain
{
public:
    /*!
     * \brief Adds a new data set filter to the end of the chain. The chain
     * will incref the filter until it is removed again.
     * \return Returns the position in the chain, starting at 0.
     */
    int add(DataSetFilter* filter);

    /*!
     * \brief Removes a filter from the chain. The chain will decref the
     * filter.
     * \return Returns the position it was in the chain, starting at 0, or
     * returns -1 if the filter was not part of the chain.
     */
    int remove(DataSetFilter* filter);

    /*!
     * \brief Moves the filter one slot later in the chain (swaps the next
     * filter with this one). If the filter is already at the end then nothing
     * happens.
     * \return Returns the new position in the chain starting at 0, or returns
     * -1 if the filter is not part of the chain.
     */
    int moveLater(DataSetFilter* filter);

    /*!
     * \brief Moves the filter one slot earlier in the chain (swaps the previous
     * filter with this one). If the filter is already at the beginning of the
     * chain then nothing happens.
     * \return Returns the new position in the chain starting at 0, or returns
     * -1 if the filter is not part of the chain.
     */
    int moveEarlier(DataSetFilter* filter);

    /*!
     * \brief Returns the number of filters in the chain.
     */
    int filterCount() const;

    /*!
     * \brief Returns the filter at the specified position. Returns nullptr
     * if the index is out of bounds.
     */
    DataSetFilter* filter(int idx) const;

    /*!
     * \brief Applies every filter in the chain in succession to the input
     * data set.
     */
    DataSet* apply(const DataSet* ds);

private:
    SmallVector<Reference<DataSetFilter>, 8> filters_;
};

}
