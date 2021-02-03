#pragma once

#include "span"

// get a clamped view on a contiguous container

namespace detail {
	template<class T>
	gsl::span<T> clamp_span_impl(gsl::span<T> s, std::ptrdiff_t offset, std::ptrdiff_t count) noexcept {
		offset = std::clamp<std::ptrdiff_t>(offset, 0, s.size());
		count = std::clamp<std::ptrdiff_t>(count, 0, s.size() - offset);
		return s.subspan(offset, count);
	}
}

namespace fek {
	template<class C>
	std::span<typename C::value_type>  clamp_span(C& c, std::ptrdiff_t offset, std::ptrdiff_t count) noexcept {
	//auto clamp_span(C& c, long offset, long count) noexcept -> std::span<std::declval<C>().data()> {
		return ::detail::clamp_span_impl(make_span(c), offset, count);
	}
	template<class C>
	std::span<const typename C::value_type>  clamp_span(const C& c, std::ptrdiff_t offset, std::ptrdiff_t count) noexcept {
	//auto clamp_span(C& c, long offset, long count) noexcept -> gsl::span<const std::declval<C>().data()> {
		return ::detail::clamp_span_impl(make_span(c), offset, count);
	}

	template<class T, size_t N>
	std::span<T> clamp_span( T(&arr)[N], std::ptrdiff_t offset, std::ptrdiff_t count) {
		return ::detail::clamp_span_impl(make_span(arr), offset, count);
	}
	template<class T, size_t N>
	std::span<const T> clamp_span( const T(&arr)[N], std::ptrdiff_t offset, std::ptrdiff_t count) {
		return ::detail::clamp_span_impl(make_span(arr), offset, count);
	}
}
