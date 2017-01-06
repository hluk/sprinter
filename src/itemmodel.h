/*
    Copyright (c) 2014, Lukas Holecek <hluk@email.cz>

    This file is part of Sprinter.

    Sprinter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Sprinter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sprinter.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QTimer>
#include <QVariant>

class QSize;
class QTimer;

class ItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemModel(QObject *parent = NULL);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    void fetchMore(const QModelIndex &parent = QModelIndex());
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setItemSize(QSize &size);

    const QStringList &items() const { return m_items; }

private:
    int m_count;
    QStringList m_items;
    QTimer m_timerFetch;
    QTimer m_timerUpdate;
    QVariant m_itemSize;

private slots:
    void updateItems();
    void readStdin();
};

#endif // ITEMMODEL_H
