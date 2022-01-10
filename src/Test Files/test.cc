#include <string>
#include <iostream>

#include "../engine.hh"

int main()
{
    // Create instance with memtable/blocksize capacity of 2 key-value pairs
    CapsuleDB instance = spawnDB(50);
    instance.benchmark2();
    //std::string check_value = instance.get("6164213995759621");
    //std::cout << "OUTPUT value=" << check_value << "\n\n";
    instance.benchmark_verify();
    // // Put testval at testkey
    // CapsuleDB instance = spawnDB(2);
    // kvs_payload kvsp_put = {};
    // std::string requestedVal;

    // // Put testval2 at testkey2
    // kvsp_put.key = "testkey2";
    // kvsp_put.value = "testval2";
    // kvsp_put.txn_timestamp = 1;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey2");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval5 at testkey5
    // kvsp_put.key = "testkey5";
    // kvsp_put.value = "testval5";
    // kvsp_put.txn_timestamp = 2;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey5");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval4 at testkey4, causing memtable to write out to Level 0
    // kvsp_put.key = "testkey4";
    // kvsp_put.value = "testval4";
    // kvsp_put.txn_timestamp = 3;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey4");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval1 at testkey1
    // kvsp_put.key = "testkey1";
    // kvsp_put.value = "testval1";
    // kvsp_put.txn_timestamp = 4;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey1");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval6 at testkey6, causing memtable to write out to Level 0. This should also cause compaction to Level 1.
    // kvsp_put.key = "testkey6";
    // kvsp_put.value = "testval6";
    // kvsp_put.txn_timestamp = 5;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey6");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval3 at testkey1, overriding testval
    // kvsp_put.key = "testkey1";
    // kvsp_put.value = "testval3";
    // kvsp_put.txn_timestamp = 6;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey1");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval7 at testkey7, causing memtable to write out to Level 0. 
    // kvsp_put.key = "testkey7";
    // kvsp_put.value = "testval7";
    // kvsp_put.txn_timestamp = 7;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey7");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval8 at testkey1, overriding testval
    // kvsp_put.key = "testkey1";
    // kvsp_put.value = "testval8";
    // kvsp_put.txn_timestamp = 8;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey1");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval9 at testkey9, causing memtable to write out to Level 0. This should also cause compaction to Level 1.
    // kvsp_put.key = "testkey9";
    // kvsp_put.value = "testval9";
    // kvsp_put.txn_timestamp = 9;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey9");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";


    // // Expected Outcome:
    // // Memtable = {[9, 9]}
    // // L0 = {}  
    // // L1 = {[[1, 8] [2, 2]], [[4, 4] [5, 5]], [[6, 6] [7, 7]]}

    // // Get value of testkey1 (should be testval8), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey1");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey2 (should be testval2), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey2");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey4 (should be testval4), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey4");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey5 (should be testval5), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey5");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey6 (should be testval6), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey6");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey7 (should be testval7), shouldn't be in memtable, should have to check Level 1
    // requestedVal = instance.get("testkey7");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";
    // // Get value of testkey9 (should be testval9), should be in memtable
    // requestedVal = instance.get("testkey9");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";

    // // Put testval10 at testkey10, causing memtable to write out to Level 0. This should also cause compaction to Level 1.
    // kvsp_put.key = "testkey10";
    // kvsp_put.value = "testval10";
    // kvsp_put.txn_timestamp = 10;
    // instance.put(&kvsp_put);
    // requestedVal = instance.get("testkey10");
    // std::cout << "OUTPUT value=" << requestedVal << "\n\n";


    // Expected Outcome:
    // Memtable = {[9, 9], [10, 10]}
    // L0 = {}  
    // L1 = {[[1, 8] [2, 2]], [[4, 4] [5, 5]], [[6, 6] [7, 7]]}

}
