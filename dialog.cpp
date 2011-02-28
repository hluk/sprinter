#include "dialog.h"
#include "ui_dialog.h"
#include <QIODevice>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "stdinthread.h"
#include <cstdio>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_exit_code(1)
{
    ui->setupUi(this);

    /* model */
    m_model = new QStandardItemModel(ui->listWidget);

    /* filtering */
    m_proxy = new QSortFilterProxyModel(this);
    //m_proxy->thread()->setPriority(QThread::IdlePriority);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->listWidget->setModel(m_proxy);

    connect( ui->listWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(itemSelected(QModelIndex,QModelIndex)) );
    connect( ui->lineEdit, SIGNAL(textEdited(QString)),
             this, SLOT(setFilter(QString)) );

    thread()->setPriority(QThread::HighestPriority);

    /* read stdin (non-blocking) */
    m_stdin_thread = new StdinThread(m_model, this);
    m_stdin_thread->start();
    m_stdin_thread->setPriority(QThread::IdlePriority);
}

Dialog::~Dialog()
{
    m_stdin_thread->close();
    m_stdin_thread->quit();
    m_stdin_thread->wait(500);
    m_stdin_thread->terminate();
    delete ui;

    exit(m_exit_code);
}

void Dialog::setFilter(const QString &currentText)
{
    QString filter = currentText;
    /*
    filter.replace(' ', ".*");
    m_proxy->setFilterRegExp( QRegExp(filter, Qt::CaseInsensitive) );
    */
    filter.replace(' ', "*");
    m_proxy->setFilterWildcard(filter);
}

void Dialog::itemSelected(const QModelIndex &index, const QModelIndex &)
{
    QLineEdit *edit = ui->lineEdit;
    QStandardItem *item = m_model->itemFromIndex( m_proxy->mapToSource(index) );
    if ( item && !edit->hasFocus() )
        edit->setText( item->text() );
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    QString text;
    QLineEdit *edit;

    int key = e->key();


    /* CTRL */
    if (e->modifiers() & Qt::ControlModifier) {
        switch(key){
        case Qt::Key_L:
            edit = ui->lineEdit;
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
            text = ui->lineEdit->text();
            printf( text.toLocal8Bit().constData() );
            m_exit_code = 0;
            close();
            break;
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
            ui->listWidget->setFocus();
            break;
        case Qt::Key_Left:
            ui->lineEdit->setCursorPosition(0);
            ui->lineEdit->setFocus();
            break;
        case Qt::Key_Right:
            ui->lineEdit->setFocus();
            break;
        default:
            text = e->text();
            if ( !text.isEmpty() ) {
                edit = ui->lineEdit;
                text.prepend( edit->text() );
                edit->setText(text);
                edit->setFocus();
                setFilter(text);
            }
            break;
        }
    }
}
