SnowFox extentions
==================
This repository contains the official non-core components for SnowFox.


Command line parsers
--------------------
The current command line partser uses GFlag.
The long term goal is to re-implement the parser from scratch to allow
more complex command line structures (with sub-commands similar to git).

The solution is to abstract away the implementation and let the user choose.
Built in options are:

  * `ext.cli.gflags`: GFlags based command line parser.
