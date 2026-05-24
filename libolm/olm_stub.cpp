#include <olm/olm.h>
#include <cstddef>
#include <cstdint>

// ==== Opaque struct definitions ====

struct OlmAccount               { uint8_t _[1024]; };
struct OlmSession               { uint8_t _[2048]; };
struct OlmInboundGroupSession   { uint8_t _[1024]; };
struct OlmSAS                   { uint8_t _[1024]; };

// ==== Global ====

size_t olm_error(void) {
    return static_cast<size_t>(-1);
}

// ==== OlmAccount ====

size_t olm_account_size(void) {
    return sizeof(OlmAccount);
}

OlmAccount* olm_account(void* memory) {
    return static_cast<OlmAccount*>(memory);
}

size_t olm_clear_account(OlmAccount* account) {
    (void)account;
    return 0;
}

size_t olm_create_account_random_length(OlmAccount* account) {
    (void)account;
    return 64;
}

size_t olm_create_account(OlmAccount* account, const void* random, size_t random_length) {
    (void)account; (void)random; (void)random_length;
    return 0;
}

size_t olm_account_identity_keys_length(OlmAccount* account) {
    (void)account;
    return 256;
}

size_t olm_account_identity_keys(OlmAccount* account, void* output, size_t output_length) {
    (void)account; (void)output; (void)output_length;
    return output_length;
}

size_t olm_account_signature_length(OlmAccount* account) {
    (void)account;
    return 64;
}

size_t olm_account_sign(OlmAccount* account, const void* message, size_t message_length,
                        void* output, size_t output_length) {
    (void)account; (void)message; (void)message_length; (void)output;
    return output_length;
}

size_t olm_pickle_account_length(OlmAccount* account) {
    (void)account;
    return 512;
}

size_t olm_pickle_account(OlmAccount* account, const void* key, size_t key_length,
                          void* output, size_t output_length) {
    (void)account; (void)key; (void)key_length; (void)output;
    return output_length;
}

size_t olm_unpickle_account(OlmAccount* account, const void* key, size_t key_length,
                            const void* pickled, size_t pickled_length) {
    (void)account; (void)key; (void)key_length; (void)pickled; (void)pickled_length;
    return 0;
}

size_t olm_account_max_number_of_one_time_keys(OlmAccount* account) {
    (void)account;
    return 100;
}

size_t olm_account_generate_one_time_keys_random_length(OlmAccount* account, size_t number_of_keys) {
    (void)account; (void)number_of_keys;
    return 64;
}

size_t olm_account_generate_one_time_keys(OlmAccount* account, size_t number_of_keys,
                                          const void* random, size_t random_length) {
    (void)account; (void)number_of_keys; (void)random; (void)random_length;
    return 0;
}

const char* olm_account_last_error(OlmAccount* account) {
    (void)account;
    return nullptr;
}

size_t olm_account_one_time_keys_length(OlmAccount* account) {
    (void)account;
    return 256;
}

size_t olm_account_one_time_keys(OlmAccount* account, void* output, size_t output_length) {
    (void)account; (void)output;
    return output_length;
}

size_t olm_account_generate_fallback_key_random_length(OlmAccount* account) {
    (void)account;
    return 64;
}

size_t olm_account_generate_fallback_key(OlmAccount* account, const void* random, size_t random_length) {
    (void)account; (void)random; (void)random_length;
    return 0;
}

size_t olm_account_fallback_key_length(OlmAccount* account) {
    (void)account;
    return 256;
}

size_t olm_account_fallback_key(OlmAccount* account, void* output, size_t output_length) {
    (void)account; (void)output;
    return output_length;
}

size_t olm_account_mark_keys_as_published(OlmAccount* account) {
    (void)account;
    return 0;
}

// ==== OlmSession ====

size_t olm_session_size(void) {
    return sizeof(OlmSession);
}

OlmSession* olm_session(void* memory) {
    return static_cast<OlmSession*>(memory);
}

size_t olm_clear_session(OlmSession* session) {
    (void)session;
    return 0;
}

size_t olm_create_outbound_session_random_length(OlmSession* session) {
    (void)session;
    return 64;
}

size_t olm_create_outbound_session(OlmSession* session, OlmAccount* account,
                                   const void* their_identity_key, size_t their_identity_key_length,
                                   const void* their_one_time_key, size_t their_one_time_key_length,
                                   const void* random, size_t random_length) {
    (void)session; (void)account;
    (void)their_identity_key; (void)their_identity_key_length;
    (void)their_one_time_key; (void)their_one_time_key_length;
    (void)random; (void)random_length;
    return 0;
}

size_t olm_create_inbound_session(OlmSession* session, OlmAccount* account,
                                  const void* one_time_key_message, size_t message_length) {
    (void)session; (void)account; (void)one_time_key_message; (void)message_length;
    return 0;
}

size_t olm_create_inbound_session_from(OlmSession* session, OlmAccount* account,
                                       const void* their_identity_key, size_t their_identity_key_length,
                                       const void* one_time_key_message, size_t message_length) {
    (void)session; (void)account;
    (void)their_identity_key; (void)their_identity_key_length;
    (void)one_time_key_message; (void)message_length;
    return 0;
}

size_t olm_encrypt_random_length(OlmSession* session) {
    (void)session;
    return 64;
}

size_t olm_encrypt_message_length(OlmSession* session, size_t plaintext_length) {
    (void)session;
    return plaintext_length + 64;
}

size_t olm_encrypt(OlmSession* session, const void* plaintext, size_t plaintext_length,
                   const void* random, size_t random_length,
                   void* message, size_t message_length) {
    (void)session; (void)plaintext; (void)plaintext_length;
    (void)random; (void)random_length; (void)message;
    return message_length;
}

size_t olm_encrypt_message_type(OlmSession* session) {
    (void)session;
    return 1;
}

size_t olm_decrypt_max_plaintext_length(OlmSession* session, size_t message_type,
                                        const void* message, size_t message_length) {
    (void)session; (void)message_type; (void)message;
    return message_length;
}

size_t olm_decrypt(OlmSession* session, size_t message_type,
                   const void* message, size_t message_length,
                   void* plaintext, size_t max_plaintext_length) {
    (void)session; (void)message_type; (void)message; (void)message_length; (void)plaintext;
    return max_plaintext_length;
}

size_t olm_pickle_session_length(OlmSession* session) {
    (void)session;
    return 512;
}

size_t olm_pickle_session(OlmSession* session, const void* key, size_t key_length,
                          void* output, size_t output_length) {
    (void)session; (void)key; (void)key_length; (void)output;
    return output_length;
}

size_t olm_unpickle_session(OlmSession* session, const void* key, size_t key_length,
                            const void* pickled, size_t pickled_length) {
    (void)session; (void)key; (void)key_length; (void)pickled; (void)pickled_length;
    return 0;
}

size_t olm_matches_inbound_session(OlmSession* session, const void* one_time_key_message, size_t message_length) {
    (void)session; (void)one_time_key_message; (void)message_length;
    return 0;
}

size_t olm_session_id_length(OlmSession* session) {
    (void)session;
    return 32;
}

size_t olm_session_id(OlmSession* session, void* output, size_t output_length) {
    (void)session; (void)output;
    return output_length;
}

const char* olm_session_last_error(OlmSession* session) {
    (void)session;
    return nullptr;
}

// ==== OlmInboundGroupSession (Megolm) ====

size_t olm_inbound_group_session_size(void) {
    return sizeof(OlmInboundGroupSession);
}

OlmInboundGroupSession* olm_inbound_group_session(void* memory) {
    return static_cast<OlmInboundGroupSession*>(memory);
}

size_t olm_clear_inbound_group_session(OlmInboundGroupSession* session) {
    (void)session;
    return 0;
}

size_t olm_import_inbound_group_session(OlmInboundGroupSession* session,
                                        const void* session_key, size_t session_key_length) {
    (void)session; (void)session_key; (void)session_key_length;
    return 0;
}

size_t olm_inbound_group_session_id_length(OlmInboundGroupSession* session) {
    (void)session;
    return 32;
}

size_t olm_inbound_group_session_id(OlmInboundGroupSession* session, void* output, size_t output_length) {
    (void)session; (void)output;
    return output_length;
}

size_t olm_inbound_group_session_first_known_index(OlmInboundGroupSession* session) {
    (void)session;
    return 0;
}

size_t olm_group_decrypt_max_plaintext_length(OlmInboundGroupSession* session,
                                              void* message, size_t message_length) {
    (void)session; (void)message;
    return message_length;
}

size_t olm_group_decrypt(OlmInboundGroupSession* session, void* message, size_t message_length,
                         void* plaintext, size_t max_plaintext_length, uint32_t* message_index) {
    (void)session; (void)message; (void)message_length; (void)plaintext;
    if (message_index) *message_index = 0;
    return max_plaintext_length;
}

size_t olm_export_inbound_group_session_length(OlmInboundGroupSession* session) {
    (void)session;
    return 256;
}

size_t olm_export_inbound_group_session(OlmInboundGroupSession* session, void* key, size_t key_length,
                                        uint32_t message_index) {
    (void)session; (void)key; (void)key_length; (void)message_index;
    return key_length;
}

const char* olm_inbound_group_session_last_error(OlmInboundGroupSession* session) {
    (void)session;
    return nullptr;
}

// ==== OlmSAS ====

size_t olm_sas_size(void) {
    return sizeof(OlmSAS);
}

OlmSAS* olm_sas(void* memory) {
    return static_cast<OlmSAS*>(memory);
}

size_t olm_clear_sas(OlmSAS* sas) {
    (void)sas;
    return 0;
}

size_t olm_create_sas_random_length(OlmSAS* sas) {
    (void)sas;
    return 64;
}

size_t olm_create_sas(OlmSAS* sas, const void* random, size_t random_length) {
    (void)sas; (void)random; (void)random_length;
    return 0;
}

size_t olm_sas_pubkey_length(OlmSAS* sas) {
    (void)sas;
    return 32;
}

size_t olm_sas_get_pubkey(OlmSAS* sas, void* output, size_t output_length) {
    (void)sas; (void)output;
    return output_length;
}

size_t olm_sas_set_their_key(OlmSAS* sas, const void* their_key, size_t their_key_length) {
    (void)sas; (void)their_key; (void)their_key_length;
    return 0;
}

size_t olm_sas_generate_bytes(OlmSAS* sas, const void* extra_info, size_t extra_info_length,
                              void* output, size_t output_length) {
    (void)sas; (void)extra_info; (void)extra_info_length; (void)output;
    return output_length;
}

size_t olm_sas_mac_length(OlmSAS* sas) {
    (void)sas;
    return 32;
}

size_t olm_sas_calculate_mac(OlmSAS* sas, const void* input, size_t input_length,
                             const void* info, size_t info_length,
                             void* output, size_t output_length) {
    (void)sas; (void)input; (void)input_length; (void)info; (void)info_length; (void)output;
    return output_length;
}

size_t olm_sas_calculate_mac_long_kdf(OlmSAS* sas, const void* input, size_t input_length,
                                      const void* info, size_t info_length,
                                      void* output, size_t output_length) {
    (void)sas; (void)input; (void)input_length; (void)info; (void)info_length; (void)output;
    return output_length;
}

const char* olm_sas_last_error(OlmSAS* sas) {
    (void)sas;
    return nullptr;
}

// ==== Crypto utilities ====

void _olm_crypto_sha256(const uint8_t* input, size_t input_length, uint8_t* output) {
    (void)input; (void)input_length; (void)output;
}

void _olm_crypto_hmac_sha256(const uint8_t* key, size_t key_length,
                             const uint8_t* input, size_t input_length,
                             uint8_t* output) {
    (void)key; (void)key_length; (void)input; (void)input_length; (void)output;
}

void _olm_crypto_hkdf_sha256(const uint8_t* input, size_t input_length,
                             const uint8_t* salt, size_t salt_length,
                             const uint8_t* info, size_t info_length,
                             uint8_t* output, size_t output_length) {
    (void)input; (void)input_length; (void)salt; (void)salt_length;
    (void)info; (void)info_length; (void)output; (void)output_length;
}
