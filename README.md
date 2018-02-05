# What Is My Stdio?

An extremely simple demonstration program to report what the standard I/O
streams `stdin`, `stdout`, and `stderr` are connected to.

For each stream, the program reports the file type and attempts to report the
name of the file.

If the stream is associated with a pipe, it attempts to find the process that
owns the other end of the pipe.

## Usage

Build and run with

    make
    ./what-is-my-stdio

## Examples

In an interactive shell

    $ ./what-is-my-stdio
    what-is-my-stdio: stdin is a terminal, open to /dev/pts/3
    what-is-my-stdio: stdout is a terminal, open to /dev/pts/3
    what-is-my-stdio: stderr is a terminal, open to /dev/pts/3

With some streams redirected to the null device

    $ ./what-is-my-stdio < /dev/null 2> /dev/null
    what-is-my-stdio: stdin is a character special device, open to /dev/null
    what-is-my-stdio: stdout is a terminal, open to /dev/pts/3
    what-is-my-stdio: stderr is a character special device, open to /dev/null

With input and output in a pipeline

    $ yes | ./what-is-my-stdio | cat
    what-is-my-stdio: stdin is a pipe with the other end owned by yes (32427)
    what-is-my-stdio: stdout is a pipe with the other end owned by cat (32429)
    what-is-my-stdio: stderr is a terminal, open to /dev/pts/3

With output sent to a named pipe

    $ mkfifo /tmp/foo
    $ ./what-is-my-stdio > /tmp/foo &
    $ cat /tmp/foo
    what-is-my-stdio: stdin is a terminal, open to /dev/pts/3
    what-is-my-stdio: stdout is a named pipe, open to /tmp/foo
    what-is-my-stdio: stderr is a terminal, open to /dev/pts/3

## License

This program is licensed under the
[GPL](https://www.gnu.org/licenses/gpl.html) license.
See [COPYING](COPYING) for the full license text.
