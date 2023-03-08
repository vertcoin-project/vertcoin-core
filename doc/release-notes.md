23.1 Release Notes
==================

Vertcoin Core version 23.1 is now available from:

<https://github.com/vertcoin-project/vertcoin-core/releases/tag/v23.1>

This release includes new features, various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

<https://github.com/vertcoin-project/vertcoin-core/issues>

To receive upstream security and update notifications, please subscribe to:

<https://bitcoincore.org/en/list/announcements/join/>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the
installer (on Windows) or just copy over `/Applications/Vertcoin-Qt` (on macOS)
or `vertcoind`/`vertcoin-qt` (on Linux).

Upgrading directly from a version of Vertcoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be migrated. Old
wallet versions of Vertcoin Core are generally supported.

Compatibility
==============

Vertcoin Core is supported and extensively tested on operating systems
using the Linux kernel, macOS 10.14+, and Windows 7 and newer.  Vertcoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them.  It is not recommended to use Vertcoin Core on
unsupported systems.

From Vertcoin Core 22.0 onwards, macOS versions earlier than 10.14 are no longer supported.

Notable changes
===============

Updated settings
----------------

- In previous releases, the meaning of the command line option
  `-persistmempool` (without a value provided) incorrectly disabled mempool
  persistence.  `-persistmempool` is now treated like other boolean options to
  mean `-persistmempool=1`. Passing `-persistmempool=0`, `-persistmempool=1`
  and `-nopersistmempool` is unaffected. (#23061)

### P2P

- #25314 p2p: always set nTime for self-advertisements
- Vertcoin Core #211 Reduce getheaders spam by serializing getheaders requests per peer
### RPC and other APIs

- #25220 rpc: fix incorrect warning for address type p2sh-segwit in createmultisig
- #25237 rpc: Capture UniValue by ref for rpcdoccheck
- #25983 Prevent data race for pathHandlers
- #26275 Fix crash on deriveaddresses when index is 2147483647 (2^31-1)

### Build system

- #25201 windeploy: Renewed windows code signing certificate
- #25788 guix: patch NSIS to remove .reloc sections from installer stubs
- #25861 guix: use --build={arch}-guix-linux-gnu in cross toolchain
- #25985 Revert "build: Use Homebrew's sqlite package if it is available"

### GUI

- #24668 build, qt: bump Qt5 version to 5.15.3
- gui#631 Disallow encryption of watchonly wallets
- gui#680 Fixes MacOS 13 segfault by preventing certain notifications

### Tests

- #24454 tests: Fix calculation of external input weights

### Utilities

- Vertcoin Core #210 modernize verthash.dat boost fs calls to support future boost releases

### Miscellaneous

- #26321 Adjust .tx/config for new Transifex CLI

Credits
=======

Thanks to everyone who directly contributed to this release:

- Andrew Chow
- brunoerg
- Hennadii Stepanov
- Joan Karadimov
- John Moffett
- MacroFake
- Martin Zumsande
- Matthew Cummings
- Michael Ford
- muxator
- Pavol Rusnak
- Sebastian Falbesoner
- vertion
- W. J. van der Laan

As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/bitcoin/bitcoin/).
