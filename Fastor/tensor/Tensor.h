#ifndef TENSOR_H
#define TENSOR_H

#include "Fastor/commons/commons.h"
#include "Fastor/backend/backend.h"
#include "Fastor/simd_vector/SIMDVector.h"
#include "Fastor/tensor/AbstractTensor.h"
#include "Fastor/tensor/Ranges.h"
#include "Fastor/tensor/ForwardDeclare.h"
#include "Fastor/expressions/linalg_ops/linalg_ops.h"
#include "Fastor/meta/tensor_pre_meta.h"

namespace Fastor {

template<typename T, size_t ... Rest>
class Tensor: public AbstractTensor<Tensor<T,Rest...>,sizeof...(Rest)> {
private:
#ifdef FASTOR_ZERO_INITIALISE
    T FASTOR_ALIGN _data[prod<Rest...>::value] = {};
#else
    T FASTOR_ALIGN _data[prod<Rest...>::value];
#endif
public:
    using scalar_type = T;
    using result_type = Tensor<T,Rest...>;
    using Dimension_t = std::integral_constant<FASTOR_INDEX, sizeof...(Rest)>;
    static constexpr FASTOR_INDEX Dimension = sizeof...(Rest);
    static constexpr FASTOR_INDEX Size = prod<Rest...>::value;
    static constexpr FASTOR_INDEX Stride = stride_finder<T>::value;
    static constexpr FASTOR_INDEX Remainder = prod<Rest...>::value % sizeof(T);
    static constexpr FASTOR_INLINE FASTOR_INDEX rank() {return Dimension;}
    static constexpr FASTOR_INLINE FASTOR_INDEX size() {return Size;}
    FASTOR_INLINE FASTOR_INDEX dimension(FASTOR_INDEX dim) const {
#ifndef NDEBUG
        FASTOR_ASSERT(dim>=0 && dim < sizeof...(Rest), "TENSOR SHAPE MISMATCH");
#endif
        const FASTOR_INDEX DimensionHolder[sizeof...(Rest)] = {Rest...};
        return DimensionHolder[dim];
    }
    FASTOR_INLINE Tensor<T,Rest...>& noalias() {
        return *this;
    }


    // Classic constructors
    //----------------------------------------------------------------------------------------------------------//
    constexpr FASTOR_INLINE Tensor(){}

    template<typename U=T, typename std::enable_if<std::is_arithmetic<U>::value,bool>::type=0>
    FASTOR_INLINE Tensor(U num) {
        SIMDVector<T,DEFAULT_ABI> reg(static_cast<T>(num));
        FASTOR_INDEX i;
        for (i = 0; i< ROUND_DOWN(Size,Stride); i+=Stride) {
            reg.store(&_data[i]);
        }
        for (; i<Size; ++i) {
            _data[i] = (T)num;
        }
    }

    FASTOR_INLINE Tensor(const Tensor<T,Rest...> &other) {
        // This constructor cannot be default
        // Note that all other data members are static constexpr
        if (_data == other.data()) return;

        std::copy(other.data(),other.data()+Size,_data);
        // using V = SIMDVector<T,DEFAULT_ABI>;
        // const T* other_data = other.data();
        // FASTOR_INDEX i=0;
        // for (; i<ROUND_DOWN(Size,V::Size); i+=V::Size) {
        //     V(&other_data[i]).store(&_data[i]);
        // }
        // for (; i<Size; ++i) {
        //     _data[i] = other_data[i];
        // }
    };

    // Initialiser list constructors
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/InitializerListConstructors.h"
    //----------------------------------------------------------------------------------------------------------//

    // Classic array wrappers
    //----------------------------------------------------------------------------------------------------------//
    FASTOR_INLINE Tensor(const T *arr, int layout=RowMajor) {
        std::copy(arr,arr+Size,_data);
        if (layout == RowMajor)
            return;
        else
            *this = tocolumnmajor(*this);
    }
    FASTOR_INLINE Tensor(const std::array<T,sizeof...(Rest)> &arr, int layout=RowMajor) {
        std::copy(arr,arr+prod<Rest...>::value,_data);
        if (layout == RowMajor)
            return;
        else
            *this = tocolumnmajor(*this);
    }
    //----------------------------------------------------------------------------------------------------------//

    // CRTP constructors
    //----------------------------------------------------------------------------------------------------------//
    //----------------------------------------------------------------------------------------------------------//
    // Generic AbstractTensors
#ifndef FASTOR_DISABLE_SPECIALISED_CTR
    template<typename Derived, size_t DIMS,
        typename std::enable_if<(!internal::has_tensor_view<Derived>::value &&
        !internal::has_tensor_fixed_view_2d<Derived>::value) || DIMS!=sizeof...(Rest),bool>::type=0>
#else
    template<typename Derived, size_t DIMS>
#endif
    FASTOR_INLINE Tensor(const AbstractTensor<Derived,DIMS>& src) {
        FASTOR_ASSERT(src.self().size()==size(), "TENSOR SIZE MISMATCH");
        assign(*this, src.self());
    }
    //----------------------------------------------------------------------------------------------------------//

    // In-place operators
    //----------------------------------------------------------------------------------------------------------//
    FASTOR_INLINE void operator +=(const Tensor<T,Rest...> &a) {
        trivial_assign_add(*this, a);
    }

    FASTOR_INLINE void operator -=(const Tensor<T,Rest...> &a) {
        trivial_assign_sub(*this, a);
    }

    FASTOR_INLINE void operator *=(const Tensor<T,Rest...> &a) {
        trivial_assign_mul(*this, a);
    }

    FASTOR_INLINE void operator /=(const Tensor<T,Rest...> &a) {
        trivial_assign_div(*this, a);
    }

    // Specialised constructors
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/SpecialisedConstructors.h"
    //----------------------------------------------------------------------------------------------------------//

    // AbstractTensor and scalar in-place operators
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/TensorInplaceOperators.h"
    //----------------------------------------------------------------------------------------------------------//

    // Raw pointer providers
    //----------------------------------------------------------------------------------------------------------//
#ifdef FASTOR_ZERO_INITIALISE
    constexpr FASTOR_INLINE T* data() const { return const_cast<T*>(this->_data);}
#else
    FASTOR_INLINE T* data() const { return const_cast<T*>(this->_data);}
#endif

    FASTOR_INLINE T* data() {return this->_data;}
    //----------------------------------------------------------------------------------------------------------//

    // Scalar & block indexing
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/IndexRetriever.h"
    #include "Fastor/tensor/ScalarIndexing.h"
    #include "Fastor/tensor/BlockIndexing.h"
    //----------------------------------------------------------------------------------------------------------//

    // Expression templates evaluators
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/TensorEvaluator.h"
    //----------------------------------------------------------------------------------------------------------//

    //----------------------------------------------------------------------------------------------------------//
    // #include "Fastor/tensor/SmartExpressionsPlugin.h"
    //----------------------------------------------------------------------------------------------------------//

    // Tensor methods
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/TensorMethods.h"
    //----------------------------------------------------------------------------------------------------------//

    // Converters
    //----------------------------------------------------------------------------------------------------------//
    #include "Fastor/tensor/PODConverters.h"
    //----------------------------------------------------------------------------------------------------------//

    // Cast method
    //----------------------------------------------------------------------------------------------------------//
    template<typename U>
    FASTOR_INLINE Tensor<U,Rest...> cast() const {
        Tensor<U,Rest...> out;
        U *out_data = out.data();
        for (FASTOR_INDEX i=0; i<Size; ++i) {
            out_data[get_mem_index(i)] = static_cast<U>(_data[i]);
        }
        return out;
    }
    //----------------------------------------------------------------------------------------------------------//

    // Boolean functions
    //----------------------------------------------------------------------------------------------------------//
    constexpr FASTOR_INLINE bool is_uniform() const {
        //! A tensor is uniform if it spans equally in all dimensions,
        //! i.e. generalisation of square matrix to n dimension
        return no_of_unique<Rest...>::value==1 ? true : false;
    }

    template<typename U, size_t ... RestOther>
    FASTOR_INLINE bool is_equal(const Tensor<U,RestOther...> &other, const double Tol=PRECI_TOL) const {
        //! Two tensors are equal if they have the same type, rank, size and elements
        if(!std::is_same<T,U>::value) return false;
        if(sizeof...(Rest)!=sizeof...(RestOther)) return false;
        if(prod<Rest...>::value!=prod<RestOther...>::value) return false;
        else {
            bool out = true;
            const T *other_data = other.data();
            for (size_t i=0; i<Size; ++i) {
                if (std::fabs(_data[i]-other_data[i])>Tol) {
                    out = false;
                    break;
                }
            }
            return out;
        }
    }

    FASTOR_INLINE bool is_orthogonal() const {
        //! A second order tensor A is orthogonal if A*A'= I
        if (!is_uniform())
            return false;
        else {
            static_assert(sizeof...(Rest)==2,"ORTHOGONALITY OF MATRIX WITH RANK!=2 CANNOT BE DETERMINED");
            Tensor<T,Rest...> out;
            out = matmul(transpose(*this),*this);
            Tensor<T,Rest...> ey; ey.eye();
            return is_equal(ey);
        }
    }

    FASTOR_INLINE bool does_belong_to_so3(const double Tol=PRECI_TOL) const {
        //! A second order tensor belongs to special orthogonal 3D group if
        //! it is orthogonal and its determinant is +1
        if (is_orthogonal()) {
            // Check if we are in 3D space
            if (prod<Rest...>::value!=9) {
                return false;
            }
            T out = _det<T,Rest...>(_data);
            if (std::fabs(out-1)>Tol) {
                return false;
            }
            return true;
        }
        else {
            return false;
        }
    }

    FASTOR_INLINE bool does_belong_to_sl3(const double Tol=PRECI_TOL) const {
        //! A second order tensor belongs to special linear 3D group if
        //! its determinant is +1
        T out = _det<T,Rest...>(_data);
        if (std::fabs(out-1.)>Tol) {
            return false;
        }
        return true;
    }

    FASTOR_INLINE bool is_symmetric(const double Tol=PRECI_TOL) const {
        if (is_uniform()) {
            bool bb = true;
            size_t M = dimension(0);
            size_t N = dimension(1);
            for (size_t i=0; i<M; ++i)
                for (size_t j=0; j<N; ++j)
                    if (std::fabs(_data[i*N+j] - _data[j*N+i])>Tol) {
                        bb = false;
                    }
            return bb;
        }
        else {
            return false;
        }
    }
    template<typename ... Args, typename std::enable_if<sizeof...(Args)==2,bool>::type=0>
    FASTOR_INLINE bool is_symmetric(Args ...) const {
        return true;
    }

    FASTOR_INLINE bool is_deviatoric(const double Tol=PRECI_TOL) const {
        if (std::fabs(trace(*this))<Tol)
            return true;
        else
            return false;
    }
    //----------------------------------------------------------------------------------------------------------//

protected:
    template<typename Derived, size_t DIMS>
    FASTOR_INLINE void verify_dimensions(const AbstractTensor<Derived,DIMS>& src_) const {
        static_assert(DIMS==Dimension, "TENSOR RANK MISMATCH");
#ifndef NDEBUG
        const Derived &src = src_.self();
        FASTOR_ASSERT(src.size()==this->size(), "TENSOR SIZE MISMATCH");
        // Check if shape of tensors match
        for (FASTOR_INDEX i=0; i<Dimension; ++i) {
            FASTOR_ASSERT(src.dimension(i)==dimension(i), "TENSOR SHAPE MISMATCH");
        }
#endif
    }
    //----------------------------------------------------------------------------------------------------------//

};


} // end of namespace Fastor


#include "Fastor/tensor/TensorAssignment.h"


#endif // TENSOR_H

