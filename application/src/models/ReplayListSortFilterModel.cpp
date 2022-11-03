#include "application/models/ReplayListSortFilterModel.hpp"
#include "application/models/ReplayListModel.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListSortFilterModel::ReplayListSortFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

// ----------------------------------------------------------------------------
ReplayListSortFilterModel::~ReplayListSortFilterModel()
{}

// ----------------------------------------------------------------------------
void ReplayListSortFilterModel::setStage(const QString& stage)
{}

// ----------------------------------------------------------------------------
void ReplayListSortFilterModel::setFighterNames(const QStringList& fighterNames)
{}

// ----------------------------------------------------------------------------
void ReplayListSortFilterModel::setPlayerNames(const QStringList& playerNames)
{}

// ----------------------------------------------------------------------------
void ReplayListSortFilterModel::setGenericSearchTerms(const QStringList& terms)
{
    genericSearchTerms_ = terms;
    invalidateFilter();
}

// ----------------------------------------------------------------------------
bool ReplayListSortFilterModel::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    static int columnsToSearch[] = {
        ReplayListModel::P1,
        ReplayListModel::P2,
        ReplayListModel::P1Char,
        ReplayListModel::P2Char,
        ReplayListModel::SetFormat,
        ReplayListModel::Stage,
    };

    if (parent.isValid() == false)
        return true;

    auto rowContainsTerm = [this, &parent](int row, const QString& term) -> bool {
        for (auto col : columnsToSearch)
        {
            const QModelIndex idx = sourceModel()->index(row, col, parent);
            const QString data = sourceModel()->data(idx).toString();

            if (data.contains(term))
                return true;
        }
        return false;
    };

    for (const auto& term : genericSearchTerms_)
        if (rowContainsTerm(row, term) == false)
            return false;

    return true;
}

// ----------------------------------------------------------------------------
bool ReplayListSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (left.parent().isValid() == false || right.parent().isValid() == false)
        return false;

    int leftRow = left.row();
    int rightRow = right.row();
    int numCols = sourceModel()->columnCount();

    static int sortOrder[] = {
        ReplayListModel::P1,
        ReplayListModel::P2,
        ReplayListModel::P1Char,
        ReplayListModel::P2Char,
        ReplayListModel::SetFormat,
        ReplayListModel::SetNumber,
        ReplayListModel::GameNumber,
    };

    for (auto col : sortOrder)
    {
        const QModelIndex leftIdx = sourceModel()->index(leftRow, col, left.parent());
        const QModelIndex rightIdx = sourceModel()->index(rightRow, col, right.parent());

        switch (col)
        {
            case ReplayListModel::P1:
            case ReplayListModel::P2:
            case ReplayListModel::P1Char:
            case ReplayListModel::P2Char:
            case ReplayListModel::SetFormat: {
                QString a = sourceModel()->data(leftIdx).toString();
                QString b = sourceModel()->data(rightIdx).toString();
                int comp = QString::localeAwareCompare(a, b);
                if (comp != 0)
                    return comp < 0;
            } break;

            case ReplayListModel::SetNumber:
            case ReplayListModel::GameNumber: {
                int a = sourceModel()->data(leftIdx).toInt();
                int b = sourceModel()->data(rightIdx).toInt();
                if (a > b) return false;
                if (a < b) return true;
            } break;
        }
    }

    return false;
}

// ----------------------------------------------------------------------------
bool ReplayListSortFilterModel::filtersCleared() const
{
    return stage_.isEmpty() && fighterNames_.size() == 0 && playerNames_.size() == 0 && genericSearchTerms_.size() == 0;
}

}
