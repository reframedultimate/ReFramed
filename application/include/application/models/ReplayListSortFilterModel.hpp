#pragma once

#include <QSortFilterProxyModel>
#include <QDate>

namespace rfapp {

class ReplayListSortFilterModel : public QSortFilterProxyModel
{
public:
    explicit ReplayListSortFilterModel(QObject* parent=nullptr);
    ~ReplayListSortFilterModel();

    const QString& stage() const { return stage_; }
    void setStage(const QString& stage);

    const QStringList& fighterNames() const { return fighterNames_; }
    void setFighterNames(const QStringList& fighterNames);

    const QStringList& playerNames() const { return playerNames_; }
    void setPlayerNames(const QStringList& playerNames);

    const QStringList& genericSearchTerms() const { return genericSearchTerms_; }
    void setGenericSearchTerms(const QStringList& terms);

    bool filtersCleared() const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString stage_;
    QStringList fighterNames_;
    QStringList playerNames_;
    QStringList genericSearchTerms_;
};

}
