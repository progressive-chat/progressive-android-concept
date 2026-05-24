#ifndef OLM_OLM_H
#define OLM_OLM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==== Opaque types ====

typedef struct OlmAccount OlmAccount;
typedef struct OlmSession OlmSession;
typedef struct OlmInboundGroupSession OlmInboundGroupSession;
typedef struct OlmSAS OlmSAS;

// ==== Global ====

size_t olm_error(void);

// ==== OlmAccount ====

size_t olm_account_size(void);
OlmAccount* olm_account(void* memory);
size_t olm_clear_account(OlmAccount* account);
size_t olm_create_account_random_length(OlmAccount* account);
size_t olm_create_account(OlmAccount* account, const void* random, size_t random_length);
size_t olm_account_identity_keys_length(OlmAccount* account);
size_t olm_account_identity_keys(OlmAccount* account, void* output, size_t output_length);
size_t olm_account_signature_length(OlmAccount* account);
size_t olm_account_sign(OlmAccount* account, const void* message, size_t message_length,
                        void* output, size_t output_length);
size_t olm_pickle_account_length(OlmAccount* account);
size_t olm_pickle_account(OlmAccount* account, const void* key, size_t key_length,
                          void* output, size_t output_length);
size_t olm_unpickle_account(OlmAccount* account, const void* key, size_t key_length,
                            const void* pickled, size_t pickled_length);
size_t olm_account_max_number_of_one_time_keys(OlmAccount* account);
size_t olm_account_generate_one_time_keys_random_length(OlmAccount* account, size_t number_of_keys);
size_t olm_account_generate_one_time_keys(OlmAccount* account, size_t number_of_keys,
                                          const void* random, size_t random_length);
const char* olm_account_last_error(OlmAccount* account);
size_t olm_account_one_time_keys_length(OlmAccount* account);
size_t olm_account_one_time_keys(OlmAccount* account, void* output, size_t output_length);
size_t olm_account_generate_fallback_key_random_length(OlmAccount* account);
size_t olm_account_generate_fallback_key(OlmAccount* account, const void* random, size_t random_length);
size_t olm_account_fallback_key_length(OlmAccount* account);
size_t olm_account_fallback_key(OlmAccount* account, void* output, size_t output_length);
size_t olm_account_mark_keys_as_published(OlmAccount* account);

// ==== OlmSession ====

size_t olm_session_size(void);
OlmSession* olm_session(void* memory);
size_t olm_clear_session(OlmSession* session);
size_t olm_create_outbound_session_random_length(OlmSession* session);
size_t olm_create_outbound_session(OlmSession* session, OlmAccount* account,
                                   const void* their_identity_key, size_t their_identity_key_length,
                                   const void* their_one_time_key, size_t their_one_time_key_length,
                                   const void* random, size_t random_length);
size_t olm_create_inbound_session(OlmSession* session, OlmAccount* account,
                                  const void* one_time_key_message, size_t message_length);
size_t olm_create_inbound_session_from(OlmSession* session, OlmAccount* account,
                                       const void* their_identity_key, size_t their_identity_key_length,
                                       const void* one_time_key_message, size_t message_length);
size_t olm_encrypt_random_length(OlmSession* session);
size_t olm_encrypt_message_length(OlmSession* session, size_t plaintext_length);
size_t olm_encrypt(OlmSession* session, const void* plaintext, size_t plaintext_length,
                   const void* random, size_t random_length,
                   void* message, size_t message_length);
size_t olm_encrypt_message_type(OlmSession* session);
size_t olm_decrypt_max_plaintext_length(OlmSession* session, size_t message_type,
                                        const void* message, size_t message_length);
size_t olm_decrypt(OlmSession* session, size_t message_type,
                   const void* message, size_t message_length,
                   void* plaintext, size_t max_plaintext_length);
size_t olm_pickle_session_length(OlmSession* session);
size_t olm_pickle_session(OlmSession* session, const void* key, size_t key_length,
                          void* output, size_t output_length);
size_t olm_unpickle_session(OlmSession* session, const void* key, size_t key_length,
                            const void* pickled, size_t pickled_length);
size_t olm_matches_inbound_session(OlmSession* session, const void* one_time_key_message, size_t message_length);
size_t olm_session_id_length(OlmSession* session);
size_t olm_session_id(OlmSession* session, void* output, size_t output_length);
const char* olm_session_last_error(OlmSession* session);

// ==== OlmInboundGroupSession (Megolm) ====

size_t olm_inbound_group_session_size(void);
OlmInboundGroupSession* olm_inbound_group_session(void* memory);
size_t olm_clear_inbound_group_session(OlmInboundGroupSession* session);
size_t olm_import_inbound_group_session(OlmInboundGroupSession* session,
                                        const void* session_key, size_t session_key_length);
size_t olm_inbound_group_session_id_length(OlmInboundGroupSession* session);
size_t olm_inbound_group_session_id(OlmInboundGroupSession* session, void* output, size_t output_length);
size_t olm_inbound_group_session_first_known_index(OlmInboundGroupSession* session);
size_t olm_group_decrypt_max_plaintext_length(OlmInboundGroupSession* session,
                                              void* message, size_t message_length);
size_t olm_group_decrypt(OlmInboundGroupSession* session, void* message, size_t message_length,
                         void* plaintext, size_t max_plaintext_length, uint32_t* message_index);
size_t olm_export_inbound_group_session_length(OlmInboundGroupSession* session);
size_t olm_export_inbound_group_session(OlmInboundGroupSession* session, void* key, size_t key_length,
                                        uint32_t message_index);
const char* olm_inbound_group_session_last_error(OlmInboundGroupSession* session);

// ==== OlmSAS ====

size_t olm_sas_size(void);
OlmSAS* olm_sas(void* memory);
size_t olm_clear_sas(OlmSAS* sas);
size_t olm_create_sas_random_length(OlmSAS* sas);
size_t olm_create_sas(OlmSAS* sas, const void* random, size_t random_length);
size_t olm_sas_pubkey_length(OlmSAS* sas);
size_t olm_sas_get_pubkey(OlmSAS* sas, void* output, size_t output_length);
size_t olm_sas_set_their_key(OlmSAS* sas, const void* their_key, size_t their_key_length);
size_t olm_sas_generate_bytes(OlmSAS* sas, const void* extra_info, size_t extra_info_length,
                              void* output, size_t output_length);
size_t olm_sas_mac_length(OlmSAS* sas);
size_t olm_sas_calculate_mac(OlmSAS* sas, const void* input, size_t input_length,
                             const void* info, size_t info_length,
                             void* output, size_t output_length);
size_t olm_sas_calculate_mac_long_kdf(OlmSAS* sas, const void* input, size_t input_length,
                                      const void* info, size_t info_length,
                                      void* output, size_t output_length);
const char* olm_sas_last_error(OlmSAS* sas);

// ==== Crypto utilities ====

void _olm_crypto_sha256(const uint8_t* input, size_t input_length, uint8_t* output);
void _olm_crypto_hmac_sha256(const uint8_t* key, size_t key_length,
                             const uint8_t* input, size_t input_length,
                             uint8_t* output);
void _olm_crypto_hkdf_sha256(const uint8_t* input, size_t input_length,
                             const uint8_t* salt, size_t salt_length,
                             const uint8_t* info, size_t info_length,
                             uint8_t* output, size_t output_length);

#ifdef __cplusplus
}
#endif

#endif // OLM_OLM_H
