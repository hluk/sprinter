#ifndef STDINTHREAD_H
#define STDINTHREAD_H

#include <QThread>
#include <QTimer>
#include <QMutex>
class QStandardItemModel;

class StdinThread : public QThread
{
    Q_OBJECT
public:
    explicit StdinThread(QObject *parent = 0);
    void run();
    void close();
    void setModel(QStandardItemModel *model) {m_model = model;}

private:
    QStandardItemModel *m_model;
    QMutex m_model_mutex;

    QStringList *m_new_items;
    QTimer m_new_items_t;
    QMutex m_new_items_mutex;

private slots:
    void addNewItems();
};

#endif // STDINTHREAD_H
