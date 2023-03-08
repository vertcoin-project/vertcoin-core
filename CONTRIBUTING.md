Contributing to Vertcoin Core
============================

The Vertcoin Core project operates an open contributor model where anyone is
welcome to contribute towards development in the form of peer review, testing
and patches. This document explains the practical process and guidelines for
contributing.

First, in terms of structure, there is no particular concept of "Bitcoin Core
developers" in the sense of privileged people. Open source often naturally
revolves around a meritocracy where contributors earn trust from the developer
community over time. Nevertheless, some hierarchy is necessary for practical
purposes. As such, there are repository "maintainers" who are responsible for
merging pull requests, as well as a "lead maintainer" who is responsible for the
release cycle as well as overall merging, moderation and appointment of
maintainers.

Communication Channels
----------------------

Most communication about Vertcoin Core development happens onthe #development channel
on [Discord](https://discord.gg/vertcoin).

Discussion about codebase improvements happens in GitHub issues and pull
requests.

Open issues can be found on github in the [issues list](https://github.com/vertcoin-project/vertcoin-core/issues).

Contributor Workflow
--------------------

The codebase is maintained using the "contributor workflow" where everyone
without exception contributes patch proposals using "pull requests" (PRs). This
facilitates social contribution, easy testing and peer review.

To contribute a patch, the workflow is as follows:

  1. Fork repository ([only for the first time](https://help.github.com/en/articles/fork-a-repo))
  1. Create topic branch
  1. Commit patches

For GUI-related issues or pull requests, the https://github.com/bitcoin-core/gui repository should be used.
For all other issues and pull requests, the https://github.com/bitcoin/bitcoin node repository should be used.

The master branch for all monotree repositories is identical.

As a rule of thumb, everything that only modifies `src/qt` is a GUI-only pull
request. However:

* For global refactoring or other transversal changes the node repository
  should be used.
* For GUI-related build system changes, the node repository should be used
  because the change needs review by the build systems reviewers.
* Changes in `src/interfaces` need to go to the node repository because they
  might affect other components like the wallet.

For large GUI changes that include build system and interface changes, it is
recommended to first open a pull request against the GUI repository. When there
is agreement to proceed with the changes, a pull request with the build system
and interfaces changes can be submitted to the node repository.

The project coding conventions in the [developer notes](doc/developer-notes.md)
must be followed.

### Committing Patches

In general, [commits should be atomic](https://en.wikipedia.org/wiki/Atomic_commit#Atomic_commit_convention)
and diffs should be easy to read. For this reason, do not mix any formatting
fixes or code moves with actual code changes.

Make sure each individual commit is hygienic: that it builds successfully on its
own without warnings, errors, regressions, or test failures.

Commit messages should be verbose by default consisting of a short subject line
(50 chars max), a blank line and detailed explanatory text as separate
paragraph(s), unless the title alone is self-explanatory (like "Correct typo
in init.cpp") in which case a single title line is sufficient. Commit messages should be
helpful to people reading your code in the future, so explain the reasoning for
your decisions. Further explanation [here](https://chris.beams.io/posts/git-commit/).

If a particular commit references another issue, please add the reference. For
example: `refs #1234` or `fixes #4321`. Using the `fixes` or `closes` keywords
will cause the corresponding issue to be closed when the pull request is merged.

Commit messages should never contain any `@` mentions (usernames prefixed with "@").

Please refer to the [Git manual](https://git-scm.com/doc) for more information
about Git.

  - Push changes to your fork
  - Create pull request

### Creating the Pull Request

The title of the pull request should be prefixed by the component or area that
the pull request affects. Valid areas as:

  - `consensus` for changes to consensus critical code
  - `doc` for changes to the documentation
  - `qt` or `gui` for changes to bitcoin-qt
  - `log` for changes to log messages
  - `mining` for changes to the mining code
  - `net` or `p2p` for changes to the peer-to-peer network code
  - `refactor` for structural changes that do not change behavior
  - `rpc`, `rest` or `zmq` for changes to the RPC, REST or ZMQ APIs
  - `script` for changes to the scripts and tools
  - `test`, `qa` or `ci` for changes to the unit tests, QA tests or CI code
  - `util` or `lib` for changes to the utils or libraries
  - `wallet` for changes to the wallet code
  - `build` for changes to the GNU Autotools or reproducible builds

Examples:

    consensus: Add new opcode for BIP-XXXX OP_CHECKAWESOMESIG
    net: Automatically create onion service, listen on Tor
    qt: Add feed bump button
    log: Fix typo in log message

The body of the pull request should contain sufficient description of *what* the
patch does, and even more importantly, *why*, with justification and reasoning.
You should include references to any discussions (for example, other issues or
mailing list discussions).

The description for a new pull request should not contain any `@` mentions. The
PR description will be included in the commit message when the PR is merged and
any users mentioned in the description will be annoyingly notified each time a
fork of Bitcoin Core copies the merge. Instead, make any username mentions in a
subsequent comment to the PR.

### Translation changes

Note that translations should not be submitted as pull requests. Please see
[Translation Process](https://github.com/bitcoin/bitcoin/blob/master/doc/translation_process.md)
for more information on helping with translations.

### Work in Progress Changes and Requests for Comments

If a pull request is not to be considered for merging (yet), please
prefix the title with [WIP] or use [Tasks Lists](https://help.github.com/articles/basic-writing-and-formatting-syntax/#task-lists)
in the body of the pull request to indicate tasks are pending.

### Address Feedback

At this stage, one should expect comments and review from other contributors. You
can add more commits to your pull request by committing them locally and pushing
to your fork.

You are expected to reply to any review comments before your pull request is
merged. You may update the code or reject the feedback if you do not agree with
it, but you should express so in a reply. If there is outstanding feedback and
you are not actively working on it, your pull request may be closed.

Please refer to the [peer review](#peer-review) section below for more details.

### Squashing Commits

If your pull request contains fixup commits (commits that change the same line of code repeatedly) or too fine-grained
commits, you may be asked to [squash](https://git-scm.com/docs/git-rebase#_interactive_mode) your commits
before it will be reviewed. The basic squashing workflow is shown below.

    git checkout your_branch_name
    git rebase -i HEAD~n
    # n is normally the number of commits in the pull request.
    # Set commits (except the one in the first line) from 'pick' to 'squash', save and quit.
    # On the next screen, edit/refine commit messages.
    # Save and quit.
    git push -f # (force push to GitHub)

Please update the resulting commit message, if needed. It should read as a
coherent message. In most cases, this means not just listing the interim
commits.

If you have problems with squashing or other git workflows, you can enable
"Allow edits from maintainers" in the right-hand sidebar of the GitHub web
interface and ask for help in the pull request.

Please refrain from creating several pull requests for the same change.
Use the pull request that is already open (or was created earlier) to amend
changes. This preserves the discussion and review that happened earlier for
the respective change set.

The length of time required for peer review is unpredictable and will vary from
pull request to pull request.

Release Policy
--------------

The project leader is the release manager for each Vertcoin Core release.

Copyright
---------

By contributing to this repository, you agree to license your work under the
MIT license unless specified otherwise in `contrib/debian/copyright` or at
the top of the file itself. Any work contributed where you are not the original
author must contain its license header with the original author(s) and source.
