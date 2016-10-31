# Dynamic C 10
## for Rabbit-based embedded systems

### Description
This repository is the Open Source release of the libraries and sample
code from Dynamic C 10, an integrated development environment for
[Digi International's](http://www.digi.com/) embedded systems based on
the Rabbit 4000, Rabbit 5000 and Rabbit 6000 microprocessors.

It is of limited use without the rest of the IDE and the appropriate
target hardware.  See the Installation section for instructions on installing
Dynamic C 10.72 and updating its libraries and samples with the code in
this repository.

The `release` branch is a **RELEASE** software release which is fully
QA-tested and supported by Digi International.
The `master` branch is an **ALPHA** software release which has received
limited functional/unit testing.

### Licensing
The [MPL 2.0 license](./LICENSE.txt) covers the majority of this project
with the following exceptions:

- The [ISC license](Samples/LICENSE.txt) covers the contents of the
  `Samples` directory.

- Some libraries were derived from existing Open Source projects and
  carry those projects' original licenses.  Please review those files
  for details on their licensing.
  
  - GPL 2.0: multiple libraries in `Lib/tcpip/WiFi`, including
    `wifi_eloop.lib`, `wifi_md5.lib`, `wifi_sha1.lib`, `wifi_wpa.lib`,
    `wifi_wpa_supplicant.lib`, and `wpa.lib`; `Crypto/SHA2.LIB`
	
  - MPL 1.1/GPL 2.0/LGPL 2.1: `Lib/XBee/util/jslong.c`

### Installation
Instructions on using Git and GitHub are beyond the scope of this document.
If you are new to using Git, we recommend the Windows GUI [Git Extensions][1].
Their website includes a manual and video tutorial.  The program has a
command-line "Git bash" tool, available in the Tools menu, that you can use
to execute the script below.

To make use of this code, either start with an existing (backed-up)
Dynamic C installation (10.72 or later), or [download and install][2] a fresh
copy.  Then add the GitHub repository to the directory.  These instructions
have you creating a private branch where you can store your own changes
to the libraries and samples, merging them in with Digi International's
changes.  It assumes you've already changed to the directory with Dynamic
C installed (e.g., `cd /c/DCRABBIT_10.72A`).

    # Connect the Dynamic C installation with the GitHub repository,
    # and download all of the branches and tags.
    git init
    git remote add origin https://github.com/digidotcom/DCRabbit_10.git
    git fetch --tags
    
	# Configuration options to ignore file modes and preserve line endings
	# (since this is a Windows-only repository).
	git config core.filemode false
	git config core.autocrlf false
    
    # Reset to the matching release (without changing directory contents)
	# by selecting one of the following statements:
	git reset 10.72
	git reset 10.72A
    
    # Get the .gitignore file from the repository.
    git checkout .gitignore
	
    # Create a private branch using the release as a start point.
    git checkout -B mybranch

At this point, `git status` will show any changes to the directory
you have made since the original installation.  You should reset any
accidental changes, or create commits to track intentional changes.

Once you have processed all of your changes, you can cherry-pick
individual commits from the `master` and `release` branches, or merge
either branch, in its entirety, into yours.

[1]: http://gitextensions.github.io/
[2]: http://www.digi.com/support/productdetail?pid=4978&type=software
