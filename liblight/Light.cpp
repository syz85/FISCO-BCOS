//
// Created by sunyuanzhen on 2022/9/26.
//

#include "Light.h"


bool isLight(std::shared_ptr<dev::blockchain::BlockChainInterface> const& blockChain, NodeID const& nodeId) {
    auto _nodeList = blockChain->lightList();
    auto it = std::find(_nodeList.begin(), _nodeList.end(), nodeId);
    if (it == _nodeList.end())
    {
        return false;
    }
    return true;
}
