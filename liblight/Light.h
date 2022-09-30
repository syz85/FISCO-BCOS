//
// Created by sunyuanzhen on 2022/9/26.
//

#pragma once
#include <libblockchain/BlockChainInterface.h>

using NodeID = dev::h512;

bool isLight(std::shared_ptr<dev::blockchain::BlockChainInterface> const& blockChain, NodeID const& nodeId);
