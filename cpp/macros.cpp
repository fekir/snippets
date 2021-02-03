// WIDEN: 
// takes string literal or macro string literal
// and output the widened variant of it
// does not work with u8, works with R"(....)"
#define PP_WIDEN_NX( x ) L##x
#define PP_WIDEN( x ) PP_WIDEN_NX( x )

// AFAIK impossible to define UNWIDEN macro

namespace widen{
	constexpr wchar_t a[] = PP_WIDEN(R"(HELLO)");
	constexpr auto file = PP_WIDEN( __FILE__ );
	#define mystring R"(mystring!)"
	constexpr auto c = PP_WIDEN(mystring);
	static_assert(a[0] == 'H', "");
	static_assert(c[0] == 'm', "");
}

// CONCAT


// Concatenate preprocessor tokens A and B without expanding macro definitions
#define PP_CAT_NX(A, B) A ## B

// Concatenate preprocessor tokens A and B after macro-expanding them.
#define PP_CAT(A, B) PP_CAT_NX(A, B)
// macros

namespace concat {
	constexpr int a = PP_CAT(1,2);
	#define ONE 1
	static_assert(12 == PP_CAT(1,2), "");
	static_assert(PP_CAT(ONE,2) == PP_CAT(1,2), "");
	static_assert(PP_CAT_NX(1,2) == PP_CAT(1,2), "");
}


namespace str {
	// Turn A into a string literal without expanding macro definitions
	#define PP_STRINGIZE_NX(A) #A

	// Turn A into a string literal after macro-expanding it.
	#define PP_STRINGIZE(A) PP_STRINGIZE_NX(A)

	// same as PP_STRINGIZE, but produces error if A is not defined
	// not necessary for 
	//  * PP_WIDEN, as it produces a macro error
	//  * PP_CAT, as it produces a compile error
	#define PP_STRINGIZE_EXPR(A) ((A),PP_STRINGIZE_NX(A))

	#define T1 s
	#define T2 1
	constexpr auto a = PP_STRINGIZE(PP_CAT(T1, T2));
	static_assert(a[0] == 's', "");
	static_assert(a[1] == '1', "");

	constexpr auto b = PP_STRINGIZE(12);
	static_assert(b[0] == '1', "");
	static_assert(b[1] == '2', "");

	constexpr auto c = PP_STRINGIZE_NX("1");
	static_assert(c[0] == '"');
	static_assert(c[1] == '1', "");


	constexpr auto e = PP_STRINGIZE(ENOENT);
	static_assert(e[0] == 'E'); // forgot to define enoent...

	#define ENOENT 12
	constexpr auto e2 = PP_STRINGIZE_EXPR(ENOENT);
	static_assert(e2[0] == '1');
}
