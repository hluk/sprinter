#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractListModel>
class QStringList;
class QTimer;

class ItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemModel(QObject *parent = 0);
    ~ItemModel();

    // needs to be implemented
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    void fetchMore(const QModelIndex &parent = QModelIndex());
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    int m_count;
    QStringList *m_items;
    QTimer *m_fetch_t;
    QTimer *m_update_t;

signals:

public slots:

private slots:
    void readStdin();
    void updateItems();

};

#endif // ITEMMODEL_H
