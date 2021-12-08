#pragma once

#include "application/models/CategoryType.hpp"
#include <QString>

namespace rfapp {

class CategoryListener
{
public:
    /*!
     * \brief When the user clicks on a top-level item or one of their child
     * items. The main window will want to switch the current widget in response
     * to a category change. This is only called when the model switches between
     * different top-level categories, and not when switching between two
     * items in the same category.
     */
    virtual void onCategorySelected(CategoryType category) = 0;

    virtual void onCategoryItemSelected(CategoryType category, const QString& name) = 0;
};

}
