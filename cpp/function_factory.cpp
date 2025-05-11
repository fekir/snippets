// function factory
// together with destructure function


template <auto V>
struct nontype_t{
  explicit nontype_t() = default;
  using type = decltype( V );
};
template <auto V>
inline constexpr nontype_t<V> nontype{};

template <typename P, auto fptr>
consteval auto make_fptr(P, nontype_t<fptr>){
  static_assert(std::is_pointer_v<P>);
  using void_ptr = std::conditional_t<std::is_const_v<std::remove_pointer_t<P>>,
    const void*,
    void*
  >;
  using fptr_traits = function_traits<typename nontype_t<fptr>::type>;
  if constexpr(std::is_member_function_pointer_v<typename nontype_t<fptr>::type>){
    auto l = [] <class R, class... Args>(R(*)(Args...)){
      return +[]( void_ptr ptr, Args... xs ) noexcept(fptr_traits::is_noexcept) -> R {
        auto obj = reinterpret_cast<P>( ptr );
        return ( obj->*fptr )(std::forward<Args>( xs )... );
      };
    };
    return l(typename fptr_traits::ptr());
 }else{
    auto l = [] <class R, class... Args>(R(*)(P, Args...)){
      return +[]( void_ptr ptr, Args... xs ) noexcept(fptr_traits::is_noexcept) -> R {
        auto obj = reinterpret_cast<P>( ptr );
        return ( *fptr )(obj, std::forward<Args>( xs )... );
      };
    };
    return l(typename fptr_traits::ptr());
 }
}



template <typename F, typename P, auto fptr>
constexpr void init_fptr(F*& f, P, nontype_t<fptr>) noexcept {
  static_assert(std::is_pointer_v<P>);
  using void_ptr = std::conditional_t<std::is_const_v<std::remove_pointer_t<P>>,
    const void*,
    void*
  >;
  using fptr_traits = function_traits<typename nontype_t<fptr>::type>;
  using fptr_target_traits = function_traits<F>;
  auto l = [] <class R, class... Args>(R(*)(void_ptr, Args...)){
    return +[]( void_ptr ptr, Args... xs ) noexcept(fptr_traits::is_noexcept) -> R {
      auto obj = reinterpret_cast<P>( ptr );
      if constexpr(std::is_void_v<R>){
        (void) std::invoke(fptr, obj, std::forward<Args>( xs )... );
      } else {
        return std::invoke(fptr, obj, std::forward<Args>( xs )... );
      }
    };
  };
  f = l(typename fptr_target_traits::ptr());
}


struct square {
  double x;
  double y;
  double area() const;
  double perimeter() const;
  int edges() const;
};

struct circle {
  double radius;
  double area() const;
  double perimeter() const;
  int edges() const;
};

class shape_view {
  const void* data;
  double(*area_fun)(const void*);
  double(*perimeter_fun)(const void*);
  int(*edges_fun)(const void*);

 public:
  template <typename T>
  shape_view(const T& t) :
    data(&t),
    area_fun(     make_fptr(&t, nontype_t<&T::area>())),
    perimeter_fun(make_fptr(&t, nontype_t<&T::perimeter>())),
    edges_fun(    make_fptr(&t, nontype_t<&T::edges>()))
 {
   init_fptr(area_fun, &t, nontype_t<&T::area>());
 }

  double area() const { return this->area_fun(data); }
  double perimeter() const { return this->perimeter_fun(data); }
  int edges() const { return this->edges_fun(data); }
};

void foo(square& s){
    auto sv = shape_view(s);
    sv.area();
    sv.perimeter();
    sv.edges();
}
