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

#include "itemmodel.h"

#include <QApplication>
#include <QFileIconProvider>
#include <QFont>
#include <QPalette>
#include <QStringList>

#include <cstdio>
#include <unistd.h>

static const int stdin_batch_size = 250;

static void initSingleShotTimer(
        QTimer *timer, int msecs, const QObject *receiver, const char *slot)
{
    timer->setSingleShot(true);
    timer->setInterval(msecs);
    QObject::connect(timer, SIGNAL(timeout()), receiver, slot);
    timer->start();
}

ItemModel::ItemModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_count(0)
{
    /* no buffer for stdin */
    setbuf(stdin, NULL);

    /* fetch lines from stdin - doesn't block application */
    initSingleShotTimer(&m_timerFetch, 0, this, SLOT(readStdin()));
    /* update list in intervals */
    initSingleShotTimer(&m_timerUpdate, 500, this, SLOT(updateItems()));
}

int ItemModel::rowCount(const QModelIndex &) const
{
    return m_count;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    static QFileIconProvider icon_provider;
    int row = index.row();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return m_items.at(row);

    if (role == Qt::SizeHintRole)
        return m_itemSize;

    if (role == Qt::DecorationRole) {
        QFileInfo info( m_items.at(row) );
        if ( info.exists() ) {
            QIcon icon = icon_provider.icon(info);
            return icon;
        }
    }

    return QVariant();
}

void ItemModel::setItemSize(QSize &size)
{
    m_itemSize = size;
}

bool ItemModel::canFetchMore(const QModelIndex &) const
{
    return ( m_count != m_items.size() );
}

void ItemModel::fetchMore(const QModelIndex &)
{
    int rows = m_items.size();
    if (m_count == rows) return;

    beginInsertRows(QModelIndex(), m_count, rows - 1);
    m_count = rows;
    endInsertRows();
}

Qt::ItemFlags ItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index);
}

void ItemModel::readStdin()
{
    static char buffer[BUFSIZ];
    static QByteArray line;
    static struct timeval stdin_tv = {0,0};
    fd_set stdin_fds;

    /* disable stdin buffering (otherwise select waits on new line) */
    setbuf(stdin, NULL);

    /*
     * interrupt after reading at most N lines and
     * resume after processing pending events in event loop
     */
    for( int i = 0; i < stdin_batch_size; ++i ) {
        /* set stdin */
        FD_ZERO(&stdin_fds);
        FD_SET(STDIN_FILENO, &stdin_fds);

        /* check if data available */
        if ( select(STDIN_FILENO + 1, &stdin_fds, NULL, NULL, &stdin_tv) <= 0 )
            break;

        /* read data */
        if ( fgets(buffer, BUFSIZ, stdin) ) {
            line.append(buffer);
            /* each line is one item */
            if ( line.endsWith('\n') ) {
                line.resize( line.size()-1 );
                m_items.append( QString::fromLocal8Bit(line.constData()) );
                line.clear();
            }
        } else {
            break;
        }
    }

    if ( ferror(stdin) )
        perror( tr("Error reading stdin!").toLocal8Bit().constData() );
    else if ( !feof(stdin) )
        m_timerFetch.start();
}

void ItemModel::updateItems()
{
    if ( canFetchMore() )
        fetchMore();
    if ( !ferror(stdin) && !feof(stdin) )
        m_timerUpdate.start();
}
