#pragma once
#include "he_context.cuh"
#include "utils/dynamic_array.cuh"
#include "ciphertext.cuh"

namespace troy {

    class LWECiphertext {

        size_t coeff_modulus_size_;
        size_t poly_modulus_degree_;
        utils::DynamicArray<uint64_t> c0_; // c0.len == coeff_modulus_size
        utils::DynamicArray<uint64_t> c1_; // c1.len == coeff_modulus_size * poly_modulus_degree
        ParmsID parms_id_;
        double scale_;
        uint64_t correction_factor_;

    public:

        inline LWECiphertext() : 
            coeff_modulus_size_(0), poly_modulus_degree_(0),
            c0_(), c1_(),
            parms_id_(parms_id_zero), 
            scale_(1.0), correction_factor_(1) {}

        inline bool on_device() const {
            if (c0_.on_device() != c1_.on_device()) {
                throw std::runtime_error("[LWECiphertext::on_device] c0 and c1 are not on the same device");
            }
            return c0_.on_device();
        }

        inline LWECiphertext clone() const {
            LWECiphertext res;
            res.coeff_modulus_size_ = coeff_modulus_size_;
            res.poly_modulus_degree_ = poly_modulus_degree_;
            res.c0_ = c0_.clone();
            res.c1_ = c1_.clone();
            res.parms_id_ = parms_id_;
            res.scale_ = scale_;
            res.correction_factor_ = correction_factor_;
            return res;
        }

        inline void to_device_inplace() {
            c0_.to_device_inplace();
            c1_.to_device_inplace();
        }

        inline LWECiphertext to_device() const {
            LWECiphertext res = this->clone();
            res.to_device_inplace();
            return res;
        }

        inline void to_host_inplace() {
            c0_.to_host_inplace();
            c1_.to_host_inplace();
        }

        inline LWECiphertext to_host() const {
            LWECiphertext res = this->clone();
            res.to_host_inplace();
            return res;
        }

        inline const size_t& coeff_modulus_size() const { return coeff_modulus_size_; }
        inline size_t& coeff_modulus_size() { return coeff_modulus_size_; }

        inline const size_t& poly_modulus_degree() const { return poly_modulus_degree_; }
        inline size_t& poly_modulus_degree() { return poly_modulus_degree_; }

        inline const utils::DynamicArray<uint64_t>& c0_dyn() const { return c0_; }
        inline utils::DynamicArray<uint64_t>& c0_dyn() { return c0_; }

        inline const utils::DynamicArray<uint64_t>& c1_dyn() const { return c1_; }
        inline utils::DynamicArray<uint64_t>& c1_dyn() { return c1_; }

        inline utils::ConstSlice<uint64_t> c0() const { return c0_.const_reference(); }
        inline utils::Slice<uint64_t> c0() { return c0_.reference(); }
        inline utils::ConstSlice<uint64_t> const_c0() const { return c0_.const_reference(); }

        inline utils::ConstSlice<uint64_t> c1() const { return c1_.const_reference(); }
        inline utils::Slice<uint64_t> c1() { return c1_.reference(); }
        inline utils::ConstSlice<uint64_t> const_c1() const { return c1_.const_reference(); }

        inline const ParmsID& parms_id() const { return parms_id_; }
        inline ParmsID& parms_id() { return parms_id_; }

        inline const double& scale() const { return scale_; }
        inline double& scale() { return scale_; }

        inline const uint64_t& correction_factor() const { return correction_factor_; }
        inline uint64_t& correction_factor() { return correction_factor_; }

        Ciphertext assemble_lwe() const;

    };

}