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
 * @brief : Downloading header queue
 */

#include "DownloadingHeaderQueue.h"
#include "Common.h"

#ifndef FISCO_DEBUG
#include "gperftools/malloc_extension.h"
#endif

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::sync;


/// Push a block
void DownloadingHeaderQueue::pushHeader(RLP const& _rlps)
{
    WriteGuard l(x_headerBuffer);
    if (m_headerBuffer->size() >= c_maxDownloadingBlockQueueBufferSize)
    {
        SYNC_LOG(WARNING) << LOG_BADGE("Download") << LOG_BADGE("HeaderSync")
                          << LOG_DESC("DownloadingBlockQueueHeaderBuffer is full")
                          << LOG_KV("queueSize", m_headerBuffer->size());
        return;
    }
    ShardPtr headersShard = make_shared<DownloadBlocksShard>(0, 0, _rlps.data().toBytes());
    m_headerBuffer->emplace_back(headersShard);
}

/// Is the queue empty?
bool DownloadingHeaderQueue::empty()
{
    bool res;
    {
        ReadGuard l1(x_headerBuffer);
        ReadGuard l2(x_headers);
        res = m_headers.empty() && (!m_headerBuffer || m_headerBuffer->empty());
    }
    return res;
}

size_t DownloadingHeaderQueue::size()
{
    ReadGuard l1(x_headerBuffer);
    ReadGuard l2(x_headers);
    size_t s = (!m_headerBuffer ? 0 : m_headerBuffer->size()) + m_headers.size();
    return s;
}


void DownloadingHeaderQueue::popHeader()
{
    WriteGuard l(x_headers);
    if (!m_headers.empty())
    {
        m_headers.pop();
    }
    // block queue is empty, reset m_maxRequestBlocks to c_maxRequestBlocks
    else
    {
        m_maxRequestBlocks = c_maxRequestBlocks;
    }
}


HeaderPtr DownloadingHeaderQueue::topHeader(bool isFlushBuffer)
{
    if (isFlushBuffer)
        flushHeaderBufferToQueue();

    ReadGuard l(x_headers);
    if (!m_headers.empty())
        return m_headers.top();
    else
        return nullptr;
}


void DownloadingHeaderQueue::clearHeader()
{
    {
        WriteGuard l(x_headerBuffer);
        m_headerBuffer->clear();
    }

    clearHeaderQueue();
}


void DownloadingHeaderQueue::clearHeaderQueue()
{
    WriteGuard l(x_headers);
    std::priority_queue<HeaderPtr, HeaderPtrVec, HeaderQueueCmp> emptyQueue;
    swap(m_headers, emptyQueue);  // Does memory leak here ?
    // give back the memory to os
#ifndef FISCO_DEBUG
    MallocExtension::instance()->ReleaseFreeMemory();
#endif
}

void DownloadingHeaderQueue::flushHeaderBufferToQueue()
{
    WriteGuard l(x_headerBuffer);
    bool ret = true;
    while (m_headerBuffer->size() > 0 && ret)
    {
        auto headersShard = m_headerBuffer->front();
        m_headerBuffer->pop_front();
        ret = flushOneHeaderShard(headersShard);
    }
}


bool DownloadingHeaderQueue::flushOneHeaderShard(ShardPtr _headersShard)
{
    // pop buffer into queue
    WriteGuard l(x_headers);
    if (m_headers.size() >= c_maxDownloadingBlockQueueSize)  // TODO not to use size to
                                                            // control insert
    {
        SYNC_LOG(TRACE) << LOG_BADGE("Download") << LOG_BADGE("HeaderSync")
                        << LOG_DESC("DownloadingHeaderQueueBuffer is full")
                        << LOG_KV("queueSize", m_headers.size());

        return false;
    }

    SYNC_LOG(TRACE) << LOG_BADGE("Download") << LOG_BADGE("HeaderSync")
                    << LOG_DESC("Decoding header buffer")
                    << LOG_KV("headersShardSize", _headersShard->blocksBytes.size());


    RLP const& rlps = RLP(ref(_headersShard->blocksBytes));
    unsigned itemCount = rlps.itemCount();
    size_t successCnt = 0;
    for (unsigned i = 0; i < itemCount; ++i)
    {
        try
        {
            // TODO: syz: 这里的 HeaderData 对不对？
            shared_ptr<BlockHeader> header = make_shared<BlockHeader>(rlps[i].toBytes(), HeaderData);
            if (isNewerHeader(header))
            {
                successCnt++;
                m_headers.push(header);
            }
        }
        catch (std::exception& e)
        {
            SYNC_LOG(WARNING) << LOG_BADGE("Download") << LOG_BADGE("HeaderSync")
                              << LOG_DESC("Invalid header RLP") << LOG_KV("reason", e.what())
                              << LOG_KV("RLPDataSize", rlps.data().size());
            continue;
        }
    }

    SYNC_LOG(TRACE) << LOG_BADGE("Download") << LOG_BADGE("HeaderSync")
                    << LOG_DESC("Flush buffer to header queue") << LOG_KV("import", successCnt)
                    << LOG_KV("rcv", itemCount) << LOG_KV("downloadHeaderQueue", m_headers.size());
    return true;
}


void DownloadingHeaderQueue::clearFullHeaderQueueIfNotHas(int64_t _blockNumber)
{
    bool needClear = false;
    {
        ReadGuard l(x_headers);

        if (m_headers.size() == c_maxDownloadingBlockQueueSize &&
            m_headers.top()->number() > _blockNumber)
            needClear = true;
    }
    if (needClear)
        clearHeaderQueue();
}


bool DownloadingHeaderQueue::isNewerHeader(shared_ptr<BlockHeader> _header)
{
    if (m_blockChain != nullptr && _header->number() <= m_blockChain->number())
        return false;

    // if (block->header()->)
    // if //Check sig list
    // return false;

    return true;
}