#ifndef META_H_
#define META_H_


#include <type_traits>


namespace Fastor {

//----------------------------------------------------------------------------------------------------------//
template< bool B, class T = void >
using enable_if_t_ = typename std::enable_if<B,T>::type;

template< bool B, class T = void >
using disable_if_t_ = typename std::enable_if<!B,T>::type;

template< bool B, class T, class F >
using conditional_t_ = typename std::conditional<B,T,F>::type;

template< class T, class U >
constexpr bool is_same_v_ = std::is_same<T, U>::value;

template< class T >
constexpr bool is_arithmetic_v_ = std::is_arithmetic<T>::value;
//----------------------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------------------//
template<typename T> struct remove_all { using type = T; };
template<typename T> struct remove_all<const T>   { using type = typename remove_all<T>::type; };
template<typename T> struct remove_all<T const&>  { using type = typename remove_all<T>::type; };
template<typename T> struct remove_all<T&>        { using type = typename remove_all<T>::type; };
template<typename T> struct remove_all<T const*>  { using type = typename remove_all<T>::type; };
template<typename T> struct remove_all<T*>        { using type = typename remove_all<T>::type; };

template<typename T>
using remove_all_t = typename remove_all<T>::type;
//----------------------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------------------//
template<size_t...Rest> struct add;
template<size_t Head, size_t ...Rest>
struct add<Head, Rest...> {
    static const size_t value = Head+add<Rest...>::value;
};
template<>
struct add<> {
    static const size_t value = 0;
};

template<size_t...Rest> struct prod;
template<size_t Head, size_t ...Rest>
struct prod<Head, Rest...> {
    static const size_t value = Head*prod<Rest...>::value;
};
template<>
struct prod<> {
    static const size_t value = 1;
};

template<size_t Idx, size_t ... Rest>
struct prod_nel;
template<size_t Idx, size_t First, size_t ... Rest>
struct prod_nel<Idx, First, Rest...> {
    static const size_t value = prod_nel<Idx-1, Rest...>::value;
};
template<size_t First, size_t ... Rest>
struct prod_nel<1, First, Rest...> {
    static const size_t value = prod<Rest...>::value;
};

//! Partial product of a sequence up to an index up_to
template<class T, T N>
constexpr
inline T partial_prod(const T (&ind)[N], T up_to, T num=0) {
    return num == up_to ? ind[num] : ind[num]*partial_prod(ind, up_to, num+1);
}

//! Partial product of a sequence up to an index num from the end
template<class T, T N>
constexpr
inline T partial_prod_reverse(const T (&ind)[N], T up_to, T num=N-1) {
    return num == up_to ? ind[num] : ind[num]*partial_prod_reverse(ind, up_to, num-1);
}

template<size_t Idx, size_t ... Rest>
struct get_all {
    static const size_t indices[sizeof...(Rest)];
};


template<typename T>
constexpr T size_proder_(T one){
    return one.size();
}
template<typename T>
constexpr T size_proder_(T one, T two){
    return one.size()*two.size();
}
template<typename T, typename ... Ts>
constexpr T size_proder_(T one, T two, Ts ... ts) {
    return _proder_(_proder_(one,two),ts...);
}
//----------------------------------------------------------------------------------------------------------//


// comparisons
//----------------------------------------------------------------------------------------------------------//
template<size_t I, size_t J>
struct is_less {
    static constexpr bool value = (I < J);
};
template<size_t I, size_t J>
static constexpr bool is_less_v_ = is_less<I,J>::value;

template<size_t I, size_t J>
struct is_less_equal {
    static constexpr bool value = I <= J;
};
template<size_t I, size_t J>
static constexpr bool is_less_equal_v_ = is_less_equal<I,J>::value;

template<size_t I, size_t J>
struct is_greater {
    static constexpr bool value = I > J;
};
template<size_t I, size_t J>
static constexpr bool is_greater_v_ = is_greater<I,J>::value;

template<size_t I, size_t J>
struct is_greater_equal {
    static constexpr bool value = I >= J;
};
template<size_t I, size_t J>
static constexpr bool is_greater_equal_v_ = is_greater_equal<I,J>::value;
//----------------------------------------------------------------------------------------------------------//


// min-max
//----------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------//
template<size_t ... rest>
struct meta_min;
template<size_t m, size_t n, size_t ... rest>
struct meta_min<m,n,rest...> {
    static constexpr size_t pval = meta_min<m,n>::value;
    static const size_t value = (pval <= meta_min<pval,rest...>::value) ?
                pval : meta_min<pval,rest...>::value;
};
template <size_t m, size_t n>
struct meta_min<m,n> {
    static const size_t value = (m<=n) ? m : n;
};


template<size_t ... rest>
struct meta_max;
template<size_t m, size_t n, size_t ... rest>
struct meta_max<m,n,rest...> {
    static constexpr size_t pval = meta_max<m,n>::value;
    static const size_t value = (pval >= meta_max<pval,rest...>::value) ?
                pval : meta_max<pval,rest...>::value;
};
template<size_t m, size_t n>
struct meta_max<m,n> {
    static const size_t value = (m>=n) ? m : n;
};
//----------------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------------//
template<size_t ... rest>
struct meta_argmin;
template<size_t m, size_t n, size_t ... rest>
struct meta_argmin<m,n,rest...> {
    static constexpr size_t pval = meta_min<m,n>::value;
    static const size_t value = (pval <= meta_min<pval,rest...>::value) ?
                meta_argmin<m,n>::value : meta_argmin<pval,rest...>::value+1;
};
template<size_t m, size_t n>
struct meta_argmin<m,n> {
    static const size_t value = (m<n) ? 0 : 1;
};


template<size_t ... rest>
struct meta_argmax;
template<size_t m, size_t n, size_t ... rest>
struct meta_argmax<m,n,rest...> {
    static constexpr size_t pval = meta_max<m,n>::value;
    static const size_t value = (pval >= meta_max<pval,rest...>::value) ?
                meta_argmax<m,n>::value : meta_argmax<pval,rest...>::value+1;
};
template<size_t m, size_t n>
struct meta_argmax<m,n> {
    static const size_t value = (m>n) ? 0 : 1;
};
//----------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------//


// square/cube
//----------------------------------------------------------------------------------------------------------//
namespace internal {
template<size_t Val>
struct meta_square {
    static constexpr size_t value = Val*Val;
};

template<size_t Val>
struct meta_cube {
    static constexpr size_t value = Val*Val*Val;
};
}
//----------------------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------------------//
//square root of integers at compile time, use like meta_sqrt<36>::ret
template<int Y,
         int InfX = 0,
         int SupX = ((Y==1) ? 1 : Y/2),
         bool Done = ((SupX-InfX)<=1 ? true : ((SupX*SupX <= Y) && ((SupX+1)*(SupX+1) > Y))) >
                                // use ?: instead of || just to shut up a gcc 4.3 warning
class meta_sqrt
{
    enum {
  MidX = (InfX+SupX)/2,
  TakeInf = MidX*MidX > Y ? 1 : 0,
  NewInf = int(TakeInf) ? InfX : int(MidX),
  NewSup = int(TakeInf) ? int(MidX) : SupX
};
  public:
    enum { ret = meta_sqrt<Y,NewInf,NewSup>::ret };
};

template<int Y, int InfX, int SupX>
class meta_sqrt<Y, InfX, SupX, true>
{
    public:  enum { ret = (SupX*SupX <= Y) ? SupX : InfX };
};
//----------------------------------------------------------------------------------------------------------//


// Checks if all parameters in a variadic parameter pack are arithmetic
//----------------------------------------------------------------------------------------------------------//
template<class ... T>
struct is_arithmetic_pack;

template<class T, class ... U>
struct is_arithmetic_pack<T,U...> {
    static constexpr bool value = std::is_arithmetic<T>::value && is_arithmetic_pack<U...>::value;
};

template<class T>
struct is_arithmetic_pack<T> {
    static constexpr bool value = std::is_arithmetic<T>::value;
};
//----------------------------------------------------------------------------------------------------------//



} // end of namespace Fastor

#endif // META_H_