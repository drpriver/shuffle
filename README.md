# Shuffle
A simple cli app for shuffling input, drawing with or without replacement.

## Building
A meson.build, CMakeLists.txt and a Makefile are provided. Makefile works with clang or gcc.

## Usage
```
$ shuffle -h
shuffle: Shuffles lines, outputting them in a random order.

usage: shuffle [-bhirs] [-S SEED] [-n N] [--] [file ...] [-a ARG ...]

Flags:
------
-b: Stop when the first blank line is encountered.
-h: Print this help and exit.
-i: Read lines from stdin (in addition to the input files).
-r: Allow repeats in the output (draw with replacement).
-s: Skip blank lines in files.
--: Interpret all following arguments as filenames
    so filenames starting with '-' can be read.

Consuming Args (consumes next argument):
----------------------------------------
-a [ARG ...]: Treat the following arguments as input lines.
-n N:         Print no more than N output lines.
              Will not print more than the number of input lines,
              unless the -r flag is set.
-S SEED:      Seed the rng with the given string
              If not given, seeds via the system.

If no filenames are listed or the -i flag is passed, shuffle will read
from stdin until the EOF is encounted (eg, ^D) or a blank line is inputted.
```

```
$ shuffle -a foo bar baz
bar
foo
baz
```

```
$ cat food.txt
pizza
thai
burgers
$ shuffle food.txt
burgers
thai
pizza
```

```
$ shuffle -r -n 6 -a cat dog
dog
cat
dog
dog
dog
dog
```
