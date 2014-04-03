# Pseudo
**Authenticate sudo sessions using OS X’s Security dialog**

Pseudo uses the OS X Security framework to validate the current
user's `sudo` session. Run `pseudo` to prompt the user for his or her
password using the standard OS X Security dialog. Once authenticated,
`pseudo` will touch the user's session timestamp in `/var/db/sudo`,
granting the user or other programs the ability to invoke `sudo`
without a password for the duration of sudo's timeout period (5
minutes by default).

<img src="http://i.imgur.com/AYKC8nb.png" width="557" height="353">

## Installation

    make
    make install [PREFIX=<prefix>]

## Usage

    pseudo [-m | --message <message>] [-w | --wait]

* `-m`, `--message=<message>`:
  Specifies the message to display in the authentication dialog. The
  default message is "pseudo wants to make changes."

* `-w`, `--wait`:
  Tells `pseudo` to wait in the foreground indefinitely, touching the
  user's session timestamp every 30 seconds. If the session expires
  during this time, `pseudo` exits with a non-zero status code.

## Examples

**Problem**: You have a Mac shell script or command-line utility
that needs to run a command as root, but the standard sudo prompt
looks shady to a novice user.

**Solution**: Use `pseudo` to authenticate the `sudo` session using a
standard OS X Security dialog. If it succeeds, invoke `sudo` with the
command you need to run as root.

```bash
pseudo && sudo mkdir -p /my/system/directory || echo "canceled" >&2
```

**Problem**: You have a Mac shell script or command-line utility that
needs to run several commands as root. Some of the commands may take a
long time to run—longer than sudo's default timeout period of 5
minutes—but you only want to prompt for a password once.

**Solution**: Use `pseudo` to authenticate the `sudo` session the
first time you need to run a command as root. Then fork off `pseudo
--wait` in the background, which will continue to validate the sudo
session every 30 seconds while your program runs.

```bash
AUTHENTICATED=""
sudo() {
  if [ -z "$AUTHENTICATED" ]; then
    pseudo || {
      echo "canceled" >&2
      exit 1
    }
    AUTHENTICATED=true
    pseudo --wait &
  fi
  /usr/bin/sudo "$@"
}

sudo /long/running/command
sudo /another/long/running/command
```

## License

(The MIT License)

Copyright (c) 2014 Sam Stephenson

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
