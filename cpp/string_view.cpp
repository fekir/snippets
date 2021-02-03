#pragma once

#include <stdexcept>
#include <cstddef>
#include <iosfwd>

namespace detail {
	template<class CharT>
	constexpr std::size_t len(const CharT* str) {
		return *str ? 1 + len(str+1) : 0;
	}
	template<class CharT>
	constexpr int compare(const CharT* data1, const CharT* data2, std::size_t len) {
		return *data1 == *data2 and len > 1 ? compare(data1+1, data2+1, len-1) : int(unsigned(*data1) - unsigned(*data2));
	}
}

namespace fek{
	// compatible subset of std::string_view for c++11
	template<class CharT>
	class basic_string_view {
		static_assert(sizeof(CharT) <= sizeof(int), "change impl of detail::compare");

		using difference_type = ptrdiff_t;
		using size_type = std::size_t;
		using value_type = CharT;
		using pointer = const value_type*;
		using const_pointer = const value_type*;
		using reference = const value_type&;
		using const_reference = const value_type&;
		using const_iterator = pointer;

		pointer m_begin;
		size_type m_length;

		public:
		constexpr static size_type npos = -1;
		constexpr basic_string_view() noexcept : m_begin(nullptr), m_length(0) {}
		constexpr basic_string_view(const CharT* s, size_type count) noexcept : m_begin(s), m_length(count) {}
		constexpr basic_string_view(const CharT* s) noexcept : m_begin(s), m_length(detail::len(s)) {}

		constexpr const_iterator begin() const noexcept {
			return m_begin;
		}

		constexpr size_type size() const noexcept {
			return this->m_length;
		}
		constexpr const_iterator end() const noexcept {
			return this->begin() + this->size();
		}
		constexpr pointer data() const noexcept {
			return &(*this->begin());
		}
		constexpr bool empty() const noexcept {
			return this->size() == 0;
		}
		constexpr const_reference operator[](size_type pos) const {
			return this->data()[pos];
		}
		constexpr basic_string_view substr(size_type pos = 0, size_type count = npos ) const {
			return pos > this->size() ?
				throw std::out_of_range("basic_string_view substr") :
				basic_string_view(this->data() + pos, Base::cpp14::min(count, this->size() - pos));
		}
		constexpr int compare(basic_string_view v) const noexcept {
			return detail::compare(this->data(), v.data(), Base::cpp14::min(this->size(), v.size())) == 0 ?
				this->size() - v.size() :
				detail::compare(this->data(), v.data(), Base::cpp14::min(this->size(), v.size()));
		}
		constexpr int compare(size_type pos1, size_type count1, basic_string_view v) const noexcept {
			return this->substr(pos1, count1).compare(v);
		}
		constexpr int compare(size_type pos1, size_type count1, basic_string_view v, size_type pos2, size_type count2) const noexcept {
			return this->substr(pos1, count1).compare(v.substr(pos2, count2));
		}
		constexpr int compare(const CharT* s) const noexcept {
			return this->compare(basic_string_view(s));
		}
		constexpr int compare(size_type pos1, size_type count1, const CharT* s) const noexcept {
			return this->substr(pos1, count1).compare(basic_string_view(s));
		}
		constexpr int compare(size_type pos1, size_type count1, const CharT* s, size_type count2) const noexcept {
			return this->substr(pos1, count1).compare(basic_string_view(s, count2));
		}

		// notice: implicit conversion from string to string_view belongs to std::string, but we cant modify it
		basic_string_view(const std::basic_string<CharT>& s) noexcept : m_begin(s.data()), m_length(s.size()) {}
		// notice: explicit conversion to string from string_view belongs to std::string, but we cant modify it
		explicit operator std::basic_string<CharT>() const {
			return std::basic_string<CharT>(this->data(), this->size());
		}
	};

	template<class CharT>
	constexpr bool operator==( basic_string_view<CharT> lhs, basic_string_view<CharT> rhs ) noexcept {
		return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
	}
	template<class CharT>
	constexpr bool operator!=( basic_string_view<CharT> lhs, basic_string_view<CharT> rhs ) noexcept {
		return !(lhs == rhs);
	}

	template<class CharT>
	constexpr bool operator<( basic_string_view<CharT> lhs, basic_string_view<CharT> rhs ) noexcept {
		return lhs.compare(rhs) < 0;
	}
	template<class CharT>
	std::basic_ostream<CharT, std::char_traits<CharT>>& operator<<(std::basic_ostream<CharT, std::char_traits<CharT>>& os, basic_string_view<CharT> v);

	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;
	using u16string_view = basic_string_view<char16_t>;
	using u32string_view = basic_string_view<char32_t>;

	// in .cpp file, include iostream
	template<class CharT>
	std::basic_ostream<CharT, std::char_traits<CharT>>& operator<<(std::basic_ostream<CharT, std::char_traits<CharT>>& os, basic_string_view<CharT> v) {
		return os.write(v.data(), v.size());
	}

	template std::ostream& operator<<(std::ostream& os, string_view v);
	template std::wostream& operator<<(std::wostream& os, wstring_view v);
}
