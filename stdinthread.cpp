#include "stdinthread.h"
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>

StdinThread::StdinThread(QObject *parent) :
    QThread(parent), m_model(NULL)
{
}

void StdinThread::addNewItems()
{
    QStringList *new_items = NULL;

    m_new_items_t.stop();

    m_new_items_mutex.lock();
    if ( m_new_items->size() ) {
        new_items = m_new_items;
        m_new_items = new QStringList();
    }
    m_new_items_mutex.unlock();

    if (new_items) {
        // TODO: insert multiple rows at once (or don't update view)
        for( int i = 0; i < new_items->count(); i++)
            m_model->appendRow( new QStandardItem(new_items->at(i)) );
        delete new_items;
    }

    m_new_items_t.start();
}

void StdinThread::run()
{
    QFile in;
    in.open(stdin, QFile::ReadOnly);

    m_new_items = new QStringList();
    m_new_items_t.setInterval(100);
    m_new_items_t.setSingleShot(true);
    connect( &m_new_items_t, SIGNAL(timeout()),
             this, SLOT(addNewItems()) );
    m_new_items_t.start();

    while( !in.atEnd() && m_model )
    {
        QByteArray line = in.readLine();
        int size = line.size();

        // empty line?
        if(size <= 1) continue;

        // remove newline char
        line.remove(size-1, 1);
        // remove \r if newline is \r\n
        if (line.at(size-2) == '\r')
            line.remove(size-2, 1);

        m_new_items_mutex.lock();
        m_new_items->append( QString::fromLocal8Bit(line.constData()) );
        m_new_items_mutex.unlock();
    }

    addNewItems();
    m_new_items_t.stop();
    delete m_new_items;

    in.close();
}
