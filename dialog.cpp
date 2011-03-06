#include "dialog.h"
#include "ui_dialog.h"
#include <QIODevice>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QDebug>
#include <cstdio>
#include "itemmodel.h"

Dialog::Dialog(QWidget *parent) :
        QDialog(parent,
                Qt::WindowStaysOnTopHint |
                /*Qt::X11BypassWindowManagerHint |*/
                Qt::FramelessWindowHint),
    ui(new Ui::Dialog),
    m_exit_code(1)
{
    ui->setupUi(this);

    /* model */
    m_model = new ItemModel(ui->listWidget);

    /* filtering */
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->listWidget->setModel(m_proxy);

    connect( ui->listWidget->selectionModel(),
             SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(itemSelected(QItemSelection,QItemSelection)) );
    connect( ui->lineEdit, SIGNAL(textEdited(QString)),
             this, SLOT(setFilter(QString)) );

    //thread()->setPriority(QThread::HighestPriority);

    setLabel("");

    /* completion */
    QCompleter *completer = new QCompleter(m_model, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEdit->setCompleter(completer);
}

Dialog::~Dialog()
{
    delete ui;

    exit(m_exit_code);
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
    ui->listWidget->setHorizontalScrollBarPolicy(
            enable ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff );
    ui->listWidget->setWrapping(enable);
}

void Dialog::setGridSize(int w, int h)
{
    QSize size(w, h);
    ui->listWidget->setGridSize(size);
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
        ui->listWidget->setCurrentIndex( list.at(0) );
    }
}

void Dialog::hideList(bool hide)
{
    ui->listWidget->setHidden(hide);
    ui->lineEdit->completer()->setCompletionMode(
            hide ? QCompleter::PopupCompletion : QCompleter::InlineCompletion);
    adjustSize();
}

void Dialog::itemSelected(const QItemSelection &,
                          const QItemSelection &)
{
    QLineEdit *edit = ui->lineEdit;
    if ( edit->hasFocus() )
        return;

    QModelIndexList indexes =
            ui->listWidget->selectionModel()->selectedIndexes();

    QStringList captions;
    foreach (QModelIndex index, indexes) {
        QVariant data = m_proxy->data(index);
        if ( data.isValid() )
            captions.append( data.toString() );
    }
    edit->setText( captions.join("\n") );
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    QString text;
    QLineEdit *edit = ui->lineEdit;
    ListWidget *view = ui->listWidget;
    QModelIndex index;
    int row;

    int key = e->key();

    if (e->modifiers() & Qt::ControlModifier) {
        /* CTRL */
        switch(key){
        case Qt::Key_L:
            edit->setFocus();
            edit->selectAll();
            break;
        default:
            break;
        }
    } else {
        switch(key){
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            text = edit->text();
            printf( text.toLocal8Bit().constData() );
            m_exit_code = 0;
            close();
            break;
        case Qt::Key_Tab:
            if ( ui->listWidget->isHidden() ) {
                edit->completer()->setCompletionPrefix( edit->text() );
                edit->completer()->complete();
            }
            break;
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            if ( view->isVisible() && view->currentIndex().row() == 0 ) {
                edit->setFocus();
                break;
            }
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            /* list is hidden - same behaviour as command line */
            /* TODO: restore original text when row == -1*/
            if ( ui->listWidget->isHidden() ) {
                row = ui->lineEdit->completer()->currentRow();
                if (key == Qt::Key_Down || key == Qt::Key_PageDown)
                    --row;
                else
                    ++row;
                ui->lineEdit->completer()->setCurrentRow(row);
                index = ui->lineEdit->completer()->currentIndex();
                edit->setText( m_model->data(index).toString() );
                break;
            }
            if ( edit->hasFocus() ) {
                row = 0;
            } else if (key == Qt::Key_Down || key == Qt::Key_PageDown) {
                row = view->currentIndex().row() + 1;
            } else {
                row = view->currentIndex().row() - 1;
            }
            index = m_proxy->index(row, 0);
            if ( index.isValid() ) {
                view->setCurrentIndex(index);
                edit->setText( m_proxy->data(index).toString() );
                view->setFocus();
            }
            break;
        case Qt::Key_Left:
            if ( !view->isWrapping() ) {
                edit->setCursorPosition(0);
                edit->setFocus();
            }
            break;
        case Qt::Key_Right:
            if ( !view->isWrapping() ) {
                edit->setFocus();
            }
            break;
        default:
            text = e->text();
            if ( !text.isEmpty() ) {
                text.prepend( edit->text() );
                edit->setText(text);
                edit->setFocus();
                setFilter(text);
            }
            break;
        }
    }
}
