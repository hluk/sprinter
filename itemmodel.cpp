#include "itemmodel.h"
#include <QStringList>
#include <QTimer>
#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QFileIconProvider>
#include <QDebug>
#include <cstdio>

ItemModel::ItemModel(QObject *parent) :
    QAbstractListModel(parent),
    m_count(0),
    m_item_size(NULL)
{
    m_items = new QStringList();

    /* fetch lines from stdin */
    m_fetch_t = new QTimer(this);
    m_fetch_t->setSingleShot(true);
    m_fetch_t->setInterval(0);
    connect( m_fetch_t, SIGNAL(timeout()),
             this, SLOT(readStdin()) );
    m_fetch_t->start();

    /* update list in intervals */
    m_update_t = new QTimer(this);
    m_update_t->setSingleShot(true);
    m_update_t->setInterval(500);
    connect( m_update_t, SIGNAL(timeout()),
             this, SLOT(updateItems()) );
    m_update_t->start();
}

ItemModel::~ItemModel()
{
    m_fetch_t->stop();
    m_update_t->stop();
    if (m_item_size)
        delete m_item_size;

    delete m_items;
}

int ItemModel::rowCount(const QModelIndex &) const
{
    return m_count;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    static QFileIconProvider icon_provider;
    int row = index.row();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_items->at(row);
    } else if (role == Qt::DecorationRole) {
        QFileInfo info( m_items->at(row) );
        if ( info.exists() ) {
            QIcon icon = icon_provider.icon(info);
            return icon;
        }
    } else if (role == Qt::SizeHintRole && m_item_size){
        return *m_item_size;
    }

    return QVariant();
}

void ItemModel::setItemSize(QSize &size)
{
    if (m_item_size)
        delete m_item_size;
    m_item_size = new QSize(size);
}

bool ItemModel::canFetchMore(const QModelIndex &) const
{
    return ( m_count != m_items->size() ) ||
           ( !ferror(stdin) && !feof(stdin) );
}

void ItemModel::fetchMore(const QModelIndex &)
{
    int rows = m_items->size();
    if (m_count == rows) return;

    beginInsertRows(QModelIndex(), m_count, rows-1);
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
    static char buffer[256];
    static QByteArray lines;

    struct timeval tv;
    fd_set fds;

    if ( !ferror(stdin) && !feof(stdin) )
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        if ( (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv)) <= 0 ) {
            m_fetch_t->start();
            return;
        }

        if ( fgets(buffer, 256, stdin) ) {
            lines.append(buffer);

            int i;
            while ( (i = lines.indexOf('\n')) >= 0 ) {
                QByteArray *line = new QByteArray( lines.left(i) );
                lines.remove(0, i+1);

                if (i==0) continue;

                // TODO: remove \r if newline is \r\n
                /*
                if (line->at(i-1) == '\r')
                    line->remove(i-1, 1);
                */

                m_items->append( QString::fromLocal8Bit(line->constData()) );
            }
        }

        m_fetch_t->start();
    }
}

void ItemModel::updateItems()
{
    if ( canFetchMore() ) {
        fetchMore();
        m_update_t->start();
    }
}
