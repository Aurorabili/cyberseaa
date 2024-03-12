#ifndef CONSTBUFFERHOLDER_HPP_
#define CONSTBUFFERHOLDER_HPP_

#include <cstddef>
#include <forward_list>
#include <vector>

#include <asio/buffer.hpp>

#include "common/utils/IntTypes.hpp"

namespace Core {

using MessageSizeType = u32;

class ConstBuffersHolder {
public:
    static constexpr size_t max_count = 64;

    ConstBuffersHolder() = default;

    void push_back(const char* data, size_t len, bool close)
    {
        close_ = close ? true : close_;
        buffers_.emplace_back(data, len);
        ++count_;
    }

    void push()
    {
        ++count_;
    }

    void push_slice(MessageSizeType header, const char* data, size_t len)
    {
        headers_.emplace_front(header);
        MessageSizeType& value = headers_.front();
        buffers_.emplace_back(reinterpret_cast<const char*>(&value), sizeof(value));
        if (len > 0)
            buffers_.emplace_back(data, len);
    }

    const auto& buffers() const
    {
        return buffers_;
    }

    size_t size() const
    {
        return buffers_.size();
    }

    // hold buffer's count
    size_t count() const
    {
        return count_;
    }

    void clear()
    {
        close_ = false;
        count_ = 0;
        buffers_.clear();
        headers_.clear();
    }

    bool close() const
    {
        return close_;
    }

private:
    bool close_ = false;
    size_t count_ = 0;
    std::vector<asio::const_buffer> buffers_;
    std::forward_list<MessageSizeType> headers_;
};

template <class BufferSequence>
class BuffersRef {
    BufferSequence const& buffers_;

public:
    using value_type = typename BufferSequence::value_type;

    using const_iterator = typename BufferSequence::const_iterator;

    BuffersRef(BuffersRef const&) = default;

    explicit BuffersRef(BufferSequence const& buffers)
        : buffers_(buffers)
    {
    }

    const_iterator
    begin() const
    {
        return buffers_.begin();
    }

    const_iterator
    end() const
    {
        return buffers_.end();
    }
};

// Return a reference to a buffer sequence
template <class BufferSequence>
BuffersRef<BufferSequence>
make_buffers_ref(BufferSequence const& buffers)
{
    return BuffersRef<BufferSequence>(buffers);
}
}

#endif // CONSTBUFFERHOLDER_HPP_