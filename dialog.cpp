#include "dialog.h"
#include "ui_dialog.h"
#include <QIODevice>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <cstdio>
#include "itemmodel.h"

Dialog::Dialog(QWidget *parent) :
        QDialog(parent,
                Qt::WindowStaysOnTopHint |
                /*Qt::X11BypassWindowManagerHint |*/
                Qt::FramelessWindowHint),
    ui(new Ui::Dialog),
    m_exit_code(1),
    m_strict(false),
    m_output(NULL),
    m_hide_list(false)
{
    ui->setupUi(this);

    /* filter events */
    ui->lineEdit->installEventFilter(this);
    ui->listView->installEventFilter(this);

    /* model */
    m_model = new ItemModel(ui->listView);

    /* filtering */
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->listView->setModel(m_proxy);

    /* signals & slots */
    connect( ui->listView->selectionModel(),
             SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(itemSelected(QItemSelection,QItemSelection)) );
    connect( ui->lineEdit, SIGNAL(textEdited(QString)),
             this, SLOT(textEdited(QString)) );

    /* hide label by default */
    setLabel("");

    /* completion */
    QCompleter *completer = new QCompleter(m_model, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEdit->setCompleter(completer);

    /* focus edit line */
    ui->lineEdit->setFocus();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::closeEvent(QCloseEvent *)
{
    if (!m_exit_code && m_output)
        m_output->append( ui->lineEdit->text().split('\n') );
    qApp->exit(m_exit_code);
}

void Dialog::textEdited(const QString &text)
{
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
    QModelIndexList list =
            m_proxy->match( m_proxy->index(0,0), Qt::DisplayRole, filter, 1,
                            Qt::MatchStartsWith);
    if ( !list.isEmpty() ) {
        ui->listView->setCurrentIndex( list.at(0) );
    }
}

void Dialog::updateFilter(int interval)
{
    if ( m_hide_list )
        return;

    static QTimer *update_t = NULL;
    if (interval <= 0) {
        setFilter( ui->lineEdit->text() );
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
    /* filter */
    QString filter = ui->lineEdit->text().left(
            ui->lineEdit->cursorPosition() );
    m_proxy->setFilterFixedString(filter);
    if ( !m_proxy->rowCount() )
        return;

    /* restore dialog size */
    setMaximumHeight(QWIDGETSIZE_MAX);
    resize( width(), m_height );

    /* show and focus list */
    QListView *view = ui->listView;
    view->setCurrentIndex( m_proxy->index(0,0) );
    view->show();
    view->setFocus();
    itemSelected( QItemSelection(), QItemSelection() );
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
        QVariant data = m_proxy->data(index);
        if ( data.isValid() )
            captions.append( data.toString() );
    }

    int i = edit->selectionStart();
    if (i<0)
        i = qMax( 0, edit->cursorPosition() );
    QString text = edit->text().left(i);
    QString text2 = captions.join("\n");
    edit->setText(text2);
    if ( text2.startsWith(text, Qt::CaseInsensitive) )
        edit->setSelection( text.length(), text2.length() );
}

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{
    QLineEdit *edit = ui->lineEdit;
    QListView *view = ui->listView;

    if ( event->type() == QEvent::FocusIn ) {
        if (obj == edit && m_hide_list && view->isVisible() ) {
            hideList(true);
        }
        return false;
    } else if ( event->type() != QEvent::KeyPress ) {
        return false;
    }

    QKeyEvent *e = (QKeyEvent *)event;
    QString text;
    QModelIndex index;

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
            /* submit text */
            text = edit->text();
            if (m_strict && m_model->items()->indexOf(text) == -1 ) {
                return true;
            }
            if ( !m_output ) {
                /* print to stdout */
                printf( text.toLocal8Bit().constData() );
            }
            m_exit_code = 0;
            close();
            return true;
        case Qt::Key_Tab:
            if (obj == edit) {
                if ( edit->selectionStart() >= 0 ) {
                    edit->completer()->setCompletionPrefix( edit->text() );
                    edit->setCursorPosition( edit->text().length() );
                    return true;
                } else if (m_hide_list) {
                    popList();
                    return true;
                }
            }
            break;
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            if ( obj == view && view->currentIndex().row() == 0 ) {
                edit->setText(m_original_text);
                edit->setFocus();
                return true;
            }
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            if (m_hide_list && obj == edit) {
                popList();
                return true;
            } else if (key == Qt::Key_Down && obj == edit) {
                if ( !view->selectionModel()->hasSelection() )
                    view->setCurrentIndex( m_proxy->index(0,0) );
                index = view->currentIndex();
                if ( index.isValid() ) {
                    view->setFocus();
                    edit->setText( m_proxy->data(index).toString() );
                }
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
                    updateFilter(300);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}
