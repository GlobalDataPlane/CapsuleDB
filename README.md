# CapsuleDB

CapsuleDB is a key-value store built specifically for use with the Global Data Plane and Paranoid Stateful Lambda system.  It is inspired by level-tree databases, such as RocksDB, but uses DataCapsules as its backing storage medium.  Read the initial specification for CapsuleDB [here](https://people.eecs.berkeley.edu/~kubitron/courses/cs262a-F21/projects/reports/project18_report_ver3.pdf).  This repo serves as the main development point for future CapsuleDB work.  

## How to Build and Run

CapsuleDB uses Bazel as its main building tool like the PSL project.  We have a Docker container that includes everything you need to get started.  Using the Docker container is highly recommended, and if you decide to go baremetal you are on your own.

To begin, run the following two commands:

```bash
MY_PROJECT=/path/to/repo

docker run -it --rm \
    -v bazel-cache:/root/.cache/bazel \
    -v "${MY_PROJECT}":/opt/my-project \
    -w /opt/my-project \
    keplerc/paranoid-asylo:latest 
```

This should result in being dropped into an interactive session within the Docker container.  Specifically, you should see the following:

```bash
root@<container_hash>:/opt/my-project#
```

At this point, executing the `ls` command should list the folders in the repo.  From here, you can execute Bazel commands as normal.  To ensure everything is working correctly, start with the following basic sanity check:

```bash
bazel run //src:capsuleDBTest
```

This should result in 1999 key-value pairs being written and retrieved from CapsuleDB.  The stats at the end should read as:

```text
Num levels at end: 3
First key l0: 1901
Last key l0: 1950
L0 max size: 100
First key l1: 1801
Last key l1: 999
L1 max size: 1000
First key l2: 1
Last key l2: 188
L2 max size: 10000
no.of.keys not found is:0
no.of.keys found is:1999
size of test_map: 1999
```

Congratulations!  You have just executed your first puts and gets against CapsuleDB!

## Codebase Structure

CapsuleDB is made up of several core files stored in `src/core`.  There you will find the main CapsuleDB logic, including the main engine file, the indexing logic, and the block logic.  You will also come across several `BUILD` files used to identify different Bazel packages.  For more information on packages, I highly recommend you look through [this part of the Bazel docs](https://docs.bazel.build/versions/main/tutorial/cpp.html).

In addition to the core files, you will see two other types of folder.  The first is the `src/shared` folder which contains several important files that are used repeatedly across CapsuleDB.  For example, `src/shared/capsule.h` holds the `kvs_payload` structure.  That folder already has a `BUILD` file with targets for each of the shared files ready to go, all you need to do is add `//src/shared:desiredtarget` to the `deps` section of your target.  The other type of folder is a feature folder, such as `src/network` or `src/enclave`.  This folders represent subpackages that extend the basic CapsuleDB functionality.  They must have their own BUILD files.  

You may also notice a `BUILD` file in the root of the `src` directory.  This file should only be used to write tests.  Place your tests in the `src/test_files` directory and then use this `BUILD` file to build them.  The reasoning behind this is that integration tests will be crossing packages so often it makes more sense to have the `BUILD` file at a higher level.  Frankly I am not sure this is actually the best practice, but the Bazel docs are so convoluted I am not sure they know what the best practice is either.  

## Developing for CapsuleDB

Since there are many of us working on CapsuleDB simultaneously, please make sure you are doing your work on the correct branch!  For large additions, please follow the naming convention of `dev-featurename` for your branch.  Then, place all of your additional code in a new folder (for an example see the network and enclave folder) along with a BUILD file.  Finally, submit a pull request when it is ready to go!

## CapsuleDB TODOs

There is lots to do on CapsuleDB!  Here is a short list of things that still needs to be dealt with, in order of priority.

- [x] Refactor entire codebase for better modularity and smaller BUILD files (hopefully this has partially fixed the ridiculous build times)
- [x] Change serialization away from Boost library
- [ ] Update serialization so it does not use a counter
- [ ] Add enclave compatability with `handle()` method (see PSL codebase for example)
- [ ] Update README to include how to run enclave tests
- [ ] Update local networking implementation to match that of PSL
- [ ] Update README to include how to run networking test
- [ ] Remove CapsuleDB implementation currently in PSL and convert to either Git submodule or use a Bazel load targeting this repo / version (hooray for modularity! :tada:)
- [ ] Update README with description of how CapsuleDB actually works under the hood

## Contributors

Lead: William Mullen

Contributors:
- Nivedha Krishnakumar
- Brian Wu
- Willis Wang
- Ryan Teoh
