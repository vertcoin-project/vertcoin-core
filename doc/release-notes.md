0.20.1 Release Notes
====================

Bitcoin Core version 0.20.1 is now available from:

  <https://bitcoincore.org/bin/bitcoin-core-0.20.1/>

This minor release includes various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/bitcoin/bitcoin/issues>

To receive security and update notifications, please subscribe to:

  <https://bitcoincore.org/en/list/announcements/join/>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the
installer (on Windows) or just copy over `/Applications/Bitcoin-Qt` (on Mac)
or `bitcoind`/`bitcoin-qt` (on Linux).

Known issues
============
=======
Upgrading directly from a version of Bitcoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be migrated. Old
wallet versions of Bitcoin Core are generally supported.

Compatibility
==============

Bitcoin Core is supported and extensively tested on operating systems
using the Linux kernel, macOS 10.12+, and Windows 7 and newer.  Bitcoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them.  It is not recommended to use Bitcoin Core on
unsupported systems.

From Bitcoin Core 0.20.0 onwards, macOS versions earlier than 10.12 are no
longer supported. Additionally, Bitcoin Core does not yet change appearance
when macOS "dark mode" is activated.

Known Bugs
==========

The process for generating the source code release ("tarball") has changed in an
effort to make it more complete, however, there are a few regressions in
this release:


### P2P protocol and network code
- #14685 `9406502` Fix a deserialization overflow edge case (kazcw)
- #14728 `b901578` Fix uninitialized read when stringifying an addrLocal (kazcw)

### Wallet
- #14441 `5150acc` Restore ability to list incoming transactions by label (jnewbery)
- #13546 `91fa15a` Fix use of uninitialized value `bnb_used` in CWallet::CreateTransaction(…) (practicalswift)
- #14310 `bb90695` Ensure wallet is unlocked before signing (gustavonalle)
- #14690 `5782fdc` Throw error if CPubKey is invalid during PSBT keypath serialization (instagibbs)
- #14852 `2528443` backport: [tests] Add `wallet_balance.py` (MarcoFalke)
- #14196 `3362a95` psbt: always drop the unnecessary utxo and convert non-witness utxo to witness when necessary (achow101)
- #14588 `70ee1f8` Refactor PSBT signing logic to enforce invariant and fix signing bug (gwillen)
- #14424 `89a9a9d` Stop requiring imported pubkey to sign non-PKH schemes (sipa, MeshCollider)

### RPC and other APIs
- #14417 `fb9ad04` Fix listreceivedbyaddress not taking address as a string (etscrivner)
- #14596 `de5e48a` Bugfix: RPC: Add `address_type` named param for createmultisig (luke-jr)
- #14618 `9666dba` Make HTTP RPC debug logging more informative (practicalswift)
- #14197 `7bee414` [psbt] Convert non-witness UTXOs to witness if witness sig created (achow101)
- #14377 `a3fe125` Check that a separator is found for psbt inputs, outputs, and global map (achow101)
- #14356 `7a590d8` Fix converttopsbt permitsigdata arg, add basic test (instagibbs)
- #14453 `75b5d8c` Fix wallet unload during walletpassphrase timeout (promag)

### GUI
- #14403 `0242b5a` Revert "Force TLS1.0+ for SSL connections" (real-or-random)
- #14593 `df5131b` Explicitly disable "Dark Mode" appearance on macOS (fanquake)

### Build system
- #14647 `7edebed` Remove illegal spacing in darwin.mk (ch4ot1c)
- #14698 `ec71f06` Add bitcoin-tx.exe into Windows installer (ken2812221)

### Tests and QA
- #13965 `29899ec` Fix extended functional tests fail (ken2812221)
- #14011 `9461f98` Disable wallet and address book Qt tests on macOS minimal platform (ryanofsky)
- #14180 `86fadee` Run all tests even if wallet is not compiled (MarcoFalke)
- #14122 `8bc1bad` Test `rpc_help.py` failed: Check whether ZMQ is enabled or not (Kvaciral)
- #14101 `96dc936` Use named args in validation acceptance tests (MarcoFalke)
- #14020 `24d796a` Add tests for RPC help (promag)
- #14052 `7ff32a6` Add some actual witness in `rpc_rawtransaction` (MarcoFalke)
- #14215 `b72fbab` Use correct python index slices in example test (sdaftuar)
- #14024 `06544fa` Add `TestNode::assert_debug_log` (MarcoFalke)
- #14658 `60f7a97` Add test to ensure node can generate all rpc help texts at runtime (MarcoFalke)
- #14632 `96f15e8` Fix a comment (fridokus)
- #14700 `f9db08e` Avoid race in `p2p_invalid_block` by waiting for the block request (MarcoFalke)
- #14845 `67225e2` Add `wallet_balance.py` (jnewbery)

### Documentation
- #14161 `5f51fd6` doc/descriptors.md tweaks (ryanofsky)
- #14276 `85aacc4` Add autogen.sh in ARM Cross-compilation (walterwhite81)
=======
0.18.1 change log
=================

### P2P protocol and network code
- #15990 Add tests and documentation for blocksonly (MarcoFalke)
- #16021 Avoid logging transaction decode errors to stderr (MarcoFalke)
- #16405 fix: tor: Call `event_base_loopbreak` from the event's callback (promag)
- #16412 Make poll in InterruptibleRecv only filter for POLLIN events (tecnovert)

### Wallet
- #15913 Add -ignorepartialspends to list of ignored wallet options (luke-jr)

### RPC and other APIs
- #15991 Bugfix: fix pruneblockchain returned prune height (jonasschnelli)
- #15899 Document iswitness flag and fix bug in converttopsbt (MarcoFalke)
- #16026 Ensure that uncompressed public keys in a multisig always returns a legacy address (achow101)
- #14039 Disallow extended encoding for non-witness transactions (sipa)
- #16210 add 2nd arg to signrawtransactionwithkey examples (dooglus)
- #16250 signrawtransactionwithkey: report error when missing redeemScript/witnessScript (ajtowns)

### GUI
- #16044 fix the bug of OPEN CONFIGURATION FILE on Mac (shannon1916)
- #15957 Show "No wallets available" in open menu instead of nothing (meshcollider)
- #16118 Enable open wallet menu on setWalletController (promag)
- #16135 Set progressDialog to nullptr (promag)
- #16231 Fix open wallet menu initialization order (promag) 
- #16254 Set `AA_EnableHighDpiScaling` attribute early (hebasto) 
- #16122 Enable console line edit on setClientModel (promag) 
- #16348 Assert QMetaObject::invokeMethod result (promag)

### Build system
- #15985 Add test for GCC bug 90348 (sipa)
- #15947 Install bitcoin-wallet manpage (domob1812)
- #15983 build with -fstack-reuse=none (MarcoFalke)

### Tests and QA
- #15826 Pure python EC (sipa)
- #15893 Add test for superfluous witness record in deserialization (instagibbs)
- #14818 Bugfix: test/functional/rpc_psbt: Remove check for specific error message that depends on uncertain assumptions (luke-jr)
- #15831 Add test that addmultisigaddress fails for watchonly addresses (MarcoFalke)

### Documentation
- #15890 Remove text about txes always relayed from -whitelist (harding)

### Miscellaneous
- #16095 Catch by reference not value in wallettool (kristapsk)
- #16205 Replace fprintf with tfm::format (MarcoFalke)
=======
- The generated `configure` script is currently missing, and you will need to
  install autotools and run `./autogen.sh` before you can run
  `./configure`. This is the same as when checking out from git.

- Instead of running `make` simply, you should instead run
  `BITCOIN_GENBUILD_NO_GIT=1 make`.

Notable changes
===============

Changes regarding misbehaving peers
-----------------------------------

Peers that misbehave (e.g. send us invalid blocks) are now referred to as
discouraged nodes in log output, as they're not (and weren't) strictly banned:
incoming connections are still allowed from them, but they're preferred for
eviction.

Furthermore, a few additional changes are introduced to how discouraged
addresses are treated:

- Discouraging an address does not time out automatically after 24 hours
  (or the `-bantime` setting). Depending on traffic from other peers,
  discouragement may time out at an indeterminate time.

- Discouragement is not persisted over restarts.

- There is no method to list discouraged addresses. They are not returned by
  the `listbanned` RPC. That RPC also no longer reports the `ban_reason`
  field, as `"manually added"` is the only remaining option.

- Discouragement cannot be removed with the `setban remove` RPC command.
  If you need to remove a discouragement, you can remove all discouragements by
  stop-starting your node.

Notification changes
--------------------

`-walletnotify` notifications are now sent for wallet transactions that are
removed from the mempool because they conflict with a new block. These
notifications were sent previously before the v0.19 release, but had been
broken since that release (bug
[#18325](https://github.com/bitcoin/bitcoin/issues/18325)).

0.20.1 change log
=================

### Mining
- #19019 Fix GBT: Restore "!segwit" and "csv" to "rules" key (luke-jr)

### P2P protocol and network code
- #19219 Replace automatic bans with discouragement filter (sipa)

### Wallet
- #19300 Handle concurrent wallet loading (promag)
- #18982 Minimal fix to restore conflicted transaction notifications (ryanofsky)

### RPC and other APIs
- #19524 Increment input value sum only once per UTXO in decodepsbt (fanquake)
- #19517 psbt: Increment input value sum only once per UTXO in decodepsbt (achow101)

### GUI
- #19097 Add missing QPainterPath include (achow101)
- #19059 update Qt base translations for macOS release (fanquake)

### Build system
- #19152 improve build OS configure output (skmcontrib)
- #19536 qt, build: Fix QFileDialog for static builds (hebasto)

### Tests and QA
- #19444 Remove cached directories and associated script blocks from appveyor config (sipsorcery)
- #18640 appveyor: Remove clcache (MarcoFalke)

### Miscellaneous
- #19194 util: Don't reference errno when pthread fails (miztake)
- #18700 Fix locking on WSL using flock instead of fcntl (meshcollider)

Credits
=======

Thanks to everyone who directly contributed to this release:

- Aaron Clauson
- Andrew Chow
- fanquake
- Hennadii Stepanov
- João Barbosa
- Luke Dashjr
- MarcoFalke
- MIZUTA Takeshi
- Pieter Wuille
- Russell Yanofsky
- sachinkm77
- Samuel Dobson
- Wladimir J. van der Laan

As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/bitcoin/bitcoin/).
