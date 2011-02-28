#include "listwidget.h"
#include <QEvent>
#include <QKeyEvent>

ListWidget::ListWidget(QWidget *parent) :
    QListView(parent)
{
    /* list in batch mode flickers
    setLayoutMode(Batched);
    setBatchSize(20);
    */
    setUniformItemSizes(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTextElideMode(Qt::ElideMiddle);
}

bool ListWidget::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *key_e = (QKeyEvent *)e;
        if ( !key_e->text().isEmpty() ) {
            return parent()->event(e);
        }
    }

    return QListView::event(e);
}
