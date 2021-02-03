#include <vector>
#include <algorithm>
#define NDEBUG
#include <cassert>

// remove element from container that match predicate
// -> container size changes
// if temporary -> reuse space and delete elements that do not match
// no overload for const -> but provide different function that copies elements out.

// shortcomings
// * only vector as input -> easy to make more generic as long as algorithms works, what about array?
// * not_fn in c++11? ->  simplify https://en.cppreference.com/w/cpp/utility/functional/not_fn, leave invoke, constexpr out at first
// * output is a new vector
//   might want to append to an existing different container -> should use-in-out param (or iterator)? but then it's a copy_if

// needs ownership + mutability (for shrinking)
// filter_copy needs only iterability, but different interface
// filter_copy could work on span (to reduce templates), but unsure how to overload

namespace fek {
	template <class P>
	std::vector<int> filter(std::vector<int>& o, P pred){
		const auto it = std::stable_partition(o.begin(), o.end(),[&](int i){ return !pred(i);});
		std::vector<int> vec;
		vec.reserve(std::distance(o.begin(), it));
		std::copy(std::make_move_iterator(it), std::make_move_iterator(o.end()), std::back_inserter(vec));
		o.erase(it, o.end());
		return vec;
	}

	// opt for temporaries/moved containers -> make in-place op
	template <class P>
	std::vector<int> filter(std::vector<int>&& o, P pred){
		const auto it = std::remove_if(o.begin(), o.end(), [&](int i){ return !pred(i);});
		o.erase(it, o.end());
		return std::move(o);
	}

	// convenience in case we do not want/cant to mutate original vector
	template <class P>
	std::vector<int> filter_copy(const std::vector<int>& o, P pred){
		std::vector<int> vec;
		std::copy_if(o.begin(), o.end(), std::back_inserter(vec), pred);
		return vec;
	}
}

int main(){
	std::vector<int> v = {1, 2,3,4,5};
	{
		auto v2 = filter_copy(v, [](int i){ return i == 5;});
		assert(v2.size() == 1);
		assert(v.size() == 5);
	}
	{
		auto v2 = filter(v, [](int i){ return i == 4;});
		assert(v2.size() == 1);
		assert(v.size() == 4);
	}
	{
		auto v2 = filter(std::move(v), [](int i){ return i == 3;});
		assert(v2.size() == 1);
	}
}
