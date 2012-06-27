#include <QtGui/QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include "dialog.h"

#define TR(x) (QObject::tr(x).toLocal8Bit().constData())

struct Argument {
    const char shopt;
    const char *opt;
    const char *help;
};

const Argument arguments[] = {
    {'c', "command",  "exec command on items"},
    {'g', "geometry", "window size and position (format: width,height,x,y)"},
    {'h', "help",     "show this help"},
    {'l', "label",    "text input label"},
    {'m', "minimal",  "show popup menu instead of list"},
    {'o', "sort",     "sort items alphabetically"},
    {'p', "opacity",  "window opacity (value from 0.0 to 1.0)"},
    {'s', "style",    "stylesheet"},
    {'S', "strict",   "choose only items from stdin"},
    {'t', "title",    "title"},
    {'w', "wrap",     "wrap items"},
    {'z', "size",     "item size (format: width,height)"}
};

/* print help and exit */
void help(int exit_code) {
    int len = sizeof(arguments)/sizeof(Argument);

    printf( TR("usage: sprinter [options]\n") );
    printf( TR("options:\n") );
    for ( int i = 0; i<len; ++i ) {
        const Argument &arg = arguments[i];
        printf( "  -%c, --%-12s %s\n", arg.shopt, arg.opt, TR(arg.help) );
    }
    exit(exit_code);
}

// TODO: parse %s in command string
void parseCommand(const char *cmd, QList<QByteArray> &args)
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

void parseArguments(int argc, char *argv[], Dialog &dialog,
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

#ifdef QT_DEBUG
    qDebug("NOTE: Starting debug version.");
#endif

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    Dialog dialog;

    parseArguments(argc, argv, dialog, command_args);

    dialog.show();

    exit_code = a.exec();

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
