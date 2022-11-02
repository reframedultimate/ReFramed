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
void ReplayListSortFilterModel::setFilterMinimumDate(QDate date)
{}

// ----------------------------------------------------------------------------
void ReplayListSortFilterModel::setFilterMaximumDate(QDate date)
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
{}

// ----------------------------------------------------------------------------
bool ReplayListSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
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
bool ReplayListSortFilterModel::dateInRange(QDate date) const
{
    return true;
}

}
