#pragma once

#include <stdint.h>

#define Bit(x) (1 << x)


template<class T>
struct Flags
{
	T value;

	constexpr Flags(const T& v) : value(v) {}

	constexpr inline operator bool() { return (uint32_t)value; }
	constexpr inline operator T() { return value; }
	constexpr inline Flags<T> operator|(const T& other) { return static_cast<T>(static_cast<uint32_t>(value) | static_cast<uint32_t>(other)); }
	constexpr inline Flags<T> operator|(const Flags<T>& other) { return static_cast<T>(static_cast<uint32_t>(value) | static_cast<uint32_t>(other.value)); }
	constexpr inline Flags<T> operator&(const T& other) { return static_cast<T>(static_cast<uint32_t>(value) & static_cast<uint32_t>(other)); }
	constexpr inline Flags<T> operator&(const Flags<T>& other) { return static_cast<T>(static_cast<uint32_t>(value) & static_cast<uint32_t>(other.value)); }

	constexpr inline Flags<T>& operator|=(const T& other)
	{ 
		value = static_cast<T>(static_cast<uint32_t>(value) | static_cast<uint32_t>(other));
		return *this; 
	}
	constexpr inline Flags<T>& operator|=(const Flags<T>& other)
	{
		value = static_cast<T>(static_cast<uint32_t>(value) | static_cast<uint32_t>(other.value));
		return *this;
	}
	constexpr inline Flags<T>& operator&=(const T& other)
	{
		value = static_cast<T>(static_cast<uint32_t>(value) & static_cast<uint32_t>(other));
		return *this;
	}
	constexpr inline Flags<T>& operator&=(const Flags<T>& other)
	{
		value = static_cast<T>(static_cast<uint32_t>(value) & static_cast<uint32_t>(other.value));
		return *this;
	}
};

enum class ImageFormat : uint8_t
{
	RGBA8, RGB8, RG8, R8,
	RGBA16, RGB16, RG16, R16,
	RGBA32, RGB32, RG32, R32,
	Depth32, Depth16,
	Stincel8,
	Depth24Stencel8
};

enum class ImageType : uint8_t
{
	e1D,
	e2D,
	e3D
};

enum class ImageViewType : uint8_t
{
	e1D, e2D, e3D,
	e1DArray, e2DArray,
	Cube,CubeArray
};

enum class ImageViewAspect : uint8_t
{
	Color,
	Depth,
	Stencil
};

enum class ResourceAccessRate : uint8_t
{
	Rare,
	Frequent
};

enum class ResourceAccessibilityBits : uint8_t
{
	None = 0,
	Write = Bit(0),
	Read = Bit(1)

};


enum class BufferUsageBits : uint32_t
{
	NoUse = 0,
	TransferSrc = Bit(0),
	TransferDst = Bit(1),
	VertexBuffer = Bit(2),
	IndexBuffer = Bit(3),
	UniformBuffer = Bit(4),
	StorageBuffer = Bit(5)
};

typedef Flags<BufferUsageBits> BufferUsageFlags;
typedef Flags<ResourceAccessibilityBits> ResourceAccessibilityFlags;

inline static constexpr BufferUsageFlags operator&(const BufferUsageBits& a, const BufferUsageBits& b)
{
	return BufferUsageFlags(a) & b;
}
inline static constexpr BufferUsageFlags operator|(const BufferUsageBits& a, const BufferUsageBits& b)
{
	return BufferUsageFlags(a) | b;
}
inline static constexpr ResourceAccessibilityFlags operator&(const ResourceAccessibilityBits& a, const ResourceAccessibilityBits& b)
{
	return ResourceAccessibilityFlags(a) & b;
}
inline static constexpr ResourceAccessibilityFlags operator|(const ResourceAccessibilityBits& a, const ResourceAccessibilityBits& b)
{
	return ResourceAccessibilityFlags(a) | b;
}