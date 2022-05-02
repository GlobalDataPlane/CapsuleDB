# CapsuleDB

CapsuleDB is a key-value store built specifically for use with the Global Data Plane and Paranoid Stateful Lambda system.  It is inspired by level-tree databases, such as RocksDB, but uses DataCapsules as its backing storage medium.  Read the initial specification for CapsuleDB [here](https://people.eecs.berkeley.edu/~kubitron/courses/cs262a-F21/projects/reports/project18_report_ver3.pdf).  This repo serves as the main development point for future CapsuleDB work.  

## Building and Running

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

Given that you will be running these two commands every time you start working on CapsuleDB, I would recommend writing a script.  The commands should result in you being dropped into an interactive session within the Docker container.  Specifically, you should see the following:

```bash
root@<container_hash>:/opt/my-project#
```

At this point, executing the `ls` command should list the folders in the repo.  From here, you can execute Bazel commands as normal.  To ensure everything is working correctly, start with the following basic sanity check:

```bash
bazel run //src:test_core
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

## Enclave Test

CapsuleDB is equipped with an enclave interface, allowing it to be securely run within an Intel SGX enclave.  It is built using Asylo, so that theoretically, once Asylo supports other trusted execution environment backends, it will be immediately portable.  However, Asylo only supports Intel SGX at the moment, and thus so do we.  

This codebase contains two slightly different enclave based tests, `test_enclave_basic.cc` and `test_enclave_interface.cc`, both of which can be found in the `test_files` directory.  `test_enclave_basic.cc` tests the basic functionality of the enclave itself, linking directly to it.  In contrast, `test_enclave_interface.cc` goes through a separate interface designed for better modularity.  They can be run with the following commands:

```bash
bazel run //src:test_enclave_basic_sgx_sim --
bazel run //src:test_enclave_interface_sgx_sim --
```

If SGX hardware is available, you can use the following commands instead:

```bash
bazel run //src:test_enclave_basic_sgx_hw --
bazel run //src:test_enclave_interface_sgx_hw --
```

Some guidance on writing new enclave tests:  To use the interface, you must use the provided constructor and the `finalize()` method.  In addition, to use the interface you must mark `//src/enclave:enclave_interface` as a dependency in the relevant `BUILD` file.  Then, ensure you have all the correct Asylo and SGX dependencies.  Finally, ensure you include the relevant abseil functions at the top of your test, as well as the relevant `absl` dependencies, so that your test correctly retrives the enclave name.  See the `test_enclave_interface.cc` file and the `test_enclave_interface` target in `src/BUILD` for an example.  

Note that the enclave interface differs from the way the enclave is implemented in the PSL project.  There, a function called `handle()` manages the actual processing of the CapsulePDUs.  While also doable here, CapsuleDB's current model does not require it.  Given that it is unclear whether there are any additional performance benefits, I did not implement it at this time.  

## Networking Test

Until this point, CapsuleDB and its associated tests have essentially been run as an application, it runs, finishes, and exits.  However, databases rarely operate that way, instead running as a server or service.  Using [ZeroMQ](https://zeromq.org/), CapsuleDB can be run as a server, receiving and processing requests regardless of their source.  You can run the basic networking test using two different terminals using the following commands:

```bash
MY_PROJECT=/path/to/capsuledb_repo

docker run -it --rm \
    -v bazel-cache:/root/.cache/bazel \
    -v "${MY_PROJECT}":/opt/my-project \
    -w /opt/my-project \
    keplerc/paranoid-asylo:latest 
```

You should now be in an interactive session within the container.  In a separate terminal window, execute `docker ps` to find the name of the container.  In your second terminal, run `docker exec -it <container_name> bash` to gain a second terminal within the container.

In your first terminal window, you will run the server.  Run the following command:

```bash
bazel run //src/network:capsuleDB_server
```

Then, in your second terminal, execute the test with:

```bash
bazel run //src:test_network
```

If it is successful, you should see the following result:

```text
Connecting to hello world server...
Connected
Sending payload
Start Get
Sending get request
Receiving get request
Retrieved value: Test value
```

Switching back to your first terminal, you will also be able to monitor and see CapsuleDB handling the incoming requests.  When running the test, you should see the following in your server window:

```text
Starting server
Server ready
Received new message
Received put request
Payload key: Test key and value: Test value
PUT key=Test key, value=Test value
Received new message
Received get request
GET key=Test key
```

To empty the database, simply stop the process in the server window and start it again with the same bazel run command.  

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
- [x] Add enclave compatability with `handle()` method (see PSL codebase for example)
THIS HAS BEEN ADJUSTED -> See enclave section of README.
- [x] Update README to include how to run enclave tests
- [x] Update local networking implementation to match that of PSL
- [x] See how janky the PSL networking stack was and implement simpler model
- [x] Update README to include how to run networking test
- [ ] Remove CapsuleDB implementation currently in PSL and convert to either Git submodule or use a Bazel load targeting this repo / version (hooray for modularity! :tada:)
- [ ] Update README with description of how CapsuleDB actually works under the hood

## Contributors

Lead: William Mullen

Contributors:
- Nivedha Krishnakumar
- Brian Wu
- Willis Wang
- Ryan Teoh
