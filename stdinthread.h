#ifndef STDINTHREAD_H
#define STDINTHREAD_H

#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QLinkedList>
class QStandardItemModel;

class StdinThread : public QThread
{
    Q_OBJECT
public:
    explicit StdinThread(QStandardItemModel *model, QObject *parent = 0);
    void run();
    void close() {m_close = true;}

private:
    QStandardItemModel *m_model;
    volatile bool m_close;

    QLinkedList<QByteArray*> *m_new_items;
    QTimer m_new_items_t;
    QMutex m_new_items_mutex;

private slots:
    void addNewItems();
};

#endif // STDINTHREAD_H
