#pragma once

template <typename T>
class ArrayProxy
{
public:
    VULKAN_HPP_CONSTEXPR ArrayProxy() VULKAN_HPP_NOEXCEPT
        : m_count(0)
        , m_ptr(nullptr)
    {}

    VULKAN_HPP_CONSTEXPR ArrayProxy(std::nullptr_t) VULKAN_HPP_NOEXCEPT
        : m_count(0)
        , m_ptr(nullptr)
    {}

    ArrayProxy(T& value) VULKAN_HPP_NOEXCEPT
        : m_count(1)
        , m_ptr(&value)
    {}

    template<typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(typename std::remove_const<T>::type& value) VULKAN_HPP_NOEXCEPT
        : m_count(1)
        , m_ptr(&value)
    {}

    ArrayProxy(uint32_t count, T* ptr) VULKAN_HPP_NOEXCEPT
        : m_count(count)
        , m_ptr(ptr)
    {}

    template<typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(uint32_t count, typename std::remove_const<T>::type* ptr) VULKAN_HPP_NOEXCEPT
        : m_count(count)
        , m_ptr(ptr)
    {}

    ArrayProxy(std::initializer_list<T> const& list) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(list.size()))
        , m_ptr(list.begin())
    {}

    template<typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::initializer_list<typename std::remove_const<T>::type> const& list) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(list.size()))
        , m_ptr(list.begin())
    {}

    ArrayProxy(std::initializer_list<T>& list) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(list.size()))
        , m_ptr(list.begin())
    {}

    template<typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::initializer_list<typename std::remove_const<T>::type>& list) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(list.size()))
        , m_ptr(list.begin())
    {}

    template <size_t N>
    ArrayProxy(std::array<T, N> const& data) VULKAN_HPP_NOEXCEPT
        : m_count(N)
        , m_ptr(data.data())
    {}

    template <size_t N, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::array<typename std::remove_const<T>::type, N> const& data) VULKAN_HPP_NOEXCEPT
        : m_count(N)
        , m_ptr(data.data())
    {}

    template <size_t N>
    ArrayProxy(std::array<T, N>& data) VULKAN_HPP_NOEXCEPT
        : m_count(N)
        , m_ptr(data.data())
    {}

    template <size_t N, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::array<typename std::remove_const<T>::type, N>& data) VULKAN_HPP_NOEXCEPT
        : m_count(N)
        , m_ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
    ArrayProxy(std::vector<T, Allocator> const& data) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(data.size()))
        , m_ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator> const& data) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(data.size()))
        , m_ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
    ArrayProxy(std::vector<T, Allocator>& data) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(data.size()))
        , m_ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
    ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator>& data) VULKAN_HPP_NOEXCEPT
        : m_count(static_cast<uint32_t>(data.size()))
        , m_ptr(data.data())
    {}

    const T* begin() const VULKAN_HPP_NOEXCEPT
    {
        return m_ptr;
    }

    const T* end() const VULKAN_HPP_NOEXCEPT
    {
        return m_ptr + m_count;
    }

    const T& front() const VULKAN_HPP_NOEXCEPT
    {
        VULKAN_HPP_ASSERT(m_count && m_ptr);
        return *m_ptr;
    }

    const T& back() const VULKAN_HPP_NOEXCEPT
    {
        VULKAN_HPP_ASSERT(m_count && m_ptr);
        return *(m_ptr + m_count - 1);
    }

    bool empty() const VULKAN_HPP_NOEXCEPT
    {
        return (m_count == 0);
    }

    uint32_t size() const VULKAN_HPP_NOEXCEPT
    {
        return m_count;
    }

    T* data() const VULKAN_HPP_NOEXCEPT
    {
        return m_ptr;
    }

private:
    uint32_t  m_count;
    T* m_ptr;
};