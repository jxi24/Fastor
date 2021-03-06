#ifndef TENSOR_FIXED_VIEWS_1D_H
#define TENSOR_FIXED_VIEWS_1D_H


#include "Fastor/tensor/Tensor.h"
#include "Fastor/tensor/Ranges.h"

namespace Fastor {




// 1D const fixed views
//----------------------------------------------------------------------------------------------//
template<typename T, size_t N, int F0, int L0, int S0>
struct TensorConstFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> :
    public AbstractTensor<TensorConstFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>,1> {
private:
    const Tensor<T,N> &expr;
public:
    using scalar_type = T;
    using result_type = Tensor<T,range_detector<F0,L0,S0>::value>;
    static constexpr FASTOR_INDEX Dimension = 1;
    static constexpr FASTOR_INDEX Stride = stride_finder<T>::value;
    static constexpr FASTOR_INLINE FASTOR_INDEX size() {
        return range_detector<F0,L0,S0>::value;
    }
    static constexpr FASTOR_INLINE FASTOR_INDEX dimension(FASTOR_INDEX i) {
        return range_detector<F0,L0,S0>::value;
    }

    constexpr FASTOR_INLINE TensorConstFixedViewExpr1D(const Tensor<T,N> &_ex) : expr(_ex) {}

    template<typename U>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> eval(FASTOR_INDEX i) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*i+F0,S0);
        return _vec;
    }

    template<typename U>
    constexpr FASTOR_INLINE U eval_s(FASTOR_INDEX i) const {
        return expr.data()[S0*i+F0];
    }

    template<typename U>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> eval(FASTOR_INDEX i, FASTOR_INDEX j) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*(i+j)+F0,S0);
        return _vec;
    }

    template<typename U>
    constexpr FASTOR_INLINE U eval_s(FASTOR_INDEX i, FASTOR_INDEX j) const {
        return expr.data()[S0*(i+j)+F0];
    }

    template<typename U=T>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> teval(const std::array<int,1>& as) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*as[0]+F0,S0);
        return _vec;
    }

    template<typename U=T>
    constexpr FASTOR_INLINE U teval_s(const std::array<int,1>& as) const {
        return expr.data()[S0*as[0]+F0];
    }
};





// 1D non-const fixed views
//----------------------------------------------------------------------------------------------//
template<typename T, size_t N, int F0, int L0, int S0>
struct TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> :
    public AbstractTensor<TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>,1> {
private:
    Tensor<T,N> &expr;
    bool does_alias = false;
    constexpr FASTOR_INLINE Tensor<T,N> get_tensor() const {return expr;}
public:
    using scalar_type = T;
    using result_type = Tensor<T,range_detector<F0,L0,S0>::value>;
    static constexpr FASTOR_INDEX Dimension = 1;
    static constexpr FASTOR_INDEX Stride = stride_finder<T>::value;
    static constexpr FASTOR_INLINE FASTOR_INDEX size() {
        return range_detector<F0,L0,S0>::value;
    }
    static constexpr FASTOR_INLINE FASTOR_INDEX dimension(FASTOR_INDEX i) {
        return range_detector<F0,L0,S0>::value;
    }

    FASTOR_INLINE TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>& noalias() {
        // FASTOR_ASSERT(false,"FIXED 1D VIEWS DO NOT SUPPORT OVERLAPPING ASSIGNMENTS");
        does_alias = true;
        return *this;
    }

    constexpr FASTOR_INLINE TensorFixedViewExpr1D(Tensor<T,N> &_ex) : expr(_ex) {}


    //----------------------------------------------------------------------------------//
    void operator=(const TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> &other_src) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other_src;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(other_src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] = _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] = other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] = other_src.template eval_s<T>(i);
        }
#endif
    }

    void operator+=(const TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> &other_src) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other_src;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(other_src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] += _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] += other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] += other_src.template eval_s<T>(i);
        }
#endif
    }

    void operator-=(const TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> &other_src) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other_src;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(other_src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] -= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] -= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] -= other_src.template eval_s<T>(i);
        }
#endif
    }

    void operator*=(const TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> &other_src) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other_src;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(other_src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] *= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] *= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] *= other_src.template eval_s<T>(i);
        }
#endif
    }

    void operator/=(const TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1> &other_src) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other_src;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(other_src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] /= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] /= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] /= other_src.template eval_s<T>(i);
        }
#endif
    }

    //----------------------------------------------------------------------------------//
    template<typename Derived, size_t DIMS>
    void operator=(const AbstractTensor<Derived,DIMS> &other) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
        const Derived& other_src = other.self();
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] = _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] = other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] = other_src.template eval_s<T>(i);
        }
#endif
    }

    template<typename Derived, size_t DIMS>
    void operator+=(const AbstractTensor<Derived,DIMS> &other) {
        const Derived& other_src = other.self();
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] += _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] += other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] += other_src.template eval_s<T>(i);
        }
#endif
    }

    template<typename Derived, size_t DIMS>
    void operator-=(const AbstractTensor<Derived,DIMS> &other) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
        const Derived& other_src = other.self();
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] -= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] -= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] -= other_src.template eval_s<T>(i);
        }
#endif
    }

    template<typename Derived, size_t DIMS>
    void operator*=(const AbstractTensor<Derived,DIMS> &other) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
        const Derived& other_src = other.self();
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] *= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] *= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] *= other_src.template eval_s<T>(i);
        }
#endif
    }

    template<typename Derived, size_t DIMS>
    void operator/=(const AbstractTensor<Derived,DIMS> &other) {
#ifndef FASTOR_NO_ALIAS
        if (does_alias) {
            does_alias = false;
            // Evaluate this into a temporary
            auto tmp_this_tensor = get_tensor();
            auto tmp = TensorFixedViewExpr1D<Tensor<T,N>,fseq<F0,L0,S0>,1>(tmp_this_tensor);
            // Assign other to temporary
            tmp = other;
            // assign temporary to this
            this->operator=(tmp);
            return;
        }
#endif
        const Derived& other_src = other.self();
#ifndef NDEBUG
        FASTOR_ASSERT(other_src.size()==this->size(), "TENSOR SIZE MISMATCH");
#endif
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            auto _vec_other = other_src.template eval<T>(i);
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] /= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] /= other_src.template eval_s<T>(i);
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] /= other_src.template eval_s<T>(i);
        }
#endif
    }
    //----------------------------------------------------------------------------------//


    // Scalar binders
    //----------------------------------------------------------------------------------//
    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    void operator=(U num) {
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        auto _vec_other = SIMDVector<T,DEFAULT_ABI>(num);
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] = _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] = num;
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] = num;
        }
#endif
    }

    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    void operator+=(U num) {
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        auto _vec_other = SIMDVector<T,DEFAULT_ABI>(num);
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] += _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] += num;
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] += num;
        }
#endif
    }

    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    void operator-=(U num) {
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        auto _vec_other = SIMDVector<T,DEFAULT_ABI>(num);
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] -= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] -= num;
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] -= num;
        }
#endif
    }

    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    void operator*=(U num) {
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        auto _vec_other = SIMDVector<T,DEFAULT_ABI>(num);
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] *= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] *= num;
        }
#else
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] *= num;
        }
#endif
    }

    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    void operator/=(U num) {
        T *_data = expr.data();
#ifdef FASTOR_USE_VECTORISED_EXPR_ASSIGN
        FASTOR_INDEX i;
        T inum = T(1)/num;
        auto _vec_other = SIMDVector<T,DEFAULT_ABI>(inum);
        for (i = 0; i <ROUND_DOWN(size(),Stride); i+=Stride) {
            for (auto j=0; j<SIMDVector<T,DEFAULT_ABI>::Size; ++j) {
                _data[S0*(i+j) + F0] *= _vec_other[j];
            }
        }
        for (; i <size(); i++) {
            _data[S0*i+F0] *= inum;
        }
#else
        T inum = T(1)/num;
        for (FASTOR_INDEX i = 0; i <size(); i++) {
            _data[S0*i+F0] *= inum;
        }
#endif
    }
    //----------------------------------------------------------------------------------//

    template<typename U>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> eval(FASTOR_INDEX i) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*i+F0,S0);
        return _vec;
    }

    template<typename U>
    constexpr FASTOR_INLINE U eval_s(FASTOR_INDEX i) const {
        return expr.data()[S0*i+F0];
    }

    template<typename U>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> eval(FASTOR_INDEX i, FASTOR_INDEX j) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*(i+j)+F0,S0);
        return _vec;
    }

    template<typename U>
    constexpr FASTOR_INLINE U eval_s(FASTOR_INDEX i, FASTOR_INDEX j) const {
        return expr.data()[S0*(i+j)+F0];
    }

    template<typename U=T>
    FASTOR_INLINE SIMDVector<U,DEFAULT_ABI> teval(const std::array<int,1>& as) const {
        SIMDVector<U,DEFAULT_ABI> _vec;
        vector_setter(_vec,expr.data(),S0*as[0]+F0,S0);
        return _vec;
    }

    template<typename U=T>
    constexpr FASTOR_INLINE U teval_s(const std::array<int,1>& as) const {
        return expr.data()[S0*as[0]+F0];
    }
};


}


#endif // TENSOR_FIXED_VIEWS_1D_H
