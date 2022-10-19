/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */
/**
 * @brief : Downloading block queue
 */

#pragma once
#include "Common.h"
#include <libblockchain/BlockChainInterface.h>
#include <libdevcore/Guards.h>
#include <libethcore/Block.h>
#include <climits>
#include <queue>
#include <set>
#include <vector>
#include <libsync/DownloadingBlockQueue.h>

namespace dev
{
namespace sync
{
struct HeaderQueueCmp
{
    bool operator()(HeaderPtr const& _a, HeaderPtr const& _b)
    {
        // increase order
        return _a->number() > _b->number();
    }
};


class DownloadingHeaderQueue
{
public:
    using ShardPtr = std::shared_ptr<DownloadBlocksShard>;
    using ShardPtrVec = std::list<ShardPtr>;

public:
    DownloadingHeaderQueue(std::shared_ptr<dev::blockchain::BlockChainInterface> _blockChain,
        PROTOCOL_ID, NodeID const& _nodeId)
      : m_blockChain(_blockChain),
        m_nodeId(_nodeId),
        m_headers(),
        m_headerBuffer(std::make_shared<ShardPtrVec>())
    {}

    DownloadingHeaderQueue()
      : m_blockChain(nullptr), m_headers(), m_headerBuffer(std::make_shared<ShardPtrVec>())
    {}

    void pushHeader(RLP const& _rlps);

    /// Is the queue empty?
    bool empty();

    /// get the total size of th block queue
    size_t size();

    /// 执行header queue的pop
    void popHeader();

    /// 获取header queue的top
    HeaderPtr topHeader(bool isFlushBuffer = false);

    /// 情况header的缓存
    void clearHeader();

    /// 情况header的queue
    void clearHeaderQueue();

    /// flush m_headerBuffer into queue
    void flushHeaderBufferToQueue();

    void clearFullHeaderQueueIfNotHas(int64_t _blockNumber);

    void setMaxBlockQueueSize(int64_t const& _maxBlockQueueSize)
    {
        m_maxBlockQueueSize = _maxBlockQueueSize;
    }

    int64_t maxRequestBlocks() const { return m_maxRequestBlocks; }

private:
    bool flushOneHeaderShard(ShardPtr _headersShard);

private:
    std::shared_ptr<dev::blockchain::BlockChainInterface> m_blockChain;
    NodeID m_nodeId;
    std::priority_queue<HeaderPtr, HeaderPtrVec, HeaderQueueCmp> m_headers;
    std::shared_ptr<ShardPtrVec> m_headerBuffer;

    mutable SharedMutex x_headers;
    mutable SharedMutex x_headerBuffer;

    // default max block buffer size is 512MB
    int64_t m_maxBlockQueueSize = 512 * 1024 * 1024;
    // the memory size occupied by the sync module
    std::atomic<int64_t> m_blockQueueSize = {0};
    // the max number of blocks this node can requested to
    std::atomic<int64_t> m_maxRequestBlocks = {8};

private:
    bool isNewerHeader(std::shared_ptr<dev::eth::BlockHeader> _header);
};

}  // namespace sync
}  // namespace dev