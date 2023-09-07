[![blocked Build Status](https://api.cirrus-ci.com/github/hilbix/blocked.svg?branch=master)](https://cirrus-ci.com/github/hilbix/blocked/master)

# blocked

List blocked threads in Linux.

- `./blocked '*'` lists all thread states
- `./blocked '?'` only lists threads with unknown (to `blocked` program) states (usually there should be none)
- `./blocked R` lists running threads
- `./blocked DR` lists running and blocked threads
- `./blocked D R` same


and so on.  For a complete list of states see:

- `man procfs`
  - however it may be a bit incomplete, like on Ubuntu 22.04
- <https://manpages.debian.org/latest/manpages/procfs.5.en.html>
  - look for `(3) state`


## Usage

	git clone https://github.com/hilbix/blocked.git
	cd blocked
	make
	./blocked


## Output

STDOUT (in `/proc/`-order):

- State
- PID
- Optional: TaskID
- Process-name in `(`parenthesis`)`

STDERR (in alphabetical order):

- State
- Count
- Description
  - Old states in `[`brackets`]`


## Notes

`Makefile` uses `diet` to compile it into a staticlly linked binary.

- `sudo apt install dietlibc-dev`
- For `amd64` I see a 23 KB stripped binary
- Dynamically compiled it still took 13 KB, so less than 10 KB overhead


## FAQ

WTF why?

- Because I need it

License?

-  This Works is placed under the terms of the Copyright Less License,  
   see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
- Like PD this here is free as free beer and free speech.
- But unlike PD this here also is as free as a free baby, too,
- as German Urheberrecht prohibits to cover this code with a Copyright.

