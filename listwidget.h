#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListView>

class ListWidget : public QListView
{
    Q_OBJECT
public:
    explicit ListWidget(QWidget *parent = 0);

protected:
    bool event(QEvent *e);
};

#endif // LISTWIDGET_H
