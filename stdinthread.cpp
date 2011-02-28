#include "stdinthread.h"
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>

StdinThread::StdinThread(QStandardItemModel *model, QObject *parent) :
    QThread(parent), m_model(model), m_close(false)
{
}

void StdinThread::addNewItems()
{
    QLinkedList<QByteArray*> *new_items = NULL;

    m_new_items_mutex.lock();
    if ( !m_new_items->isEmpty() ) {
        new_items = m_new_items;
        m_new_items = new QLinkedList<QByteArray*>();
    }
    m_new_items_mutex.unlock();

    if (new_items) {
        /*
        QLinkedList<QByteArray*>::ConstIterator it;
        for (it = new_items->constBegin(); it != new_items->constEnd(); ++it) {
        */
        while( !new_items->isEmpty() ) {
            QByteArray *data = new_items->takeFirst();
            m_model->appendRow( new QStandardItem(QString::fromLocal8Bit(data->constData())) );
            delete data;
        }
        delete new_items;
    }

    m_new_items_t.start();
}

void StdinThread::run()
{
    struct timeval tv;
    fd_set fds;

    m_new_items = new QLinkedList<QByteArray*>();
    m_new_items_t.setInterval(200);
    m_new_items_t.setSingleShot(true);
    connect( &m_new_items_t, SIGNAL(timeout()),
             this, SLOT(addNewItems()) );
    m_new_items_t.start();

    char buffer[256];
    QByteArray *lines = new QByteArray;

    while( !m_close && !ferror(stdin) && !feof(stdin) )
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        if ( (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv)) <= 0 )
            continue;
        if ( fgets(buffer, 256, stdin) ) {
            lines->append(buffer);

            int i;
            while ( (i = lines->indexOf('\n')) >= 0 ) {
                QByteArray *line = new QByteArray( lines->left(i) );
                lines->remove(0, i+1);

                if (i==0) continue;

                // TODO: remove \r if newline is \r\n
                /*
                if (line->at(i-1) == '\r')
                    line->remove(i-1, 1);
                */

                m_new_items_mutex.lock();
                m_new_items->append(line);
                m_new_items_mutex.unlock();
            }
            if (m_new_items->size() > 4000) {
                m_new_items_t.stop();
                addNewItems();
            }
            usleep(250);
        } else {
            if ( !lines->isEmpty() ) {
                m_new_items_mutex.lock();
                m_new_items->append(lines);
                m_new_items_mutex.unlock();
            }
            break;
        }
    }

    m_new_items_t.stop();
    addNewItems();
    m_new_items_t.stop();
    delete m_new_items;
}
