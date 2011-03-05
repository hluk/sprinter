#include <QtGui/QApplication>
#include <QFile>
#include <cstdio>
#include <QDebug>
#include "dialog.h"

/* print help and exit */
void help(int exit_code) {
    printf(
            "usage: sprinter [options]\n"
            "options:\n"
            "  -?, --help\n"
            "  -w, --width   window width\n"
            "  -h, --height  window height\n"
            "  -s, --style   stylesheet\n"
            "  -l, --label   text input label\n"
            "  -t, --title   title\n"
            "  -x, --wrap    wrap items\n"
            "  -z, --size    item size (width,height)\n"
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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Dialog w;

    QStringList args = a.arguments();
    args.takeFirst(); // program file path
    while ( !args.isEmpty() ) {
        bool ok;
        int num, num2;
        QStringList args2;
        QString arg = takeArgument(args, ok);

        if (!ok) help(1);

        if (arg.startsWith("-?") || arg == "--help") {
            help(0);
        } else if (arg.startsWith("-w") || arg == "--width") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            num = arg.toInt(&ok);
            if (!ok) help(1);
            w.resize( num, w.height() );
        } else if (arg.startsWith("-h") || arg == "--height") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            num = arg.toInt(&ok);
            if (!ok) help(1);
            w.resize( w.width(), num );
        } else if (arg.startsWith("-s") || arg == "--style") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);

            QFile file(arg);
            ok = file.open(QIODevice::ReadOnly);
            if (!ok) help(1);

            a.setStyleSheet( file.readAll() );
            file.close();
        } else if (arg.startsWith("-l") || arg == "--label") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            w.setLabel(arg);
        } else if (arg.startsWith("-t") || arg == "--title") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);
            w.setWindowTitle(arg);
        } else if (arg.startsWith("-x") || arg == "--wrap") {
            w.setWrapping(true);
        } else if (arg.startsWith("-z") || arg == "--size") {
            arg = takeArgument(args, ok);
            if (!ok) help(1);

            args2 = arg.split(',');
            if (args2.size() != 2) help(1);

            num = args2.at(0).toInt(&ok);
            if (!ok) help(1);

            num2 = args2.at(1).toInt(&ok);
            if (!ok) help(1);

            w.setGridSize(num, num2);
        } else {
            help(1);
        }
    }

    w.show();

    return a.exec();
}
