// Copyright (c) 2012-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018 The Sphinx Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VERSSPHX_H
#define BITCOIN_VERSSPHX_H

/**
 * network protocol verssphxing
 */

static const int PROTOCOL_VERSSPHX = 61403;

//! initial proto verssphx, to be increased after verssphx/verack negotiation
static const int INIT_PROTO_VERSSPHX = 614;

//! In this verssphx, 'getheaders' was introduced.
static const int GETHEADERS_VERSSPHX = 61403;

//! disconnect from peers older than this proto verssphx
static const int MIN_PEER_PROTO_VERSSPHX_BEFORE_ENFORCEMENT = 61402;
static const int MIN_PEER_PROTO_VERSSPHX_AFTER_ENFORCEMENT = 61403;

//! nTime field added to CAddress, starting with this verssphx;
//! if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSSPHX = 61403;

//! BIP 0031, pong message, is enabled for all verssphxs AFTER this one
static const int BIP0031_VERSSPHX = 61403;

//! "mempool" command, enhanced "getdata" behavior starts with this verssphx
static const int MEMPOOL_GD_VERSSPHX = 61403;

//! "filter*" commands are disabled without NODE_BLOOM after and including this verssphx
static const int NO_BLOOM_VERSSPHX = 61403;


#endif // BITCOIN_VERSSPHX_H