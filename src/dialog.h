/*
    Copyright (c) 2014, Lukas Holecek <hluk@email.cz>

    This file is part of Sprinter.

    Sprinter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Sprinter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sprinter.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    void saveOutput(QList<QByteArray> *output) {m_output = output;}
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
    QList<QByteArray> *m_output;
    bool m_hide_list;
    int m_height;

    QString unselectedText() const;

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
