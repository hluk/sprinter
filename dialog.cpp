#include "dialog.h"
#include "ui_dialog.h"
#include <QIODevice>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
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

    connect( ui->listWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(itemSelected(QModelIndex,QModelIndex)) );
    connect( ui->lineEdit, SIGNAL(textEdited(QString)),
             this, SLOT(setFilter(QString)) );

    //thread()->setPriority(QThread::HighestPriority);

    setLabel("");
}

Dialog::~Dialog()
{
    delete ui;

    exit(m_exit_code);
}

void Dialog::setLabel(const QString &text)
{
    ui->label->setText(text);
}

void Dialog::setWrapping(bool enable)
{
    ui->listWidget->setHorizontalScrollBarPolicy(
            enable ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff );
    ui->listWidget->setWrapping(enable);
}

void Dialog::setGridSize(int w, int h)
{
    ui->listWidget->setGridSize( QSize(w, h) );
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
    if ( edit->hasFocus() )
        return;

    QModelIndexList indeces;
    indeces = ui->listWidget->selectionModel()->selectedIndexes();
    if ( indeces.indexOf(index) == -1 )
        indeces.append(index);

    QStringList captions;
    foreach (QModelIndex index, indeces) {
        QVariant data = m_model->data( m_proxy->mapToSource(index) );
        if ( data.isValid() )
            captions.append( data.toString() );
    }
    edit->setText( captions.join("\n") );
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
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            if ( ui->listWidget->currentIndex().row() == 0 ) {
                ui->lineEdit->setFocus();
                break;
            }
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            ui->listWidget->setFocus();
            break;
        case Qt::Key_Left:
            if ( !ui->listWidget->isWrapping() ) {
                ui->lineEdit->setCursorPosition(0);
                ui->lineEdit->setFocus();
            }
            break;
        case Qt::Key_Right:
            if ( !ui->listWidget->isWrapping() ) {
                ui->lineEdit->setFocus();
            }
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
