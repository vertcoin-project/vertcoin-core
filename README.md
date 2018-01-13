Verticalcoin Core integration/staging tree
=====================================

https://www.vrticalcrypto.com

What is Verticalcoin?
----------------

Verticalcoin is a fork of Vertcoin.

License
-------

Verticalcoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/vertcoin/vertcoin/tags) are created
regularly to indicate new official, stable release versions of Vertcoin Core.

Developers work on their own forks and submit pull requests in order to merge
changes with `master`. Due to the relatively small size of the development team,
developers also commit directly to the repo often. Anyone is allowed to contribute
though and useful pull requests will almost always be accepted given various
obvious stipulations regarding stability etc. 

The Verticalcoin [discord](https://discord.gg/Htm3qa3) or [subreddit](https://reddit.com/r/vrticalcrypto)
should be used to discuss complicated or controversial changes with the developers 
before working on a patch set.

Testing
-------

Vertcoin currently relies on Bitcoin Core for its testcases, and few of them are
known to work, though the software is based on fully test conforming upstream 
Bitcoin Core versions. We would be grateful to those who can help port the existing
Bitcoin Core test cases to Vertcoin such that they can be used to assure correctness.

Translations
------------

Changes to translations as well as new translations can be submitted to as pull
requests to this repo or to upstream Bitcoin Core.
