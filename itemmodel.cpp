#include "itemmodel.h"
#include <QStringList>
#include <QTimer>
#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QFileIconProvider>
#include <QDebug>
#include <cstdio>

static struct timeval stdin_tv;
static fd_set stdin_fds;

ItemModel::ItemModel(QObject *parent) :
    QAbstractListModel(parent),
    m_count(0),
    m_item_size(NULL)
{
    m_items = new QStringList();

    /* set stdin */
    stdin_tv.tv_sec = 0;
    stdin_tv.tv_usec = 0;
    FD_ZERO(&stdin_fds);
    FD_SET(STDIN_FILENO, &stdin_fds);
    setbuf(stdin, NULL);

    /* fetch lines from stdin - doesn't block application */
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
    return ( m_count != m_items->size() );
}

void ItemModel::fetchMore(const QModelIndex &)
{
    int rows = m_items->size();
    if (m_count == rows) return;

    beginInsertRows(QModelIndex(), m_count, rows-1);
    m_count = rows;
    qDebug()<<rows;
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

    if ( !ferror(stdin) && !feof(stdin) )
    {
        if ( select(STDIN_FILENO+1, &stdin_fds, NULL, NULL, &stdin_tv) > 0 &&
             fgets(buffer, BUFSIZ, stdin) ) {
            line.append(buffer);
            if ( line.endsWith('\n') ) {
                line.resize( line.size()-1 );
                m_items->append( QString::fromLocal8Bit(line.constData()) );
                line.clear();
            }
        }

        m_fetch_t->start();
    }
}

void ItemModel::updateItems()
{
    if ( canFetchMore() )
        fetchMore();
    if ( !ferror(stdin) && !feof(stdin) )
        m_update_t->start();
}
