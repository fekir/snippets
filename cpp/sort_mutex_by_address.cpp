#include <algorithm>
#include <mutex>


// with std::scoped_lock helgrind triggers a warning if arguments are passed in different order
// it is not (probably not) an error (in particular a live-lock), as std::scoped_lock should take care of it,
// but it is annoying
// as std::mutex are not moveable, addresses are an invariant (notice: does not hold for mutexes outside of std)
// thus it can be used as sorting criteria
// works only for mutexes of same type

template<class indices, class mutexes, std::size_t... I>
auto create_lock_i(const indices& idxs, mutexes& ms, std::index_sequence<I...>) {
	return std::scoped_lock(*ms[idxs[I]]...);
}

template<int N>
constexpr auto init_indices() noexcept {
	std::array<int,N> indexes{};
	for(int i = 0; i != N; ++i){
		indexes[i] = i;
	}
	return indexes;
}

template <class...Args>
auto create_lock(Args&... args){
	std::array addresses = {&args...}; // not constexpr!!!
	auto indexes = [&](){
		auto indexes = init_indices<sizeof...(args)>();
		std::sort(std::begin(indexes), std::end(indexes), [&](int i1, int i2){
				return std::less<void>()(addresses[i1], addresses[i2]);
				});
		return indexes;
	}();
	// for non homogenuos types tuple would be better,
	// if sorting would be constexpr, otherwise some sort of type erasure...
	//auto tuple = std::make_tuple(std::ref(args)...);
	return create_lock_i(indexes, addresses, std::make_index_sequence<sizeof...(args)>{});
}

int main(){
	std::mutex m1;
	std::mutex m2;
	std::mutex m3;
	auto lg2 = create_lock(m1, m2, m3);
}
