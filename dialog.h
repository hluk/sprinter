#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QSortFilterProxyModel;
class QModelIndex;
class ItemModel;
class StdinThread;
class QItemSelection;

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    void setLabel(const QString &text);
    void setWrapping(bool enable);
    void setGridSize(int w, int h);

private:
    Ui::Dialog *ui;
    StdinThread *m_stdin_thread;
    ItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
    int m_exit_code;

protected:
    void keyPressEvent(QKeyEvent *e);

public slots:
    void setFilter(const QString &currentText);
    void itemSelected(const QItemSelection &selected,
                      const QItemSelection &deselected);
};

#endif // DIALOG_H
