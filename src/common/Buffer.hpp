#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include "common/utils/IntTypes.hpp"
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

namespace Common {
template <typename ValueType>
class buffer_iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using self_type = buffer_iterator;
    using value_type = ValueType;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;

    explicit buffer_iterator(pointer p)
        : _Ptr(p)
    {
    }

    reference operator*() const
    {
        return *_Ptr;
    }

    buffer_iterator& operator++()
    {
        ++_Ptr;
        return *this;
    }

    buffer_iterator operator++(int)
    { // postincrement
        buffer_iterator _Tmp = *this;
        ++*this;
        return (_Tmp);
    }

    buffer_iterator& operator--()
    {
        --_Ptr;
        return *this;
    }

    buffer_iterator operator--(int)
    { // postdecrement
        buffer_iterator _Tmp = *this;
        --*this;
        return (_Tmp);
    }

    buffer_iterator& operator+=(const difference_type _Off)
    { // increment by integer
        _Ptr += _Off;
        return (*this);
    }

    buffer_iterator operator+(const difference_type _Off) const
    { // return this + integer
        buffer_iterator _Tmp = *this;
        return (_Tmp += _Off);
    }

    buffer_iterator& operator-=(const difference_type _Off)
    { // decrement by integer
        return (*this += -_Off);
    }

    buffer_iterator operator-(const difference_type _Off) const
    { // return this - integer
        buffer_iterator _Tmp = *this;
        return (_Tmp -= _Off);
    }

    reference operator[](const difference_type _Off) const
    { // subscript
        return (*(*this + _Off));
    }

    difference_type operator-(const buffer_iterator& _Right) const
    { // return difference of iterators
        return (_Ptr - _Right._Ptr);
    }

    bool operator!=(const buffer_iterator& other) const
    {
        return _Ptr != other._Ptr;
    }

    bool operator==(const buffer_iterator& _Right) const
    { // test for iterator equality
        return (_Ptr == _Right._Ptr);
    }

    bool operator<(const buffer_iterator& _Right) const
    { // test if this < _Right
        return (_Ptr < _Right._Ptr);
    }

    bool operator>(const buffer_iterator& _Right) const
    { // test if this > _Right
        return (_Right < *this);
    }

    bool operator<=(const buffer_iterator& _Right) const
    { // test if this <= _Right
        return (!(_Right < *this));
    }

    bool operator>=(const buffer_iterator& _Right) const
    { // test if this >= _Right
        return (!(*this < _Right));
    }

private:
    pointer _Ptr;
};

template <class Alloc>
class base_buffer {
    struct compressed_pair final : private Alloc {
        using allocator_type = Alloc;
        using value_type = typename allocator_type::value_type;
        using iterator = buffer_iterator<value_type>;
        using const_iterator = buffer_iterator<const value_type>;
        using pointer = typename iterator::pointer;
        using const_pointer = typename const_iterator::pointer;

        constexpr allocator_type& first() noexcept
        {
            return *this;
        }

        constexpr const allocator_type& first() const noexcept
        {
            return *this;
        }

        compressed_pair(size_t cap, uint16_t head)
            : headreserved(head)
        {
            prepare(cap + head);
            readpos = writepos = headreserved;
        }

        compressed_pair(compressed_pair&& other) noexcept
            : flag(std::exchange(other.flag, 0))
            , headreserved(std::exchange(other.headreserved, 0))
            , capacity(std::exchange(other.capacity, 0))
            , readpos(std::exchange(other.readpos, 0))
            , writepos(std::exchange(other.writepos, 0))
            , data(std::exchange(other.data, nullptr))

        {
        }

        compressed_pair& operator=(compressed_pair&& other) noexcept
        {
            if (this != std::addressof(*other)) {
                if (nullptr != data)
                    first().deallocate(data, capacity);
                flag = std::exchange(other.flag, 0);
                headreserved = std::exchange(other.headreserved, 0);
                capacity = std::exchange(other.capacity, 0);
                readpos = std::exchange(other.readpos, 0);
                writepos = std::exchange(other.writepos, 0);
                data = std::exchange(other.data, nullptr);
            }
            return *this;
        }

        ~compressed_pair()
        {
            if (nullptr != data) {
                first().deallocate(data, capacity);
            }
        }

        size_t next_pow2(size_t x)
        {
            if (!(x & (x - 1))) {
                return x;
            }
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;
            return x + 1;
        }

        std::pair<pointer, size_t> prepare(size_t need)
        {
            assert(capacity >= writepos);
            size_t writeable = capacity - writepos;

            if (writeable >= need) {
                return std::pair { data + writepos, need };
            }

            if (writeable + readpos < need + headreserved) {
                auto required_size = writepos + need;
                required_size = next_pow2(required_size);
                auto tmp = first().allocate(required_size);
                if (nullptr != data) {
                    std::memcpy(tmp, data, writepos);
                    first().deallocate(data, capacity);
                }
                data = tmp;
                capacity = required_size;
            } else {
                size_t readable = writepos - readpos;
                if (readable != 0) {
                    assert(readpos >= headreserved);
                    std::memmove(data + headreserved, data + readpos, readable);
                }
                readpos = headreserved;
                writepos = readpos + readable;
            }
            return std::pair { data + writepos, need };
        }

        uint16_t flag = 0;
        uint16_t headreserved = 0;
        size_t capacity = 0;
        // read position
        size_t readpos = 0;
        // write position
        size_t writepos = 0;
        pointer data = nullptr;
    };

public:
    using allocator_type = Alloc;
    using value_type = typename allocator_type::value_type;
    using iterator = buffer_iterator<value_type>;
    using const_iterator = buffer_iterator<const value_type>;
    using pointer = typename iterator::pointer;
    using const_pointer = typename const_iterator::pointer;

    // websocket header max len 14 bytes.
    constexpr static uint16_t DEFAULT_HEAD_RESERVE = 16;
    constexpr static size_t DEFAULT_RESERVE = 128 - DEFAULT_HEAD_RESERVE;

    enum class seek_origin {
        Begin,
        Current,
    };

    base_buffer()
        : pair_(DEFAULT_RESERVE, DEFAULT_HEAD_RESERVE)
    {
    }

    base_buffer(size_t reserve)
        : pair_(reserve, DEFAULT_HEAD_RESERVE)
    {
    }

    base_buffer(size_t reserve, uint16_t head_reserve)
        : pair_(reserve, head_reserve)
    {
    }

    template <typename... Args>
    static std::unique_ptr<base_buffer> make_unique(Args&&... args)
    {
        return std::make_unique<base_buffer>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static std::shared_ptr<base_buffer> make_shared(Args&&... args)
    {
        return std::make_shared<base_buffer>(std::forward<Args>(args)...);
    }

    base_buffer(const base_buffer&) = delete;

    base_buffer& operator=(const base_buffer&) = delete;

    base_buffer(base_buffer&& other) = default;

    base_buffer& operator=(base_buffer&& other) = default;

    base_buffer clone()
    {
        base_buffer b { pair_.capacity, pair_.headreserved };
        b.set_flag(pair_.flag);
        b.write_back(data(), size());
        return b;
    }

    template <typename T>
    void write_back(const T* Indata, size_t count)
    {
        static_assert(std::is_trivially_copyable<T>::value, "type T must be trivially copyable");
        if (nullptr == Indata || 0 == count)
            return;
        size_t n = sizeof(T) * count;
        auto space = pair_.prepare(n);
        memcpy(space.first, Indata, space.second);
        pair_.writepos += n;
    }

    void write_back(char c)
    {
        *(pair_.prepare(1).first) = c;
        ++pair_.writepos;
    }

    void unsafe_write_back(char c)
    {
        *(pair_.data + (pair_.writepos++)) = c;
    }

    template <typename T>
    bool write_front(const T* Indata, size_t count) noexcept
    {
        static_assert(std::is_trivially_copyable<T>::value, "type T must be trivially copyable");
        if (nullptr == Indata || 0 == count)
            return false;

        size_t n = sizeof(T) * count;

        if (n > pair_.readpos) {
            return false;
        }

        pair_.readpos -= n;
        auto* buff = reinterpret_cast<T*>(std::addressof(*begin()));
        memcpy(buff, Indata, n);
        return true;
    }

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    bool write_chars(T value)
    {
        static constexpr size_t MAX_NUMBER_2_STR = 44;
        auto space = pair_.prepare(MAX_NUMBER_2_STR);
        auto* b = space.first;
#ifndef _MSC_VER // std::to_chars in C++17: gcc and clang only integral types supported
        if constexpr (std::is_floating_point_v<T>) {
            int len = std::snprintf(b, MAX_NUMBER_2_STR, "%.16g", value);
            if (len < 0)
                return false;
            commit(len);
            return true;
        } else
#endif
        {
            auto* e = b + MAX_NUMBER_2_STR;
            auto res = std::to_chars(b, e, value);
            if (res.ec != std::errc())
                return false;
            commit(res.ptr - b);
            return true;
        }
    }

    template <typename T>
    bool read(T* Outdata, size_t count) noexcept
    {
        static_assert(std::is_trivially_copyable<T>::value, "type T must be trivially copyable");
        if (nullptr == Outdata || 0 == count)
            return false;

        size_t n = sizeof(T) * count;

        if (pair_.readpos + n > pair_.writepos) {
            return false;
        }

        auto* buff = std::addressof(*begin());
        memcpy(Outdata, buff, n);
        pair_.readpos += n;
        return true;
    }

    void consume(std::size_t n) noexcept
    {
        seek(n);
    }

    // set read or forward read pos
    bool seek(size_t offset, seek_origin s = seek_origin::Current) noexcept
    {
        switch (s) {
        case seek_origin::Begin:
            if (offset > pair_.writepos)
                return false;
            pair_.readpos = offset;
            break;
        case seek_origin::Current:
            if (pair_.readpos + offset > pair_.writepos)
                return false;
            pair_.readpos += offset;
            break;
        default:
            assert(false);
            return false;
        }
        return true;
    }

    void clear() noexcept
    {
        pair_.flag = 0;
        pair_.writepos = pair_.readpos = pair_.headreserved;
    }

    template <typename ValueType>
    void set_flag(ValueType v) noexcept
    {
        pair_.flag |= static_cast<uint16_t>(v);
    }

    template <typename ValueType>
    bool has_flag(ValueType v) const noexcept
    {
        return ((pair_.flag & static_cast<uint16_t>(v)) != 0);
    }

    template <typename ValueType>
    void clear_flag(ValueType v) noexcept
    {
        pair_.flag &= ~static_cast<uint16_t>(v);
    }

    void commit(std::size_t n) noexcept
    {
        pair_.writepos += n;
        assert(pair_.writepos <= pair_.capacity);
        if (pair_.writepos >= pair_.capacity) {
            pair_.writepos = pair_.capacity;
        }
    }

    std::pair<pointer, size_t> prepare(size_t need)
    {
        return pair_.prepare(need);
    }

    pointer revert(size_t n) noexcept
    {
        assert(pair_.writepos >= (pair_.readpos + n));
        if (pair_.writepos >= n) {
            pair_.writepos -= n;
        }
        return (pair_.data + pair_.writepos);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator { pair_.data + pair_.readpos };
    }

    iterator begin() noexcept
    {
        return iterator { pair_.data + pair_.readpos };
    }

    const_iterator end() const noexcept
    {
        return const_iterator { pair_.data + pair_.writepos };
    }

    iterator end() noexcept
    {
        return iterator { pair_.data + pair_.writepos };
    }

    pointer data() noexcept
    {
        return std::addressof(*begin());
    }

    const_pointer data() const noexcept
    {
        return std::addressof(*begin());
    }

    // readable size
    size_t size() const noexcept
    {
        return pair_.writepos - pair_.readpos;
    }

    size_t capacity() const noexcept
    {
        return pair_.capacity;
    }

    size_t reserved() const noexcept
    {
        return pair_.headreserved;
    }

private:
    compressed_pair pair_;
};

enum class BufferFlag : u8 {
    NONE = 0,
    PACK_SIZE = 1 << 0,
    CLOSE = 1 << 1,
    CHUNKED = 1 << 2,
    BROADCAST = 1 << 3,
    WEBSOCKET_PING = 1 << 4,
    WEBSOCKET_PONG = 1 << 5,
    WEBSOCKET_TEXT = 1 << 6,

    BUFFER_FLAG_MAX
};

using Buffer = base_buffer<std::allocator<char>>;
using BufferPtr = std::unique_ptr<Buffer>;
};

#endif // BUFFER_HPP_