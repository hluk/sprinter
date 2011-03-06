#include <QtGui/QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <cstdio>
#include <QDebug>
#include "dialog.h"

/* print help and exit */
void help(int exit_code) {
    printf(
            "usage: sprinter [options]\n"
            "options:\n"
            "  -h, --help\n"
            "  -g, --geometry    window size and position (width,height,x,y)\n"
            "  -s, --style       stylesheet\n"
            "  -l, --label       text input label\n"
            "  -t, --title       title\n"
            "  -w, --wrap        wrap items\n"
            "  -z, --size        item size (width,height)\n"
            "  -m, --minimal     show popup menu instead of list\n"
            "  -o, --opacity     window opacity (value from 0.0 to 1.0)\n"
            /*"  -c, --command   exec command (replace %%s with selected items)\n"*/
            );
    exit(exit_code);
}

QString takeArgument(QStringList &args, bool &ok) {
    QString arg;

    if ( args.isEmpty() ) {
        ok = false;
        return arg;
    }

    arg = args.takeFirst();

    if ( arg.size() > 2 && arg.at(0) == '-' && arg.at(1) != '-' ) {
        args.push_front( arg.mid(2) );
        arg = arg.left(2);
    }

    ok = true;
    return arg;
}

void parseArguments(const QStringList &arguments, Dialog &dialog)
{
    // copy arguments
    QStringList args(arguments);

    args.takeFirst(); // program file path

    while ( !args.isEmpty() ) {
        bool ok;
        int num, num2;
        float fnum;
        QStringList args2;
        QString arg = takeArgument(args, ok);
        if (!ok) help(1);

        if (arg.startsWith("-h") || arg == "--help") {
            help(0);
        } else if (arg.startsWith("-g") || arg == "--geometry") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);

            /* desktop size */
            QDesktopWidget *desk = QApplication::desktop();

            /* width,height,x,y - all optional */
            args2 = arg.split(',');
            if (args2.size() > 4 || args2.isEmpty()) help(1);
            // width
            num = args2.takeFirst().toInt(&ok);
            if (ok && num > 0)
                dialog.resize( qMin(desk->width(), num), dialog.height() );

            if ( !args2.isEmpty() ) {
                // height
                num = args2.takeFirst().toInt(&ok);
                if (ok && num > 0)
                    dialog.resize( dialog.width(), qMin(desk->height(), num) );

                if ( !args2.isEmpty() ) {
                    QPoint pos = dialog.pos();
                    // x
                    num = args2.takeFirst().toInt(&ok);
                    if (ok) {
                        if (num <= 0)
                            num = desk->width() + num;
                        pos.setX(num);
                    }

                    if ( !args2.isEmpty() ) {
                        // y
                        num = args2.takeFirst().toInt(&ok);
                        if (ok) {
                            if (num <= 0)
                                num = desk->height() + num;
                            pos.setY(num);
                        }
                    }

                    dialog.move(pos);
                }
            }
        } else if (arg.startsWith("-s") || arg == "--style") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);

            QFile file(arg);
            ok = file.open(QIODevice::ReadOnly);
            if (!ok) help(1);

            qApp->setStyleSheet( file.readAll() );
            file.close();
        } else if (arg.startsWith("-l") || arg == "--label") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            dialog.setLabel(arg);
        } else if (arg.startsWith("-t") || arg == "--title") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            dialog.setWindowTitle(arg);
        } else if (arg.startsWith("-x") || arg == "--wrap") {
            dialog.setWrapping(true);
        } else if (arg.startsWith("-z") || arg == "--size") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);

            args2 = arg.split(',');
            if (args2.size() != 2) help(1);

            num = args2.at(0).toInt(&ok);
            if (!ok) help(1);

            num2 = args2.at(1).toInt(&ok);
            if (!ok) help(1);

            dialog.setGridSize(num, num2);
        } else if (arg.startsWith("-m") || arg == "--minimal") {
            dialog.hideList(true);
        } else if (arg.startsWith("-o") || arg == "--opacity") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            fnum = arg.toFloat(&ok);
            if (!ok && fnum > 0.0f && fnum < 1.0f) help(1);
            dialog.setWindowOpacity(fnum);
        } else {
            help(1);
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Dialog dialog;

    parseArguments( a.arguments(), dialog );

    dialog.show();

    return a.exec();
}
