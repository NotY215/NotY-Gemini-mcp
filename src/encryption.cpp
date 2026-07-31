#include "encryption.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstring>

std::vector<unsigned char> Encryption::key;
std::vector<unsigned char> Encryption::iv;
bool Encryption::initialized = false;

void Encryption::init() {
    if (initialized) return;

    // Generate a random key (256-bit)
    key.resize(32);
    if (RAND_bytes(key.data(), key.size()) != 1) {
        throw std::runtime_error("Failed to generate encryption key");
    }

    // Generate random IV (128-bit)
    iv.resize(16);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate IV");
    }

    initialized = true;
}

std::string Encryption::encrypt(const std::string& plaintext) {
    if (!initialized) {
        throw std::runtime_error("Encryption not initialized");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                         reinterpret_cast<const unsigned char*>(plaintext.c_str()), 
                         plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption");
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Convert to base64 for safe storage
    std::string result;
    result.reserve(ciphertext_len * 2 + 1);
    
    for (int i = 0; i < ciphertext_len; ++i) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", ciphertext[i]);
        result += hex;
    }

    return result;
}

std::string Encryption::decrypt(const std::string& ciphertext) {
    if (!initialized) {
        throw std::runtime_error("Encryption not initialized");
    }

    // Convert from hex
    std::vector<unsigned char> encrypted;
    for (size_t i = 0; i < ciphertext.length(); i += 2) {
        std::string byte = ciphertext.substr(i, 2);
        encrypted.push_back(static_cast<unsigned char>(std::stoi(byte, nullptr, 16)));
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    std::vector<unsigned char> plaintext(encrypted.size());
    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, 
                         encrypted.data(), encrypted.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize decryption");
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
}