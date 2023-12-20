#include <gtest/gtest.h>
#include <sstream>
#include "../test_adv.cuh"
#include "../../src/app/matmul.cuh"

namespace matmul {

    using namespace troy;
    using namespace troy::linear;
    using tool::GeneralEncoder;
    using tool::GeneralVector;
    using tool::GeneralHeContext;
    using std::stringstream;
    using std::vector;

    uint64_t multiply_mod(uint64_t a, uint64_t b, uint64_t t) {
        __uint128_t c = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b);
        return static_cast<uint64_t>(c % static_cast<__uint128_t>(t));
    }

    uint64_t add_mod(uint64_t a, uint64_t b, uint64_t t) {
        if (a + b >= t) {
            return a + b - t;
        } else {
            return a + b;
        }
    }

    void add_mod_inplace(uint64_t& a, uint64_t b, uint64_t t) {
        a = add_mod(a, b, t);
    }

    void test_matmul(const GeneralHeContext& context, size_t m, size_t r, size_t n, bool pack_lwe, bool mod_switch_to_next) {
        SchemeType scheme = context.params_host().scheme();
        if (scheme != SchemeType::BFV && scheme != SchemeType::BGV) {
            throw std::runtime_error("[test_matmul] Unsupported scheme");
        }
        uint64_t t = context.t();
        
        GeneralVector x = context.random_polynomial(m * r);
        GeneralVector w = context.random_polynomial(r * n);
        GeneralVector s = context.random_polynomial(m * n);
        MatmulHelper helper(m, r, n, context.params_host().poly_modulus_degree(), MatmulObjective::EncryptLeft, pack_lwe);

        HeContextPointer he = context.context();
        const BatchEncoder& encoder = context.encoder().batch();
        const Encryptor& encryptor = context.encryptor();
        const Evaluator& evaluator = context.evaluator();
        const Decryptor& decryptor = context.decryptor();
        GaloisKeys automorphism_key;
        if (pack_lwe) {
            automorphism_key = context.key_generator().create_automorphism_keys(false);
        }
        
        Plain2d x_encoded = helper.encode_inputs(encoder, x.integers().data());
        Plain2d w_encoded = helper.encode_weights(encoder, w.integers().data());
        Plain2d s_encoded = helper.encode_outputs(encoder, s.integers().data());

        std::cerr << x_encoded.rows() << " " << x_encoded.columns() << std::endl;
        std::cerr << "x[0][0] = " << GeneralVector(encoder.decode_polynomial_new(x_encoded[0][0])) << std::endl;

        Cipher2d x_encrypted = x_encoded.encrypt_asymmetric(encryptor);

        std::cerr << x_encrypted.rows() << " " << x_encoded.columns() << std::endl;
        std::cerr << "x[0][0] = " << GeneralVector(encoder.decode_polynomial_new(decryptor.decrypt_new(x_encrypted[0][0]))) << std::endl;

        stringstream x_serialized;
        x_encrypted.save(x_serialized, he);
        x_encrypted = Cipher2d::load_new(x_serialized, he);

        std::cerr << x_encrypted.rows() << " " << x_encoded.columns() << std::endl;
        std::cerr << "x[0][0] = " << GeneralVector(encoder.decode_polynomial_new(decryptor.decrypt_new(x_encrypted[0][0]))) << std::endl;

        Cipher2d y_encrypted = helper.matmul(evaluator, x_encrypted, w_encoded);
        if (mod_switch_to_next) {
            y_encrypted.mod_switch_to_next_inplace(evaluator);
        }
        if (pack_lwe) {
            y_encrypted = helper.pack_outputs(evaluator, automorphism_key, y_encrypted);
        }

        // y_encrypted.add_plain_inplace(evaluator, s_encoded);

        stringstream y_serialized;
        helper.serialize_outputs(evaluator, y_encrypted, y_serialized);
        y_encrypted = helper.deserialize_outputs(evaluator, y_serialized);

        vector<uint64_t> y_decrypted = helper.decrypt_outputs(encoder, decryptor, y_encrypted);   

        vector<uint64_t> y_truth(m * n, 0);
        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                for (size_t k = 0; k < r; k++) {
                    add_mod_inplace(y_truth[i * n + j], 
                        multiply_mod(
                            x.integers()[i * r + k], 
                            w.integers()[k * n + j],
                        t), t);
                }
                // add_mod_inplace(y_truth[i * n + j], s.integers()[i * n + j], t);
            }
        }

        GeneralVector decrypted(std::move(y_decrypted));
        GeneralVector truthv(std::move(y_truth));

        std::cerr << "Truth:     " << truthv << std::endl;
        std::cerr << "Decrypted: " << decrypted << std::endl;
        
        ASSERT_TRUE(truthv.near_equal(decrypted, 0));
    }


    TEST(MatmulTest, HostMatmul) {
        GeneralHeContext ghe(false, SchemeType::BFV, 1024, 40, { 60, 40, 40, 60 }, true, 0x123, 0);
        srand(0);
        test_matmul(ghe, 4, 5, 6, false, false);
        test_matmul(ghe, 64, 128, 256, false, false);
        test_matmul(ghe, 4, 5, 6, true, false);
        test_matmul(ghe, 64, 128, 256, true, false);
    }

    TEST(MatmulTest, DeviceMatmul) {
        GeneralHeContext ghe(true, SchemeType::BFV, 1024, 40, { 60, 40, 40, 60 }, true, 0x123, 0);
        srand(0);
        test_matmul(ghe, 4, 5, 6, false, false);
        test_matmul(ghe, 64, 128, 256, false, false);
        test_matmul(ghe, 400, 500, 600, false, false);
        test_matmul(ghe, 4, 5, 6, true, false);
        test_matmul(ghe, 64, 128, 256, true, false);
        test_matmul(ghe, 400, 500, 600, true, false);
    }

}