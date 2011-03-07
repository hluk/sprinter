#include "dialog.h"
#include "ui_dialog.h"
#include <QIODevice>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QProcess>
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
    m_output(NULL)
{
    ui->setupUi(this);

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

    connect( ui->listView->selectionModel(),
             SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(itemSelected(QItemSelection,QItemSelection)) );
    connect( ui->lineEdit, SIGNAL(textEdited(QString)),
             this, SLOT(textEdited(QString)) );

    //thread()->setPriority(QThread::HighestPriority);

    setLabel("");

    /* completion */
    QCompleter *completer = new QCompleter(m_model, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(20);
    ui->lineEdit->setCompleter(completer);
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
    setFilter(text);
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

void Dialog::setFilter(const QString &currentText)
{
    QString filter = currentText;

    /* filter items */
    /*
    filter.replace(' ', ".*");
    m_proxy->setFilterRegExp( QRegExp(filter, Qt::CaseInsensitive) );
    */
    filter.replace(' ', "*");
    m_proxy->setFilterWildcard(filter);

    /* select first item that starts with matched text */
    QModelIndexList list =
            m_proxy->match( m_proxy->index(0,0), Qt::DisplayRole, filter, 1,
                            Qt::MatchStartsWith);
    if ( !list.isEmpty() ) {
        ui->listView->setCurrentIndex( list.at(0) );
    }
}

void Dialog::hideList(bool hide)
{
    ui->listView->setHidden(hide);
    ui->lineEdit->completer()->setCompletionMode(
            hide ? QCompleter::PopupCompletion : QCompleter::InlineCompletion);
    adjustSize();
    setMaximumHeight(hide ? height() : QWIDGETSIZE_MAX);
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
    edit->setText( captions.join("\n") );
}

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() != QEvent::KeyPress )
        return false;

    QKeyEvent *e = (QKeyEvent *)event;
    QString text;
    QLineEdit *edit = ui->lineEdit;
    QListView *view = ui->listView;
    QModelIndex index;
    int row;

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
            if ( ui->listView->isHidden() ) {
                edit->completer()->setCompletionPrefix( edit->text() );
                edit->completer()->complete();
                return true;
            }
            break;
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            if ( obj == view && view->currentIndex().row() == 0 ) {
                edit->setFocus();
                return true;
            }
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            /* if list is hidden - same behaviour as command line */
            if ( ui->listView->isHidden() ) {
                row = ui->lineEdit->completer()->currentRow();
                if (key == Qt::Key_Down || key == Qt::Key_PageDown)
                    --row;
                else
                    ++row;
                if (row == -1) {
                    /* restore original text */
                    edit->setText(m_original_text);
                } else {
                    edit->completer()->setCompletionPrefix("");
                    edit->completer()->setCurrentRow(row);
                    index = edit->completer()->currentIndex();
                    edit->setText( m_model->data(index).toString() );
                }
                return true;
            } else if (key == Qt::Key_Down && obj == edit) {
                if ( !ui->listView->selectionModel()->hasSelection() )
                    ui->listView->setCurrentIndex( m_proxy->index(0,0) );
                ui->listView->setFocus();
                index = view->currentIndex();
                edit->setText( m_proxy->data(index).toString() );
                return true;
            }
            break;
        case Qt::Key_Left:
            if ( obj == view && !view->isWrapping() ) {
                edit->setCursorPosition(0);
                edit->setFocus();
                return true;
            }
            break;
        case Qt::Key_Right:
            if ( obj == view && !view->isWrapping() ) {
                edit->setFocus();
                return true;
            }
            break;
        default:
            if (obj == view) {
                text = e->text();
                if ( !text.isEmpty() ) {
                    text.prepend( edit->text() );
                    edit->setText(text);
                    edit->setFocus();
                    setFilter(text);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}
