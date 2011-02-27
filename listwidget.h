#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListWidget>

class ListWidget : public QListView
{
    Q_OBJECT
public:
    explicit ListWidget(QWidget *parent = 0);

protected:
    bool event(QEvent *e);

signals:

public slots:

};

#endif // LISTWIDGET_H
