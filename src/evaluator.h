#pragma once
#include "batch_utils.h"
#include "encryption_parameters.h"
#include "he_context.h"
#include "plaintext.h"
#include "ciphertext.h"
#include "kswitch_keys.h"
#include "utils/memory_pool.h"
#include "utils/scaling_variant.h"
#include "lwe_ciphertext.h"
#include <string>

namespace troy {

    class Evaluator {

    public:
        enum SwitchKeyDestinationAssignMethod {
            Overwrite,
            AddInplace,
            OverwriteExceptFirst
        };

    private:
        HeContextPointer context_;

        ContextDataPointer get_context_data(const char* prompt, const ParmsID& encrypted) const;

        void translate_inplace(Ciphertext& encrypted1, const Ciphertext& encrypted2, bool subtract, MemoryPoolHandle pool) const;
        void translate_inplace_batched(
            const std::vector<Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, bool subtract, MemoryPoolHandle pool
        ) const;
        void translate(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, bool subtract, MemoryPoolHandle pool) const;
        void translate_batched(
            const std::vector<const Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, 
            const std::vector<Ciphertext*>& destination,
            bool subtract, MemoryPoolHandle pool
        ) const;
        
        void bfv_multiply(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool) const;
        void ckks_multiply(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool) const;
        void bgv_multiply(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool) const;

        void bfv_square(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool) const;
        void ckks_square(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool) const;
        void bgv_square(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool) const;

        /// Suppose kswitch_keys[kswitch_kes_index] is generated with s' on a KeyGenerator of secret key s.
        /// Then the semantic of this function is as follows: `target` is supposed to multiply with s' to contribute to the
        /// decrypted result, now we apply this function, to decompose (target * s') into (c0, c1) such that c0 + c1 * s = target * s.
        /// If assign_method is AddInplace, then we add c0, c1 to the original c0, c1 in the `destination`; if assign_method is Overwrite, we put c0 and c1 to the `destination`.
        void switch_key_internal(
            const Ciphertext& encrypted, utils::ConstSlice<uint64_t> target, 
            const KSwitchKeys& kswitch_keys, size_t kswitch_keys_index, SwitchKeyDestinationAssignMethod assign_method, Ciphertext& destination, MemoryPoolHandle pool
        ) const;

        void relinearize_inplace_internal(Ciphertext& encrypted, const RelinKeys& relin_keys, size_t destination_size, MemoryPoolHandle pool) const;
        void relinearize_internal(const Ciphertext& encrypted, const RelinKeys& relin_keys, size_t destination_size, Ciphertext& destination, MemoryPoolHandle pool) const;

        void mod_switch_scale_to_next_internal(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool) const;
        void mod_switch_drop_to_internal(const Ciphertext& encrypted, Ciphertext& destination, ParmsID target_parms_id, MemoryPoolHandle pool) const;
        void mod_switch_drop_to_plain_internal(const Plaintext& plain, Plaintext& destination, ParmsID target_parms_id, MemoryPoolHandle pool) const;

        void translate_plain_inplace(Ciphertext& encrypted, const Plaintext& plain, bool subtract, MemoryPoolHandle pool) const;
        void translate_plain(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, bool subtract, MemoryPoolHandle pool) const;

        void multiply_plain_normal(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, MemoryPoolHandle pool) const;
        void multiply_plain_normal_batched(
            const std::vector<const Ciphertext*>& encrypted, const std::vector<const Plaintext*>& plain, 
            const std::vector<Ciphertext*>& destination, MemoryPoolHandle pool
        ) const;
        void multiply_plain_ntt(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, MemoryPoolHandle pool) const;
        void multiply_plain_ntt_batched(
            const std::vector<const Ciphertext*>& encrypted, const std::vector<const Plaintext*>& plain, 
            const std::vector<Ciphertext*>& destination, MemoryPoolHandle pool
        ) const;
        void multiply_plain_ntt_inplace(Ciphertext& encrypted, const Plaintext& plain) const;

        void rotate_internal(const Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool) const;
        void conjugate_internal(const Ciphertext& encrypted, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool) const;

    public:
        inline Evaluator(HeContextPointer context): context_(context) {}
        inline HeContextPointer context() const { return context_; }
        inline bool on_device() const {return this->context()->on_device();}





        // ==================================
        //                negate
        // ==================================

        void negate_inplace(Ciphertext& encrypted) const;
        void negate(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext negate_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            negate(encrypted, destination, pool);
            return destination;
        }







        // ==================================
        //                add
        // ==================================

        inline void add_inplace(Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_inplace(encrypted1, encrypted2, false, pool);
        }
        inline void add_inplace_batched(
            const std::vector<Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            translate_inplace_batched(encrypted1, encrypted2, false, pool);
        }
        inline void add(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate(encrypted1, encrypted2, destination, false, pool);
        }
        inline void add_batched(
            const std::vector<const Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, 
            const std::vector<Ciphertext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            translate_batched(encrypted1, encrypted2, destination, false, pool);
        }
        
        inline Ciphertext add_new(const Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            add(encrypted1, encrypted2, destination, pool);
            return destination;
        }
        inline std::vector<Ciphertext> add_new_batched(
            const std::vector<const Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted1.size());
            std::vector<Ciphertext*> destination_ptr(encrypted1.size()); for (size_t i = 0; i < encrypted1.size(); i++) destination_ptr[i] = &destination[i];
            add_batched(encrypted1, encrypted2, destination_ptr, pool);
            return destination;
        }









        // ==================================
        //                subtract
        // ==================================

        inline void sub_inplace(Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_inplace(encrypted1, encrypted2, true, pool);
        }
        inline void sub_inplace_batched(
            std::vector<Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            translate_inplace_batched(encrypted1, encrypted2, true, pool);
        }
        inline void sub(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate(encrypted1, encrypted2, destination, true, pool);
        }
        inline void sub_batched(
            const std::vector<const Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, 
            std::vector<Ciphertext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            translate_batched(encrypted1, encrypted2, destination, true, pool);
        }
        inline Ciphertext sub_new(const Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            sub(encrypted1, encrypted2, destination, pool);
            return destination;
        }
        inline std::vector<Ciphertext> sub_new_batched(
            const std::vector<const Ciphertext*>& encrypted1, const std::vector<const Ciphertext*>& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted1.size());
            std::vector<Ciphertext*> destination_ptr(encrypted1.size()); for (size_t i = 0; i < encrypted1.size(); i++) destination_ptr[i] = &destination[i];
            sub_batched(encrypted1, encrypted2, destination_ptr, pool);
            return destination;
        }









        // ==================================
        //                multiply
        // ==================================

        void multiply(const Ciphertext& encrypted1, const Ciphertext& encrypted2, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void multiply_inplace(Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination; multiply(encrypted1, encrypted2, destination, pool);
            encrypted1 = std::move(destination);
        }
        inline Ciphertext multiply_new(const Ciphertext& encrypted1, const Ciphertext& encrypted2, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            multiply(encrypted1, encrypted2, destination, pool);
            return destination;
        }







        

        // ==================================
        //                square
        // ==================================

        void square(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void square_inplace(Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination; square(encrypted, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext square_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            square(encrypted, destination, pool);
            return destination;
        }





        // ==================================
        //             keyswitching
        // ==================================

        void apply_keyswitching_inplace(Ciphertext& encrypted, const KSwitchKeys& kswitch_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        void apply_keyswitching(const Ciphertext& encrypted, const KSwitchKeys& kswitch_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext apply_keyswitching_new(const Ciphertext& encrypted, const KSwitchKeys& kswitch_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            apply_keyswitching(encrypted, kswitch_keys, destination, pool);
            return destination;
        }






        // ==================================
        //             relin
        // ==================================

        inline void relinearize_inplace(Ciphertext& encrypted, const RelinKeys& relin_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            relinearize_inplace_internal(encrypted, relin_keys, 2, pool);
        }
        inline void relinearize(const Ciphertext& encrypted, const RelinKeys& relin_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            relinearize_internal(encrypted, relin_keys, 2, destination, pool);
        }
        inline Ciphertext relinearize_new(const Ciphertext& encrypted, const RelinKeys& relin_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            relinearize(encrypted, relin_keys, destination, pool);
            return destination;
        }





        // ==================================
        //             modswitch
        // ==================================

        void mod_switch_to_next(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void mod_switch_to_next_inplace(Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            mod_switch_to_next(encrypted, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext mod_switch_to_next_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            mod_switch_to_next(encrypted, destination, pool);
            return destination;
        }

        inline void mod_switch_plain_to_next(const Plaintext& plain, Plaintext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            auto context_data = this->get_context_data("[Evaluator::mod_switch_plain_to_next]", plain.parms_id());
            if (!context_data->next_context_data().has_value()) {
                throw std::invalid_argument("[Evaluator::mod_switch_plain_to_next] The input plaintext is already at the last modulus");
            }
            auto next_parms_id = context_data->next_context_data().value()->parms().parms_id();
            this->mod_switch_drop_to_plain_internal(plain, destination, next_parms_id, pool);
        }
        inline void mod_switch_plain_to_next_inplace(Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            mod_switch_plain_to_next(plain, destination, pool);
            plain = std::move(destination);
        }
        inline Plaintext mod_switch_plain_to_next_new(const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            mod_switch_plain_to_next(plain, destination, pool);
            return destination;
        }

        void mod_switch_to(const Ciphertext& encrypted, const ParmsID& parms_id, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void mod_switch_to_inplace(Ciphertext& encrypted, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            mod_switch_to(encrypted, parms_id, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext mod_switch_to_new(const Ciphertext& encrypted, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            mod_switch_to(encrypted, parms_id, destination, pool);
            return destination;
        }

        void mod_switch_plain_to(const Plaintext& plain, const ParmsID& parms_id, Plaintext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void mod_switch_plain_to_inplace(Plaintext& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            mod_switch_plain_to(plain, parms_id, destination, pool);
            plain = std::move(destination);
        }
        inline Plaintext mod_switch_plain_to_new(const Plaintext& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            mod_switch_plain_to(plain, parms_id, destination, pool);
            return destination;
        }






        // ==================================
        //             rescale
        // ==================================

        void rescale_to_next(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void rescale_to_next_inplace(Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rescale_to_next(encrypted, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext rescale_to_next_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rescale_to_next(encrypted, destination, pool);
            return destination;
        }

        void rescale_to(const Ciphertext& encrypted, const ParmsID& parms_id, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void rescale_to_inplace(Ciphertext& encrypted, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rescale_to(encrypted, parms_id, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext rescale_to_new(const Ciphertext& encrypted, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rescale_to(encrypted, parms_id, destination, pool);
            return destination;
        }






        // ==================================
        //             add plain
        // ==================================

        inline void add_plain_inplace(Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_plain_inplace(encrypted, plain, false, pool);
        }
        inline void add_plain(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_plain(encrypted, plain, destination, false, pool);
        }
        inline Ciphertext add_plain_new(const Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            add_plain(encrypted, plain, destination, pool);
            return destination;
        }





        // ==================================
        //             sub plain
        // ==================================

        inline void sub_plain_inplace(Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_plain_inplace(encrypted, plain, true, pool);
        }
        inline void sub_plain(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            translate_plain(encrypted, plain, destination, true, pool);
        }
        inline Ciphertext sub_plain_new(const Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            sub_plain(encrypted, plain, destination, pool);
            return destination;
        }






        // ==================================
        //             multiply plain
        // ==================================

        void multiply_plain(const Ciphertext& encrypted, const Plaintext& plain, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void multiply_plain_inplace(Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext c; multiply_plain(encrypted, plain, c, pool);
            encrypted = std::move(c);
        }
        inline Ciphertext multiply_plain_new(const Ciphertext& encrypted, const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            multiply_plain(encrypted, plain, destination, pool);
            return destination;
        }

        void multiply_plain_batched(
            const std::vector<const Ciphertext*>& encrypted, 
            const std::vector<const Plaintext*>& plain, 
            const std::vector<Ciphertext*>& destination, 
            MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        inline void multiply_plain_inplace_batched(
            const std::vector<Ciphertext*>& encrypted, 
            const std::vector<const Plaintext*>& plain, 
            MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted.size());
            auto destination_ptrs = batch_utils::collect_pointer(destination);
            auto encrypted_const_ptrs = batch_utils::pcollect_const_pointer(encrypted);
            multiply_plain_batched(encrypted_const_ptrs, plain, destination_ptrs, pool);
            for (size_t i = 0; i < encrypted.size(); i++) *encrypted[i] = std::move(destination[i]);
        }
        inline std::vector<Ciphertext> multiply_plain_new_batched(
            const std::vector<const Ciphertext*>& encrypted, 
            const std::vector<const Plaintext*>& plain, 
            MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted.size());
            auto destination_ptrs = batch_utils::collect_pointer(destination);
            multiply_plain_batched(encrypted, plain, destination_ptrs, pool);
            return destination;
        }









        // ==================================
        //       transform plain ntt
        // ==================================

        void transform_plain_to_ntt_inplace(Plaintext& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        void transform_plain_to_ntt(const Plaintext& plain, const ParmsID& parms_id, Plaintext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Plaintext transform_plain_to_ntt_new(const Plaintext& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            transform_plain_to_ntt(plain, parms_id, destination, pool);
            return destination;
        }
        void transform_plain_to_ntt_inplace_batched(
            const std::vector<Plaintext*>& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        void transform_plain_to_ntt_batched(
            const std::vector<const Plaintext*>& plain, const ParmsID& parms_id, 
            const std::vector<Plaintext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        inline std::vector<Plaintext> transform_plain_to_ntt_new_batched(
            const std::vector<const Plaintext*>& plain, const ParmsID& parms_id, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Plaintext> destination(plain.size()); auto destination_ptrs = batch_utils::collect_pointer(destination);
            transform_plain_to_ntt_batched(plain, parms_id, destination_ptrs, pool);
            return destination;
        }

        void transform_plain_from_ntt_inplace(Plaintext& plain) const;
        void transform_plain_from_ntt(const Plaintext& plain, Plaintext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Plaintext transform_plain_from_ntt_new(const Plaintext& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            transform_plain_from_ntt(plain, destination, pool);
            return destination;
        }
        void transform_plain_from_ntt_inplace_batched(
            const std::vector<Plaintext*>& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        void transform_plain_from_ntt_batched(
            const std::vector<const Plaintext*>& plain, const std::vector<Plaintext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        inline std::vector<Plaintext> transform_plain_from_ntt_new_batched(
            const std::vector<const Plaintext*>& plain, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Plaintext> destination(plain.size()); auto destination_ptrs = batch_utils::collect_pointer(destination);
            transform_plain_from_ntt_batched(plain, destination_ptrs, pool);
            return destination;
        }







        // ==================================
        //       transform cipher ntt
        // ==================================

        void transform_to_ntt_inplace(Ciphertext& encrypted) const;
        void transform_to_ntt(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext transform_to_ntt_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            transform_to_ntt(encrypted, destination, pool);
            return destination;
        }
        void transform_to_ntt_inplace_batched(
            const std::vector<Ciphertext*>& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        void transform_to_ntt_batched(
            const std::vector<const Ciphertext*>& encrypted, const std::vector<Ciphertext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        inline std::vector<Ciphertext> transform_to_ntt_new_batched(
            const std::vector<const Ciphertext*>& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted.size()); auto destination_ptrs = batch_utils::collect_pointer(destination);
            transform_to_ntt_batched(encrypted, destination_ptrs, pool);
            return destination;
        }

        void transform_from_ntt_inplace(Ciphertext& encrypted) const;
        void transform_from_ntt(const Ciphertext& encrypted, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext transform_from_ntt_new(const Ciphertext& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            transform_from_ntt(encrypted, destination, pool);
            return destination;
        }
        void transform_from_ntt_inplace_batched(
            const std::vector<Ciphertext*>& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        void transform_from_ntt_batched(
            const std::vector<const Ciphertext*>& encrypted, const std::vector<Ciphertext*>& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const;
        inline std::vector<Ciphertext> transform_from_ntt_new_batched(
            const std::vector<const Ciphertext*>& encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()
        ) const {
            std::vector<Ciphertext> destination(encrypted.size()); auto destination_ptrs = batch_utils::collect_pointer(destination);
            transform_from_ntt_batched(encrypted, destination_ptrs, pool);
            return destination;
        }





        // ==================================
        //       apply galois
        // ==================================

        void apply_galois(const Ciphertext& encrypted, size_t galois_element, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void apply_galois_inplace(Ciphertext& encrypted, size_t galois_element, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            apply_galois(encrypted, galois_element, galois_keys, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext apply_galois_new(const Ciphertext& encrypted, size_t galois_element, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            apply_galois(encrypted, galois_element, galois_keys, destination, pool);
            return destination;
        }

        void apply_galois_plain(const Plaintext& plain, size_t galois_element, Plaintext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline void apply_galois_plain_inplace(Plaintext& plain, size_t galois_element, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            apply_galois_plain(plain, galois_element, destination, pool);
            plain = std::move(destination);
        }
        inline Plaintext apply_galois_plain_new(const Plaintext& plain, size_t galois_element, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Plaintext destination;
            apply_galois_plain(plain, galois_element, destination, pool);
            return destination;
        }








        // ==================================
        //         rotate
        // ==================================

        void rotate_rows(const Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            SchemeType scheme = this->context()->key_context_data().value()->parms().scheme();
            if (scheme == SchemeType::BFV || scheme == SchemeType::BGV) {
                rotate_internal(encrypted, steps, galois_keys, destination, pool);
            } else {
                throw std::invalid_argument("[Evaluator::rotate_rows_inplace] Rotate rows only applies for BFV or BGV");
            }
        }
        inline void rotate_rows_inplace(Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_rows(encrypted, steps, galois_keys, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext rotate_rows_new(const Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_rows(encrypted, steps, galois_keys, destination, pool);
            return destination;
        }

        inline void rotate_columns(const Ciphertext& encrypted, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            SchemeType scheme = this->context()->key_context_data().value()->parms().scheme();
            if (scheme == SchemeType::BFV || scheme == SchemeType::BGV) {
                conjugate_internal(encrypted, galois_keys, destination, pool);
            } else {
                throw std::invalid_argument("[Evaluator::rotate_columns_inplace] Rotate columns only applies for BFV or BGV");
            }
        }
        inline void rotate_columns_inplace(Ciphertext& encrypted, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_columns(encrypted, galois_keys, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext rotate_columns_new(const Ciphertext& encrypted, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_columns(encrypted, galois_keys, destination, pool);
            return destination;
        }

        inline void rotate_vector(const Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            SchemeType scheme = this->context()->key_context_data().value()->parms().scheme();
            if (scheme == SchemeType::CKKS) {
                rotate_internal(encrypted, steps, galois_keys, destination, pool);
            } else {
                throw std::invalid_argument("[Evaluator::rotate_vector_inplace] Rotate vector only applies for CKKS");
            }
        }
        inline void rotate_vector_inplace(Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_vector(encrypted, steps, galois_keys, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext rotate_vector_new(const Ciphertext& encrypted, int steps, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            rotate_vector(encrypted, steps, galois_keys, destination, pool);
            return destination;
        }

        inline void complex_conjugate(const Ciphertext& encrypted, const GaloisKeys& galois_keys, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            SchemeType scheme = this->context()->key_context_data().value()->parms().scheme();
            if (scheme == SchemeType::CKKS) {
                conjugate_internal(encrypted, galois_keys, destination, pool);
            } else {
                throw std::invalid_argument("[Evaluator::complex_conjugate_inplace] Complex conjugate only applies for CKKS");
            }
        }
        inline void complex_conjugate_inplace(Ciphertext& encrypted, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            complex_conjugate(encrypted, galois_keys, destination, pool);
            encrypted = std::move(destination);
        }
        inline Ciphertext complex_conjugate_new(const Ciphertext& encrypted, const GaloisKeys& galois_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            complex_conjugate(encrypted, galois_keys, destination, pool);
            return destination;
        }









        // Pack LWE utilities

        LWECiphertext extract_lwe_new(const Ciphertext& encrypted, size_t term, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext assemble_lwe_new(const LWECiphertext& lwe_encrypted, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            return lwe_encrypted.assemble_lwe(pool);
        }
        
        void field_trace_inplace(Ciphertext& encrypted, const GaloisKeys& automorphism_keys, size_t logn, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        
        void divide_by_poly_modulus_degree_inplace(Ciphertext& encrypted, uint64_t mul = 1) const;
        
        Ciphertext pack_lwe_ciphertexts_new(const std::vector<LWECiphertext>& lwe_encrypted, const GaloisKeys& automorphism_keys, MemoryPoolHandle pool = MemoryPool::GlobalPool(), bool apply_field_trace = true) const;

        void negacyclic_shift(const Ciphertext& encrypted, size_t shift, Ciphertext& destination, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const;
        inline Ciphertext negacyclic_shift_new(const Ciphertext& encrypted, size_t shift, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            negacyclic_shift(encrypted, shift, destination, pool);
            return destination;
        }
        inline void negacyclic_shift_inplace(Ciphertext& encrypted, size_t shift, MemoryPoolHandle pool = MemoryPool::GlobalPool()) const {
            Ciphertext destination;
            negacyclic_shift(encrypted, shift, destination, pool);
            encrypted = std::move(destination);
        }
    };

}