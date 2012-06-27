![][icon] Sprinter
==================
*sprinter is fast item selection dialog*

Behaviour
---------
Behaviour is similar to [dmenu] application.

- Program read items from standard input (one item per line) and creates dialog containing item list.

- If user submits item(s) program exits and prints item(s) on standard output,

- otherwise (user presses escape key or closes window without selecting any item) program exits with exit code 1.

Installation
------------
To compile application just run command:

    cmake
    make

and to install it:

    make install

Usage
-----
    $ sprinter --help
    usage: sprinter [options]
    options:
      -c, --command     exec command on items
      -g, --geometry    window size and position (width,height,x,y)
      -h, --help        show this help
      -l, --label       text input label
      -m, --minimal     show popup menu instead of list
      -o, --sort        sort items alphabetically
      -s, --style       stylesheet
      -S, --strict      choose only items from stdin
      -t, --title       title
      -w, --wrap        wrap items
      -z, --size        item size (width,height)
      --opacity         window opacity (value from 0.0 to 1.0)

[icon]: https://github.com/hluk/sprinter/raw/master/icon/sprinter.png "sprinter logo"
[dmenu]: http://tools.suckless.org/dmenu

