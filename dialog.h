#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QSortFilterProxyModel;
class QModelIndex;
class ItemModel;
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
    void setStrict(bool enable) {m_strict = enable;}
    void saveOutput(QStringList *output) {m_output = output;}
    void sortList();
    void hideList(bool hide);
    void popList();

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::Dialog *ui;
    ItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
    QString m_original_text;
    int m_exit_code;
    bool m_strict;
    QStringList *m_output;
    bool m_hide_list;
    int m_height;

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void setFilter(const QString &currentText);
    void itemSelected(const QItemSelection &selected,
                      const QItemSelection &deselected);

private slots:
    void textEdited(const QString &text);
    void updateFilter(int interval = 0);
    void firstRowInserted();
    void submit();
    void submitCurrentItem(const QModelIndex &index);
};

#endif // DIALOG_H
