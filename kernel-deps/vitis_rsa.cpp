#include "vitis_rsa.hpp"
#include <iostream>

namespace xf {
	namespace security {
		namespace internal {

			/*
			 * @brief Montgomery Production of opA and opB and returns opA * opB * R^-1 mod opM
			 * Reference: "Efficient architectures for implementing montgomery modular multiplication and RSA modular exponentiation
			 * on reconfigurable logic" by Alan Daly, William Marnane
			 *
			 * @tparam N bit width of opA, opB and opM
			 *
			 * @param opA Montgomery representation of A
			 * @param opB Montgomery representation of B
			 * @param opM modulus
			 */
			template <int N>
				ap_uint<N> monProduct(ap_uint<N> opA, ap_uint<N> opB, ap_uint<N> opM) {
					ap_uint<N + 2> s = 0;
					ap_uint<1> a0 = opA[0];
MON_PRODUCT_MOD:
					for (int i = 0; i < N; i++) {
						ap_uint<1> qa = opB[i];
						ap_uint<1> qm = s[0] ^ (opB[i] & a0);
						ap_uint<N> addA = qa == ap_uint<1>(1) ? opA : ap_uint<N>(0);
						ap_uint<N> addM = qm == ap_uint<1>(1) ? opM : ap_uint<N>(0);
						s += (addA + addM);
						s >>= 1;
					}
					if (s > opM) {
						s -= opM;
					}
					return s;
				}

			/*
			 * @brief Modular Exponentiation
			 *
			 * @tparam L bit width of base and modulus
			 * @tparam N bit width of exponent
			 *
			 * @param base Base of Modular Exponentiation
			 * @param exponent Exponent of Modular Exponentiation
			 * @param modulus Modulus of Modular Exponentiation
			 * @param rMod 2^(2*L) mod modulus could be pre-calculated.
			 */
			template <int L, int N>
				ap_uint<L> modularExp(ap_uint<L> base, ap_uint<N> exponent, ap_uint<L> modulus, ap_uint<L> rMod) {
					std::cout << __PRETTY_FUNCTION__ << "\n";
					ap_uint<L> P = monProduct<L>(rMod, base, modulus);
					ap_uint<L> R = monProduct<L>(rMod, 1, modulus);
					for (int i = N - 1; i >= 0; i--) {
						R = monProduct<L>(R, R, modulus);
						if (exponent[i] == 1) {
							R = monProduct<L>(R, P, modulus);
						}
					}
					return monProduct<L>(R, 1, modulus);
				}

			/*
			 * @brief modulo operation, returns remainder of dividend against divisor.
			 *
			 * @tparam L bit width of dividend
			 * @tparam N bit width of divisor
			 *
			 * @param dividend Dividend
			 * @param divisor Divisor
			 */
			template <int L, int N>
				ap_uint<N> simpleMod(ap_uint<L> dividend, ap_uint<N> divisor) {
					ap_uint<N> remainder = dividend % divisor;
					return remainder;
				}

			/**
			 * @brief return (opA * opB) mod opM
			 *
			 * @tparam N bit width of opA, opB and opM
			 *
			 * @param opA Product input, should be less than opM
			 * @param opB Product input, should be less than opM
			 * @param opM Modulus, should be larger than 2^(N-1)
			 */
			template <int N>
				ap_uint<N> productMod(ap_uint<N> opA, ap_uint<N> opB, ap_uint<N> opM) {
#pragma HLS inline off
					ap_uint<N + 1> tmp = 0;
PRODUCT_MOD:
					for (int i = N - 1; i >= 0; i--) {
						/* DIMITRA: #pragma HLS PIPELINE II=2 */
						tmp <<= 1;
						if (tmp >= opM) {
							tmp -= opM;
						}
						if (opB[i] == 1) {
							tmp += opA;
							if (tmp >= opM) {
								tmp -= opM;
							}
						}
					}
					return tmp;
				}

			/**
			 * @brief return (opA + opB) mod opM
			 *
			 * @tparam N bit width of opA, opB and opM
			 *
			 * @param opA Product input, should be less than opM
			 * @param opB Product input, should be less than opM
			 * @param opM Modulus
			 */
			template <int N>
				ap_uint<N> addMod(ap_uint<N> opA, ap_uint<N> opB, ap_uint<N> opM) {
					ap_uint<N + 1> sum = opA + opB;
					if (sum >= opM) {
						sum -= opM;
					}
					return sum;
				}

			/**
			 * @brief return (opA - opB) mod opM
			 *
			 * @tparam N bit width of opA, opB and opM
			 *
			 * @param opA Product input, should be less than opM
			 * @param opB Product input, should be less than opM
			 * @param opM Modulus
			 */
			template <int N>
				ap_uint<N> subMod(ap_uint<N> opA, ap_uint<N> opB, ap_uint<N> opM) {
					ap_uint<N + 1> sum;
					if (opA >= opB) {
						sum = opA - opB;
					} else {
						sum = opA + opM;
						sum -= opB;
					}
					return sum;
				}

			/**
			 * @brief return montgomery inverse of opA
			 * Reference: "The Montgomery Modular Inverse - Revisited" by E Savas, CK Koç
			 *
			 * @tparam N bit width of opA and opM
			 *
			 * @param opA Input of modular inverse.
			 * @param opM Modulus of modular inverse.
			 */
			template <int N>
				ap_uint<N> monInv(ap_uint<N> opA, ap_uint<N> opM) {
					/* calc r = opA^-1 * 2^k and k */
					ap_uint<N> u = opM;
					ap_uint<N> v = opA;
					ap_uint<N> s = 1;
					ap_uint<N + 1> r = 0;
					ap_uint<32> k = 0;

					while (v > 0) {
						if (u[0] == 0) {
							u >>= 1;
							s <<= 1;
						} else if (v[0] == 0) {
							v >>= 1;
							r <<= 1;
						} else if (u > v) {
							u -= v;
							u >>= 1;
							r += s;
							s <<= 1;
						} else {
							v -= u;
							v >>= 1;
							s += r;
							r <<= 1;
						}
						k++;
					}

					if (r >= opM) {
						r -= opM;
					}
					r = opM - r;

					k -= N;

					for (int i = 0; i < k; i++) {
						if (r[0] == 1) {
							r += opM;
						}
						r >>= 1;
					}

					return r;
				}

			/**
			 * @brief return modular inverse of opA
			 * Reference: "The Montgomery Modular Inverse - Revisited" by E Savas, CK Koç
			 *
			 * @tparam N bit width of opA and opM, opM should no less than 2^(N-1)
			 *
			 * @param opA Input of modular inverse. opA should be non-zero, might need extra checking
			 * @param opM Modulus of modular inverse.
			 */
			template <int N>
				ap_uint<N> modularInv(ap_uint<N> opA, ap_uint<N> opM) {
					/* calc r = opA^-1 * 2^k and k */
					ap_uint<N> u = opM;
					ap_uint<N> v = opA;
					ap_uint<N> s = 1;
					ap_uint<N + 1> r = 0;
					ap_uint<32> k = 0;

INV_MOD_I:
					while (v > 0) {
#pragma HLS loop_tripcount max = 256
						if (u[0] == 0) {
							u >>= 1;
							s <<= 1;
						} else if (v[0] == 0) {
							v >>= 1;
							r <<= 1;
						} else if (u > v) {
							u -= v;
							u >>= 1;
							r += s;
							s <<= 1;
						} else {
							v -= u;
							v >>= 1;
							s += r;
							r <<= 1;
						}
						k++;
					}

					if (r >= opM) {
						r -= opM;
					}
					r = opM - r;

					k -= N;

INV_MOD_II:
					for (int i = 0; i < k; i++) {
#pragma HLS loop_tripcount max = 256
						if (r[0] == 1) {
							r += opM;
						}
						r >>= 1;
					}

					ap_uint<N> res = monProduct<N>(r.range(N - 1, 0), 1, opM);

					return res;
				}

		} 
	} 
} 



namespace xf {
	namespace security {

		/**
		 * @brief RSA encryption/decryption class
		 *
		 * @tparam N BitWdith of modulus of key.
		 * @tparam L BitWdith of exponents of RSA encryption and decryption
		 */
		template <int N, int L>
			class rsa {
				public:
					ap_uint<L> exponent;
					ap_uint<N> modulus;
					ap_uint<N> rMod;

					/**
					 * @brief Update key before use it to encrypt message
					 *
					 * @param inputModulus Modulus in RSA public key.
					 * @param inputExponent Exponent in RSA public key or private key.
					 */
					void updateKey(ap_uint<N> inputModulus, ap_uint<L> inputExponent) {
						modulus = inputModulus;
						exponent = inputExponent;

						ap_uint<N + 1> tmp = 0;
						tmp[N] = 1;
						tmp %= inputModulus;

						rMod = xf::security::internal::productMod<N>(tmp, tmp, inputModulus);
					}

					/**
					 * @brief Update key before use it to encrypt message
					 *
					 * @param inputModulus Modulus in RSA public key.
					 * @param inputExponent Exponent in RSA public key or private key.
					 * @param inputRMod 2^(2 * N) mod modulus, pre-calculated by user
					 */
					void updateKey(ap_uint<N> inputModulus, ap_uint<L> inputExponent, ap_uint<N> inputRMod) {
						modulus = inputModulus;
						exponent = inputExponent;
						rMod = inputRMod;
					}

					/**
					 * @brief Encrypt message and get result. It does not include any padding scheme
					 *
					 * @param message Message to be encrypted/decrypted
					 * @param result Generated encrypted/decrypted result.
					 */
					void process(ap_uint<N> message, ap_uint<N>& result) {
						std::cout << __PRETTY_FUNCTION__ << "\n";
						result = xf::security::internal::modularExp<N, L>(message, exponent, modulus, rMod);
					}
			};
	} 
}

const uint8_t input_msg_hash_size { 32 };
xf::security::rsa<512, 512> processor2;
void sign_rsa(ap_uint<512>& hash, ap_uint<512>& result) {
#if 0
	ap_uint<2048> modulus = ap_uint<2048>("0x9d41cd0d38339220ebd110e8c31feb279c5fae3c23090045a0886301588d4c8114fa5cdde708ea77ba0f527e6f6ea8f5634acf517f04"
			"ca6399e188d5c2d7f03cc90e04dbf7d5d0056ee1b14b8baaf90ef78f5142ddce9ba2eff84c0295f656c29aecaae80ddd5c7127ddc60215"
			"9458f272316100f726a71362516223f26ddeafa425d3eb2c7f61de7e8586e77d475037563425d931885f03693618bb885ab9b58de74f60"
			"4a86f28e494dcd819bd8c0bb42f699596969b84f680819e4c9fc0ba687558775f770a302d5b266905defe47bc53c98ce261523b49db624"
			"1567f4b48c661482ef9c453750c6d420a0b1a3bd4d3d05b060c026ce8efd9bb9456dfe2f5d");
#endif
/*
	ap_uint<512> modulus = ap_uint<512>("0x967d3d9b28a7a296a0b843ea4ac9724f572701d75b81d134d111068554387bcc3b68afeba9ecd7123769a652c9915162f84cb692299eed24dc96457189c823ed");

	ap_uint<20> exponent = ap_uint<512>("0x10001");


	// get test result
	xf::security::rsa<512, 20> processor;
	processor.updateKey(modulus, exponent);
	processor.process(hash, result);
*/

	static bool flag = false;
	if (!flag) {
	//https://www.mobilefish.com/services/rsa_key_generation/rsa_key_generation.php
	ap_uint<512> modulus_d = ap_uint<512>("0x967d3d9b28a7a296a0b843ea4ac9724f572701d75b81d134d111068554387bcc3b68afeba9ecd7123769a652c9915162f84cb692299eed24dc96457189c823ed");

	ap_uint<512> exponent_d = ap_uint<512>("0x8b352f57e833a9e34ca927aa03a475698f6535fed15734ff76a619321cc6633d07c63b3587419c053aa572266993a5e4695eec705c375106b654830e2b0abed");
	std::cout << __PRETTY_FUNCTION__ << ": calculate modulus\n";

#ifdef PRINT_DEBUG
	std::cout << __PRETTY_FUNCTION__ << "\n";
#endif

//	ap_uint<512> res = ap_uint<512>("0");
	// get test result
	processor2.updateKey(modulus_d, exponent_d);
	flag = true;
	}
	processor2.process(hash, result);
}


static ap_uint<512> transform_to_apuint(uint8_t* h) {
	ap_uint<512> h1 = ap_uint<512>("0");
	for (auto i = 0; i < input_msg_hash_size; i++) {
		h1[i] = h[i] + 1;
	}
	return h1;
}

static void transform_to_array(ap_uint<512>& res, uint8_t* output) {
	for (auto i = 0; i < input_msg_hash_size; i++) {
		output[i] = res[i];
	}
}

void helper(uint8_t* h, uint8_t* output) {
	ap_uint<512> h1 = transform_to_apuint(h);
	ap_uint<512> h2 = ap_uint<512>("0");
	sign_rsa(h1, h2);
	transform_to_array(h2, output);
	// std::cout << __PRETTY_FUNCTION__ << "\n";
}
