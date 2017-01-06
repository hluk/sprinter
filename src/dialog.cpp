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

#include "dialog.h"
#include "ui_dialog.h"

#include "itemmodel.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QTimer>

#include <cstdio>

Dialog::Dialog(QWidget *parent) :
        QDialog(parent,
                Qt::WindowStaysOnTopHint
                /*Qt::X11BypassWindowManagerHint |*/
                /*Qt::FramelessWindowHint*/),
    ui(new Ui::Dialog),
    m_exit_code(1),
    m_strict(false),
    m_output(NULL),
    m_hide_list(false)
{
    ui->setupUi(this);

    QLineEdit *const edit = ui->lineEdit;
    QListView *const view = ui->listView;

    /* filter events */
    edit->installEventFilter(this);
    view->installEventFilter(this);

    /* model */
    m_model = new ItemModel(view);

    /* filtering */
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    view->setModel(m_proxy);

    /* signals & slots */
    connect( view, SIGNAL(activated(QModelIndex)),
             this, SLOT(submitCurrentItem(QModelIndex)) );
    connect( view->selectionModel(),
             SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(itemSelected(QItemSelection,QItemSelection)) );
    connect( edit, SIGNAL(textEdited(QString)),
             this, SLOT(textEdited(QString)) );
    connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SLOT(firstRowInserted()),
             Qt::DirectConnection );

    /* hide label by default */
    setLabel("");

    /* completion */
    QCompleter *completer = new QCompleter(m_model, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    edit->setCompleter(completer);

    /* focus edit line */
    edit->setFocus();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::closeEvent(QCloseEvent *)
{
    if (!m_exit_code && m_output) {
        int a = 0, b = 0;
        QString text = ui->lineEdit->text();

        while ( (b = text.indexOf('\n', a)) > 0 ) {
            m_output->append( text.mid(a, b-a).toLocal8Bit() );
            a = b+1;
        }
        if ( a != text.size() )
            m_output->append( text.mid(a).toLocal8Bit() );
    }
    qApp->exit(m_exit_code);
}

void Dialog::textEdited(const QString &text)
{
    if (!m_hide_list)
        updateFilter(300);
    m_original_text = text;
}

void Dialog::setLabel(const QString &text)
{
    if (text.isEmpty()) {
        ui->label->hide();
    } else {
        ui->label->setText(text);
        ui->label->show();
    }
}

void Dialog::setWrapping(bool enable)
{
    ui->listView->setHorizontalScrollBarPolicy(
            enable ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff );
    ui->listView->setWrapping(enable);
}

void Dialog::setGridSize(int w, int h)
{
    QSize size(w, h);
    ui->listView->setGridSize(size);
    ui->listView->setIconSize( QSize(h,h) );
    m_model->setItemSize(size);
}

void Dialog::sortList()
{
    m_proxy->sort(0);
}

void Dialog::setFilter(const QString &currentText)
{
    QString filter = currentText;

    /* filter items */
    /*
    filter.replace(' ', ".*");
    m_proxy->setFilterRegExp( QRegExp(filter, Qt::CaseInsensitive) );
    */
    m_proxy->setFilterWildcard(filter);

    /* select first item that starts with matched text */
    QModelIndexList list = m_proxy->match( m_proxy->index(0,0), Qt::DisplayRole,
                                           filter, 1, Qt::MatchStartsWith );
    if ( !list.isEmpty() ) {
        ui->listView->setCurrentIndex( list.at(0) );
    }
}

void Dialog::firstRowInserted()
{
    if ( m_original_text.isEmpty() ) {
        QModelIndex index = m_proxy->index(0,0);
        if ( index.isValid() ) {
            ui->listView->setCurrentIndex(index);
            ui->lineEdit->setText( index.data().toString() );
            ui->lineEdit->selectAll();
        }
    }
    disconnect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(firstRowInserted()) );
}

QString Dialog::unselectedText() const
{
    QLineEdit *const edit = ui->lineEdit;
    int i = edit->selectionStart();
    if (i<0)
        i = qMax( 0, edit->cursorPosition() );
    return edit->text().left(i);
}

void Dialog::updateFilter(int interval)
{
    static QTimer *update_t = NULL;

    if (interval <= 0) {
        if ( !ui->lineEdit->hasFocus() )
            return;
        QString filter = unselectedText();
        setFilter(filter);
        return;
    }

    if (!update_t) {
        update_t = new QTimer();
        update_t->setSingleShot(true);
        connect( update_t, SIGNAL(timeout()),
                 this, SLOT(updateFilter()) );
    }

    if ( !update_t->isActive() ) {
        update_t->start(interval);
    }
}

void Dialog::hideList(bool hide)
{
    // TODO: remeber dialog size set by user - restore height if list shown
    int w = width();
    m_height = height();
    m_hide_list = hide;
    ui->listView->setHidden(hide);

    /* resize automatically */
    resize(0,0);
    adjustSize();

    /* restore width */
    resize( w, height() );

    /* set maximum height - fixed if list hidden */
    setMaximumHeight(hide ? height() : QWIDGETSIZE_MAX);
}

void Dialog::popList()
{
    QLineEdit *const edit = ui->lineEdit;
    QListView *const view = ui->listView;
    QModelIndex index;

    /* restore dialog size */
    if (m_hide_list) {
        setMaximumHeight(QWIDGETSIZE_MAX);
        resize( width(), m_height );
    }

    if (m_hide_list)
        updateFilter();

    /* show and focus list */
    index = view->selectionModel()->hasSelection() ?
                view->currentIndex() : m_proxy->index(0,0);
    if ( index.isValid() ) {
        view->setCurrentIndex(index);
        QString text = index.data().toString();

        if ( text == edit->text() ) {
            index = m_proxy->index( index.row()+1, 0 );
            if ( index.isValid() ) {
                view->setCurrentIndex(index);
                text = index.data().toString();
            }
        }

        view->show();
        view->setFocus();
        itemSelected( QItemSelection(), QItemSelection() );
    }
}

void Dialog::itemSelected(const QItemSelection &,
                          const QItemSelection &)
{
    QLineEdit *edit = ui->lineEdit;
    if ( edit->hasFocus() )
        return;

    QModelIndexList indexes =
            ui->listView->selectionModel()->selectedIndexes();

    QStringList captions;
    foreach (QModelIndex index, indexes) {
        QVariant data = index.data();
        if ( data.isValid() )
            captions.append( data.toString() );
    }

    QString text2 = captions.join("\n");
    edit->setText(text2);
    if ( text2.startsWith(m_original_text, Qt::CaseInsensitive) )
        edit->setSelection( m_original_text.length(), text2.length() );
}

void Dialog::submit()
{
    QLineEdit *const edit = ui->lineEdit;
    QString text;

    if ( edit->selectionStart() >= 0 )
        text = edit->completer()->currentCompletion();

    if ( text.isEmpty() || text.compare(edit->text(), Qt::CaseInsensitive) )
        text = edit->text();

    if (m_strict && m_model->items().indexOf(text) == -1 )
        return;

    /* print to stdout */
    if ( !m_output ) {
        printf( "%s", text.toLocal8Bit().constData() );
    }

    m_exit_code = 0;
    close();
}

void Dialog::submitCurrentItem(const QModelIndex &index)
{
    if ( index.isValid() ) {
        ui->lineEdit->setText( index.data().toString() );
        submit();
    }
}

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::FocusIn ) {
        if (obj == ui->lineEdit && m_hide_list && ui->listView->isVisible() ) {
            hideList(true);
        }
        return false;
    } else if ( event->type() != QEvent::KeyPress ) {
        return false;
    }

    QLineEdit *const edit = ui->lineEdit;
    QListView *const view = ui->listView;
    QKeyEvent *e = (QKeyEvent *)event;
    QString text;

    int key = e->key();

    if (e->modifiers() & Qt::ControlModifier) {
        /* CTRL */
        switch(key){
        case Qt::Key_L:
            edit->setFocus();
            edit->selectAll();
            return true;
        default:
            break;
        }
    } else {
        switch(key){
        case Qt::Key_Escape:
            close();
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            submit();
            return true;
        case Qt::Key_Tab:
            if (obj == edit) {
                if ( edit->selectionStart() >= 0 ) {
                    text = edit->completer()->currentCompletion();
                    if ( !text.isEmpty() )
                        edit->setText(text);
                } else {
                    popList();
                }
                return true;
            }
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            if (obj == edit) {
                popList();
                return true;
            }
            break;
        case Qt::Key_Left:
            if ( obj == view && !view->isWrapping() ) {
                edit->setFocus();
                edit->event(event);
                return true;
            }
            break;
        case Qt::Key_Right:
            if ( obj == view && !view->isWrapping() ) {
                edit->setFocus();
                edit->event(event);
                return true;
            }
            break;
        default:
            if (obj == view) {
                text = e->text();
                if ( !text.isEmpty() ) {
                    edit->setFocus();
                    edit->event(event);
                    if (!m_hide_list)
                        updateFilter(300);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    QLineEdit *const edit = ui->lineEdit;
    QListView *const view = ui->listView;

    if ( view->hasFocus() ) {
        if (key == Qt::Key_Up || key == Qt::Key_PageUp) {
            /* if selection is at top of list (or wrapped column in list)
             * and user wants to move up then select lineedit
             */
            if ( m_original_text != unselectedText() )
                edit->setText(m_original_text);
            edit->setFocus();
            event->accept();
        } else if (key == Qt::Key_Down || key == Qt::Key_PageDown) {
            // select next item (if wrapping enabled)
            QModelIndex index = view->currentIndex();
            if ( index.isValid() ) {
                index = m_proxy->index(index.row()+1, 0);
                if ( index.isValid() )
                    view->setCurrentIndex(index);
            }
            event->accept();
        }
    }
}
