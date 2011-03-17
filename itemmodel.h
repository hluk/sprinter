#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractListModel>

class QStringList;
class QTimer;
class QSize;

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

    void setItemSize(QSize &size);

    const QStringList *items() const {return m_items;}

private:
    int m_count;
    QStringList *m_items;
    QTimer *m_fetch_t;
    QTimer *m_update_t;
    QSize *m_item_size;

signals:

public slots:
    void updateItems();

private slots:
    void readStdin();

};

#endif // ITEMMODEL_H
