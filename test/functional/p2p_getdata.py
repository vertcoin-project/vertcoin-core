#!/usr/bin/env python3
# Copyright (c) 2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test GETDATA processing behavior"""
from collections import defaultdict

from test_framework.messages import (
    CInv,
    msg_getdata,
)
<<<<<<< HEAD
from test_framework.mininode import (
    mininode_lock,
    P2PInterface,
)
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import wait_until

class P2PStoreBlock(P2PInterface):

=======
from test_framework.p2p import P2PInterface
from test_framework.test_framework import BitcoinTestFramework


class P2PStoreBlock(P2PInterface):
>>>>>>> bitcoin-0.21.1
    def __init__(self):
        super().__init__()
        self.blocks = defaultdict(int)

    def on_block(self, message):
        message.block.calc_sha256()
        self.blocks[message.block.sha256] += 1

<<<<<<< HEAD
=======

>>>>>>> bitcoin-0.21.1
class GetdataTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
<<<<<<< HEAD
        self.nodes[0].add_p2p_connection(P2PStoreBlock())
=======
        p2p_block_store = self.nodes[0].add_p2p_connection(P2PStoreBlock())
>>>>>>> bitcoin-0.21.1

        self.log.info("test that an invalid GETDATA doesn't prevent processing of future messages")

        # Send invalid message and verify that node responds to later ping
        invalid_getdata = msg_getdata()
        invalid_getdata.inv.append(CInv(t=0, h=0))  # INV type 0 is invalid.
<<<<<<< HEAD
        self.nodes[0].p2ps[0].send_and_ping(invalid_getdata)
=======
        p2p_block_store.send_and_ping(invalid_getdata)
>>>>>>> bitcoin-0.21.1

        # Check getdata still works by fetching tip block
        best_block = int(self.nodes[0].getbestblockhash(), 16)
        good_getdata = msg_getdata()
        good_getdata.inv.append(CInv(t=2, h=best_block))
<<<<<<< HEAD
        self.nodes[0].p2ps[0].send_and_ping(good_getdata)
        wait_until(lambda: self.nodes[0].p2ps[0].blocks[best_block] == 1, timeout=30, lock=mininode_lock)
=======
        p2p_block_store.send_and_ping(good_getdata)
        p2p_block_store.wait_until(lambda: p2p_block_store.blocks[best_block] == 1)

>>>>>>> bitcoin-0.21.1

if __name__ == '__main__':
    GetdataTest().main()
