#include <tuple>
#include <cassert>

// a more powerful switch statement

const struct otherwise_t{} otherwise;

template <class T>
struct dead{};

template <class T>
struct noop{};

const struct fallthrough_t{} fallthrough;

// prohibit some types, like string literals, as pointer comparison is normally not wanted
template <class T>
struct is_t{const T& obj;};

template <class T>
is_t<T> is(const T& obj){ return is_t<T>{obj};}

struct expensive{
	expensive(int i);
	expensive(const expensive&) = delete;
	expensive(expensive&&) = delete;
	int toint();
};

// FIXME: check if perfect forwarding works as intended
template <class E, class V, class O>
inline constexpr decltype(auto) when(const E& e, const is_t<V>& v, O&& o) noexcept  {
	if(e==v.obj){return std::forward<O>(o);}
	// compiler unable to see that this is might be dead
}
template <class E, class O>
inline constexpr decltype(auto) when(const E& e, otherwise_t, O&& o) noexcept  {
	return std::forward<O>(o);
}

//otherwise_t needs a return even if dead code
//no_otherwise_t could help with that, but return type is void... and adding second template does not help
template <class E, class O>
[[noreturn]] inline constexpr auto when(const E& e, dead<O>)  {
	throw 1;
}
// FIXME: silence warning
//template <class E, class O>
//[[noreturn]] constexpr auto when(const E& e, noop<O>) -> O {}

template <class E, class V, class O, class... Types>
inline constexpr decltype(auto) when(const E& e, const is_t<V>& v, fallthrough_t, const is_t<V>& v2, O&& o, Types&&... args) noexcept(noexcept((e == v.obj) ? o : when(e, args...))) {
	return (e == v.obj) ? std::forward<O>(o) : when(e, v2, std::forward<Types>(args)...);
}
// sfinae that O != fallthough
template <class E, class V, class O, class... Types>
inline constexpr decltype(auto) when(const E& e, const is_t<V>& v, O&& o, Types&&... args) noexcept(noexcept((e == v.obj) ? o : when(e, args...)))  {
	return (e == v.obj) ? std::forward<O>(o) : when(e, std::forward<Types>(args)...);
}

// for simplicity return int
template <class Lambda>
class lazy_eval
{
	Lambda lambda_;

	public:
	lazy_eval( Lambda lambda)
		: lambda_(std::move(lambda)) {}

	operator int() const {
		return lambda_();
	}
};

template <class Lambda>
lazy_eval<Lambda> lazy_eval_f(Lambda lambda){return lazy_eval<Lambda>(std::move(lambda));} 
#define LAZY(Expr) lazy_eval_f([&]() { return Expr; })

#define ONE 1

// like switch, but
// - compiler is unable to see if we need "exaustive" switch or not, and generate useful warnings 
//   * otherwise and dead<T> helps to fix it, but it's a runtime fix
// - (!)lazy is not the norm/less readable and unable to integrate in c++<14 (issue with conversions)
// - cannot check if "is" are overlapping (could check all is for equality, more difficult with range, but value might be computed at runtime)
// 
// * contrary to switch and catch, ordering is important, as there might be multiple matches (like in java catch)
// * depending on pattern, generates more or less code than if-else and ?:
// * does not create unnecessary copies
//
// + works with non-integral types
// + works with runtime values
// + can create custom types for more appropriate op==
// ++ when in_range
// ++ pattern matching
// + is an expression (has assignable return value)
// + otherwise/dead/noop/whatever cannot appear together
#if ONE
int main1(int i, int j){
	return when(i // notice: no lazy building!
			, is(i+1), expensive(41)
			, is(j+1), expensive(42)
			//, is(3), LAZY(42)
			//, is(1), fallthrough
			, is(2), expensive(43)
			, is(expensive(12).toint()), expensive(12)
			, otherwise, expensive(44)
			//, noop<int>()
			).toint();
}
#endif
#if TWO
// here expensive is build lazily!
int main2(int i, int j){
	if(i == i+1){
		return expensive(41).toint();
	}
	if (i == j+1){
		return expensive(42).toint();
	}
	if (i == 2){
		return expensive(43).toint();
	}
	if(i == expensive(12).toint()){
		return expensive(12).toint();
	}
	return expensive(44).toint();
}
#endif
#if THREE
int main3(int i, int j){
	return (
			(i == i+1) ? expensive(41) :
			(i == j+1) ? expensive(42) :
			(i == 2) ? expensive(43) :
			(i == expensive(12).toint()) ? expensive(12) :
			expensive(44)
		   ).toint();
}
#endif

#if FOUR
#define IS
#define OTHERWISE
#define DEAD throw 1;
int main3(int i, int j){
	return (
			(i == i+1) ? expensive(41) :
			(i == j+1) ? expensive(42) :
			(i == 2) ? expensive(43) :
			(i == expensive(12).toint()) ? expensive(12) :
			expensive(44)
		   ).toint();
}
#endif
