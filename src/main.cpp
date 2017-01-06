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

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QVector>

#include <cerrno>
#include <cstdio>
#include <unistd.h>

struct Argument {
    const char shopt;
    const char *opt;
};

const Argument arguments[] = {
    {'c', "command"},
    {'g', "geometry"},
    {'h', "help"},
    {'l', "label"},
    {'m', "minimal"},
    {'o', "sort"},
    {'p', "opacity"},
    {'s', "style"},
    {'S', "strict"},
    {'t', "title"},
    {'w', "wrap"},
    {'z', "size"},
};

static QString helpString(const char shopt)
{
    if (shopt == 'c') return QObject::tr("exec command on items");
    if (shopt == 'g') return QObject::tr("window size and position (format: width,height,x,y)");
    if (shopt == 'h') return QObject::tr("show this help");
    if (shopt == 'l') return QObject::tr("text input label");
    if (shopt == 'm') return QObject::tr("show popup menu instead of list");
    if (shopt == 'o') return QObject::tr("sort items alphabetically");
    if (shopt == 'p') return QObject::tr("window opacity (value from 0.0 to 1.0)");
    if (shopt == 's') return QObject::tr("stylesheet");
    if (shopt == 'S') return QObject::tr("choose only items from stdin");
    if (shopt == 't') return QObject::tr("title");
    if (shopt == 'w') return QObject::tr("wrap items");
    if (shopt == 'z') return QObject::tr("item size (format: width,height)");
    return "";
}

static void printLine(const QString &msg)
{
    printf( "%s", (msg + "\n").toLocal8Bit().constData() );
}

/* print help and exit */
static void help(int exit_code)
{
    int len = sizeof(arguments)/sizeof(Argument);

    printLine( QObject::tr("usage: sprinter [options]") );
    printLine( QObject::tr("options:") );
    for ( int i = 0; i<len; ++i ) {
        const Argument &arg = arguments[i];
        printf( "  -%c, --%-12s ", arg.shopt, arg.opt );
        printLine(helpString(arg.shopt));
    }
    exit(exit_code);
}

// TODO: parse %s in command string
static void parseCommand(const char *cmd, QList<QByteArray> &args)
{
    QByteArray arg;
    bool quotes = false;
    bool dquotes = false;
    bool escape = false;
    bool outside = true;
    int len = sizeof(cmd);
    for(int i = 0; i<len; ++i) {
        const char c = cmd[i];
        if ( outside ) {
            if ( isspace(c) )
                continue;
            else
                outside = false;
        }

        /*
        if ( c == '1' && arg.endsWith('%') ) {
            arg.remove( arg.size()-1, 1 );
            arg.append();
            continue;
        }*/

        if (escape) {
            if ( c == 'n' ) {
                arg.append('\n');
            } else if ( c == 't' ) {
                arg.append('\t');
            } else {
                arg.append(c);
            }
        } else if (c == '\\') {
            escape = true;
        } else if (c == '\'') {
            quotes = !quotes;
        } else if (c == '"') {
            dquotes = !dquotes;
        } else if (quotes) {
            arg.append(c);
        } else if ( isspace(c) ) {
            outside = true;
            args.append(arg);
            arg.clear();
        } else {
            arg.append(c);
        }
    }
    if ( !outside ) {
        args.append(arg);
        arg.clear();
    }
}

static void parseArguments(int argc, char *argv[], Dialog &dialog,
                    QList<QByteArray> &command_args)
{
    int num, num2;
    float fnum;
    bool ok;
    char c;
    bool force_arg;
    int len = sizeof(arguments)/sizeof(Argument);

    int i = 1;
    while(i<argc) {
        const char *argp = argv[i];
        ++i;

        if (argp[0] != '-' || argp[1] == '\0')
            return help(1);

        int j = 0;
        force_arg = false;

        // long option
        if (argp[1] == '-') {
            argp += 2;
            for ( ; j<len; ++j) {
                if ( strcmp(argp, arguments[j].opt) == 0 )
                    break;
            }
            argp = i<argc ? argv[i] : NULL;
        }
        // short option
        else {
            argp += 1;
            for ( ; j<len; ++j) {
                if ( *argp == arguments[j].shopt )
                    break;
            }
            argp += 1;
            if (*argp == '\0') {
                argp = i<argc ? argv[i] : NULL;
            } else {
                force_arg = true;
                --i;
            }
        }

        if ( j == len )
            return help(1);

        /* do action */
        const char arg = arguments[j].shopt;
        if (arg == 'c') {
            if (!argp) help(1);
            ++i;
            parseCommand(argp, command_args);
            dialog.saveOutput(&command_args);
        } else if (arg == 'g') {
            if (!argp) help(1);
            ++i;

            /* desktop size */
            QDesktopWidget *desk = QApplication::desktop();
            QPoint pos = dialog.pos();

            /* width,height,x,y - all optional */
            // width
            num2 = sscanf(argp, "%d%c", &num, &c);
            if (num2 > 0 && num > 0)
                dialog.resize( qMin(desk->width(), num), dialog.height() );

            while( isdigit(*argp) || *argp=='+' ) ++argp;
            if (*argp == ',')
                ++argp;
            else if (*argp != '\0')
                help(1);

            // height
            num2 = sscanf(argp, "%d", &num);
            if (num2 > 0 && num > 0)
                dialog.resize( dialog.width(), qMin(desk->height(), num) );

            while( isdigit(*argp) || *argp=='+' ) ++argp;
            if (*argp == ',')
                ++argp;
            else if (*argp != '\0')
                help(1);

            // x
            num2 = sscanf(argp, "%d", &num);
            if (num2 > 0) {
                if (num < 0)
                    num = desk->width() + num - dialog.width();
                pos.setX(num);
                dialog.move(pos);
            }

            while( isdigit(*argp) || *argp=='+' || *argp=='-' ) ++argp;
            if (*argp == ',')
                ++argp;
            else if (*argp != '\0')
                help(1);

            // y
            num2 = sscanf(argp, "%d", &num);
            if (num2 > 0) {
                if (num < 0)
                    num = desk->height() + num - dialog.height();
                pos.setY(num);
                dialog.move(pos);
            }

            while( isdigit(*argp) || *argp=='+' || *argp=='-' ) ++argp;
            if (*argp != '\0')
                help(1);
        } else if (arg == 'h') {
            if (force_arg) help(1);
            help(0);
        } else if (arg == 'l') {
            if (!argp) help(1);
            ++i;
            dialog.setLabel(argp);
        } else if (arg == 'm') {
            if (force_arg) help(1);
            dialog.hideList(true);
        } else if (arg == 'o') {
            if (force_arg) help(1);
            dialog.sortList();
        } else if (arg == 'p') {
            if (!argp) help(1);
            ++i;
            num = sscanf(argp, "%f%c", &fnum, &c);
            if (num != 1 || fnum < 0.0f || fnum > 1.0f)
                help(1);
            dialog.setWindowOpacity(fnum);
        } else if (arg == 's') {
            if (!argp) help(1);
            ++i;

            QFile file(argp);
            ok = file.open(QIODevice::ReadOnly);
            if (!ok) help(1);

            qApp->setStyleSheet( file.readAll() );
            file.close();
        } else if (arg == 'S') {
            if (force_arg) help(1);
            dialog.setStrict(true);
        } else if (arg == 't') {
            if (!argp) help(1);
            ++i;
            dialog.setWindowTitle(argp);
        } else if (arg == 'w') {
            if (force_arg) help(1);
            dialog.setWrapping(true);
        } else if (arg == 'z') {
            if (!argp) help(1);
            ++i;

            if ( sscanf(argp, "%d,%d%c", &num, &num2, &c) != 2 )
                help(1);
            if ( num <= 0 || num2 <= 0 )
                help(1);

            dialog.setGridSize(num, num2);
        } else {
            help(1);
        }
    }
}

int main(int argc, char *argv[])
{
    int exit_code;
    QList<QByteArray> command_args;

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    Dialog dialog;

    parseArguments(argc, argv, dialog, command_args);

    dialog.show();

    exit_code = app.exec();

    /* exec command */
    if ( !exit_code && !command_args.isEmpty() ) {
        int len = command_args.size();
        char **new_argv = new char* [len+1];

        for( int i = 0; i<len; ++i ) {
            new_argv[i] = command_args[i].data();
        }
        new_argv[len] = NULL;

        execvp(new_argv[0], new_argv);
        perror( strerror(errno) );
        return errno;
    }

    return exit_code;
}
