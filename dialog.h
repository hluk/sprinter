#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QSortFilterProxyModel;
class QModelIndex;
class QStandardItemModel;
class StdinThread;

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    StdinThread *m_stdin_thread;
    QStandardItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
    int m_exit_code;

protected:
    void keyPressEvent(QKeyEvent *e);

public slots:
    void setFilter(const QString &currentText);
    void itemSelected(const QModelIndex &index, const QModelIndex &prev);
};

#endif // DIALOG_H
