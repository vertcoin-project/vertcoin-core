Vertcoin integration/staging tree
================================

http://www.vertcoin.org

Copyright (c) 2009-2017 Bitcoin Developers
Copyright (c) 2014-2017 Vertcoin Developers

What is Vertcoin?
----------------

Vertcoin is a lite version of Bitcoin using Lyra2RE as a proof-of-work algorithm.
 - 2.5 minute block targets
 - subsidy halves in 840k blocks (~4 years)
 - ~84 million total coins
 - 50 coins per block
 - Every block to retarget difficulty with Kimotos Gravity Well algorithm

For more information, as well as an immediately useable, binary version of
the Vertcoin client sofware, see http://www.vertcoin.org.

License
-------

Vertcoin is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.

Development process
-------------------

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Vertcoin
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion with the devs and community.

The patch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see `doc/coding.txt`) or are
controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/vertcoin/vertcoin/tags) are created
regularly to indicate new official, stable release versions of Vertcoin.

Building
--------

Environment specific building instructions are located in the following directories and files:

vertcoind (headless wallet):

* doc/build-msw.md (Windows)
* doc/build-osx.md (Mac OS X)
* doc/build-unix.md (Unix)

vertcoin-qt (wallet with GUI):

* doc/readme-qt.rst

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test. Please be patient and help out, and
remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write unit tests for new code, and to
submit new unit tests for old code.

Unit tests for the core code are in `src/test/`. To compile and run them:

    cd src; make -f makefile.unix test

Unit tests for the GUI code are in `src/qt/test/`. To compile and run them:

    qmake VERTCOIN_QT_TEST=1 -o Makefile.test vertcoin-qt.pro
    make -f Makefile.test
    ./vertcoin-qt_test

