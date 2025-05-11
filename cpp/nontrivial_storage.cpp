#include <new>
#include <utility>

template <bool noexcept_copy, bool noexcept_move, bool noexcept_swap>
struct operation_pointers {
  using copy_fun_t         = void(void*, const void*) noexcept(noexcept_copy);
  using assign_copy_fun_t  = void(void*, const void*) noexcept(noexcept_copy);
  using move_fun_t         = void(void*, void*)       noexcept(noexcept_move);
  using assign_move_fun_t  = void(void*, void*)       noexcept(noexcept_move);
  using swap_fun_t         = void(void*, void*)       noexcept(noexcept_swap);
  using dest_fun_t         = void(void*)              noexcept;
  copy_fun_t* copy = nullptr;
  assign_copy_fun_t* assign_copy = nullptr;
  move_fun_t* move = nullptr;
  assign_move_fun_t* assign_move = nullptr;
  swap_fun_t* swap_f = nullptr;
  dest_fun_t* destroy = nullptr;
};

template<class T, bool noexcept_copy, bool noexcept_move, bool noexcept_swap>
constexpr auto create_operation_pointers() noexcept {
  static_assert(not std::is_array_v<T>);
  static_assert(std::is_destructible_v<T>);
  // Do not change to const to prevent it to get merged with other objects
  static constinit auto mret = []{
    auto ret = operation_pointers<noexcept_copy,noexcept_move, noexcept_swap>{
      .destroy = [](void* dest) noexcept { std::launder(reinterpret_cast<T*>(dest))->~T(); }
    };
    if constexpr(std::is_copy_constructible_v<T>){
      ret.copy = [](void* dest,const void* source) noexcept(noexcept_copy) {
        ::new (dest)T(*std::launder(reinterpret_cast<const T*>(source)));
      };
      ret.assign_copy = [](void* dest,const void* source) noexcept(noexcept_copy) {
        *std::launder(reinterpret_cast<T*>(dest)) = *std::launder(reinterpret_cast<const T*>(source));
      };
    }
    if constexpr(std::is_move_constructible_v<T>){
      ret.move = [](void* dest, void* source) noexcept(noexcept_move) {
        ::new (dest) T(std::move(*std::launder(reinterpret_cast<T*>(source))));
      };
      ret.assign_move = [](void* dest, void* source) noexcept(noexcept_move) {
        *std::launder(reinterpret_cast<T*>(dest)) = std::move(*std::launder(reinterpret_cast<T*>(source)));
      };
    }
    if constexpr(std::is_swappable_v<T>){
      ret.swap_f = [](void* dest, void* source) noexcept(noexcept_swap) {
        using std::swap; swap(*std::launder(reinterpret_cast<T*>(dest)), *std::launder(reinterpret_cast<T*>(source)));
      };
    }
    return ret;
  }();
  return &mret;
}

struct nontrivial_storage_cfg {
  int size;
  bool noexcept_move = true;
  bool noexcept_copy = false;
  bool noexcept_swap = noexcept_move;
  bool is_copy_constructible = true;
  bool is_move_constructible = true;
  bool is_swappable = is_move_constructible;
};

template <nontrivial_storage_cfg cfg>
class nontrivial_storage {
  alignas(cfg.size) unsigned char buffer[cfg.size] = {};
  operation_pointers<cfg.noexcept_move, cfg.noexcept_copy, cfg.noexcept_swap>* f = nullptr;

  template <typename T, class... Args>
  explicit nontrivial_storage(T*, Args&&... args) : f(create_operation_pointers<T,cfg.noexcept_move,cfg.noexcept_copy,cfg.noexcept_swap>())
  {
    static_assert(cfg.size>0, "buffer is not big enough");
    static_assert(sizeof(T) <= cfg.size, "buffer is not big enough");
    static_assert(cfg.size % alignof(T) == 0, "wrong alignment");

    static_assert(not cfg.noexcept_move or std::is_nothrow_move_constructible_v<T>);
    static_assert(not cfg.noexcept_copy or std::is_nothrow_copy_constructible_v<T>);
    static_assert(not cfg.noexcept_swap or (std::is_nothrow_swappable_v<T> and std::is_nothrow_move_constructible_v<T>));
    static_assert(not cfg.is_copy_constructible or std::is_copy_constructible_v<T>);
    static_assert(not cfg.is_move_constructible or std::is_move_constructible_v<T>);
    static_assert(not cfg.is_swappable or std::is_swappable_v<T>);
    static_assert(std::is_destructible_v<T>);

    static_assert(not std::is_array_v<T>);
    ::new (buffer)T(std::forward<Args>(args)...);
  }

 public:

  template<class T, class... Args>
  static nontrivial_storage make(Args&&... args) {
    return nontrivial_storage(static_cast<T*>(nullptr), std::forward<Args>(args)...);
  }

  nontrivial_storage(const nontrivial_storage& other) noexcept(cfg.noexcept_copy) requires(cfg.is_copy_constructible) : f(other.f) {
    this->f.copy(this->buffer, other.buffer);
  }
  nontrivial_storage(nontrivial_storage&& other) noexcept(cfg.noexcept_move) requires(cfg.is_move_constructible) : f(other.f) {
    this->f->move(this->buffer, other.buffer);
  }
  nontrivial_storage& operator=(const nontrivial_storage& other) noexcept(cfg.noexcept_copy) requires(cfg.is_copy_constructible) {
    if (&other != this) {
      if (other.f == this->f) {
          this->f.assign_copy(this->buffer, other.buffer);
      } else {
            this->f.destroy(this->buffer);
            other.f.copy(this->buffer, other.buffer);
      }
      this->f = other.f;
    }
    return *this;
  }
  nontrivial_storage& operator=(nontrivial_storage&& other) noexcept(cfg.noexcept_move) requires(cfg.is_move_constructible) {
    if (&other != this) {
      if (other.f == this->f) {
        this->f->assign_move(this->buffer, other.buffer);
      } else {
        this->f->destroy(this->buffer);
        other.f->move(this->buffer, other.buffer);
      }
      this->f = other.f;
    }
    return *this;
  }
  ~nontrivial_storage() { this->f->destroy(this->buffer); }

  friend void swap(nontrivial_storage& lhs, nontrivial_storage& rhs) noexcept(cfg.noexcept_swap) requires(cfg.is_swappable)
  {
    if(&lhs != & rhs){
      if (lhs.f == rhs.f) {
        lhs.f->swap_f(lhs.buffer, rhs.buffer);
      } else {
        auto tmp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(tmp);
      }
    }
  }

  template <typename T>
  T* data() noexcept {
    return std::launder(reinterpret_cast<T*>(this->buffer));
  }
  template <typename T>
  const T* data() const noexcept {
    return std::launder(reinterpret_cast<const T*>(this->buffer));
  }
};

struct square {
  double x;
  double y;
  void draw() const;
  square(int,int);
};

struct circle {
  double radius;
  void draw() const;
  circle(int i);
};

class shape {
  // should be big enough for all shapes we want to type-erase
  using storage = nontrivial_storage<nontrivial_storage_cfg{.size = 4 * alignof(float)}>;
  storage data;
  using draw_fun = void(const storage&);
  draw_fun* ptr;

 public:
  template <typename T>
  shape(T t) :
    data(storage::make<T>(std::move(t))),
    ptr([](const storage& data) { data.data<T>()->draw(); }) {}

  void draw() const { ptr(data); }

  friend void swap(shape& lhs, shape& rhs) noexcept {
    swap(lhs.data, rhs.data);
    std::swap(lhs.ptr, rhs.ptr);
  }
};

int main() {
  auto s = shape(circle(0));
  s.draw();
  s = shape(circle(1));
  s.draw();
  s = shape(square(2,3));
  s.draw();
  auto s2 = std::move(s);
  s2.draw();
}
