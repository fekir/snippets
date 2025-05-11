#define FEK_EMPTY()
#define FEK_DEFER( id ) id FEK_EMPTY()
#define FEK_OBSTRUCT( ... ) __VA_ARGS__ FEK_DEFER( FEK_EMPTY )()
#define FEK_EXPAND( ... ) __VA_ARGS__

// use for parameters with embedded ","
#define FEK_PARAM( ... ) FEK_OBSTRUCT( FEK_EXPAND )( __VA_ARGS__ )
// use for return values with embedded ","
#define FEK_RET( ... ) __VA_ARGS__
// optional, use for readability
#define FEK_NOCV()

// helper for overloading a macro
#define FEK_GET_MACRO( _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ... ) NAME

// for querying function signature
#define V0_OR_NOTHING( ... ) __VA_OPT__( V0 )

#define FEK_TYPE_VALUE0( T0 )                                     T0 V0_OR_NOTHING(T0)
#define FEK_TYPE_VALUE1( T0, T1 )                                 T0 V0, T1 V1
#define FEK_TYPE_VALUE2( T0, T1, T2 )                             T0 V0, T1 V1, T2 V2
#define FEK_TYPE_VALUE3( T0, T1, T2, T3 )                         T0 V0, T1 V1, T2 V2, T3 V3
#define FEK_TYPE_VALUE4( T0, T1, T2, T3, T4 )                     T0 V0, T1 V1, T2 V2, T3 V3, T4 V4
#define FEK_TYPE_VALUE5( T0, T1, T2, T3, T4, T5 )                 T0 V0, T1 V1, T2 V2, T3 V3, T4 V4, T5 V5
#define FEK_TYPE_VALUE6( T0, T1, T2, T3, T4, T5, T6 )             T0 V0, T1 V1, T2 V2, T3 V3, T4 V4, T5 V5, T6 V6
#define FEK_TYPE_VALUE7( T0, T1, T2, T3, T4, T5, T6, T7 )         T0 V0, T1 V1, T2 V2, T3 V3, T4 V4, T5 V5, T6 V6, T7 V7
#define FEK_TYPE_VALUE8( T0, T1, T2, T3, T4, T5, T6, T7, T8 )     T0 V0, T1 V1, T2 V2, T3 V3, T4 V4, T5 V5, T6 V6, T7 V7, T8 V8
#define FEK_TYPE_VALUE9( T0, T1, T2, T3, T4, T5, T6, T7, T8, T9 ) T0 V0, T1 V1, T2 V2, T3 V3, T4 V4, T5 V5, T6 V6, T7 V7, T8 V8, T9 V9

#define FEK_VALUE0( T0 )                                     V0_OR_NOTHING(T0)
#define FEK_VALUE1( T0, T1 )                                 V0, V1
#define FEK_VALUE2( T0, T1, T2 )                             V0, V1, V2
#define FEK_VALUE3( T0, T1, T2, T3 )                         V0, V1, V2, V3
#define FEK_VALUE4( T0, T1, T2, T3, T4 )                     V0, V1, V2, V3, V4
#define FEK_VALUE5( T0, T1, T2, T3, T4, T5 )                 V0, V1, V2, V3, V4, V5
#define FEK_VALUE6( T0, T1, T2, T3, T4, T5, T6 )             V0, V1, V2, V3, V4, V5, V6
#define FEK_VALUE7( T0, T1, T2, T3, T4, T5, T6, T7 )         V0, V1, V2, V3, V4, V5, V6, V7
#define FEK_VALUE8( T0, T1, T2, T3, T4, T5, T6, T7, T8 )     V0, V1, V2, V3, V4, V5, V6, V7, V8
#define FEK_VALUE9( T0, T1, T2, T3, T4, T5, T6, T7, T8, T9 ) V0, V1, V2, V3, V4, V5, V6, V7, V8, V9

#define FEK_GET_TYPE_VALUES( ... ) \
  FEK_GET_MACRO( __VA_ARGS__, \
  FEK_TYPE_VALUE9, FEK_TYPE_VALUE8, FEK_TYPE_VALUE7, FEK_TYPE_VALUE6, FEK_TYPE_VALUE5, FEK_TYPE_VALUE4, FEK_TYPE_VALUE3, FEK_TYPE_VALUE2, FEK_TYPE_VALUE1, FEK_TYPE_VALUE0 ) \
  ( __VA_ARGS__ )
#define FEK_GET_VALUES( ... ) \
  FEK_GET_MACRO( __VA_ARGS__, \
  FEK_VALUE9, FEK_VALUE8, FEK_VALUE7, FEK_VALUE6, FEK_VALUE5, FEK_VALUE4, FEK_VALUE3, FEK_VALUE2, FEK_VALUE1, FEK_VALUE0 ) \
   __VA_ARGS__ )
#define FEK_GET_TYPES( ... ) __VA_ARGS__






// Example usage




// create a list of function signatures
#define MEMBER_FUNCTIONS( X ) \
 /* return type                   ,name ,qualifiers ,optional parameters                 */ \
  X( FEK_RET(void)                ,foo  ,FEK_NOCV() ,                                     ) \
  X( FEK_RET(int)                 ,bar  ,const       ,FEK_PARAM(std::pair<int, int>), int ) \
  X( FEK_RET(std::pair<int, int>) ,baz  ,noexcept    ,const char*                         ) \
  static_assert(true)


struct dummy {
#define MFUNCTION( RET, NAME, QUAL, ... ) RET NAME( FEK_GET_TYPES( __VA_ARGS__ ) ) QUAL;
  MEMBER_FUNCTIONS( MFUNCTION );
#undef MFUNCTION
};


template<class... T>
void sink(const T&...) {}

#define MFUNCTION( RET, NAME, QUAL, ... ) \
  RET dummy::NAME( FEK_GET_TYPE_VALUES( __VA_ARGS__ ) ) QUAL { \
    sink(FEK_GET_VALUES( __VA_ARGS__ )); return RET(); \
  }

  MEMBER_FUNCTIONS( MFUNCTION );
#undef MFUNCTION


