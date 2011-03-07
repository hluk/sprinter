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
    void hideList(bool hide);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::Dialog *ui;
    ItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
    QString m_original_text;
    int m_exit_code;
    bool m_strict;
    QStringList *m_output;

protected:
    void closeEvent(QCloseEvent *);

public slots:
    void setFilter(const QString &currentText);
    void itemSelected(const QItemSelection &selected,
                      const QItemSelection &deselected);

private slots:
    void textEdited(const QString &text);
};

#endif // DIALOG_H
