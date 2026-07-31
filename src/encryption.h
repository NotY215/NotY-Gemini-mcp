#pragma once
#include <string>
#include <vector>

class Encryption {
private:
    static std::vector<unsigned char> key;
    static std::vector<unsigned char> iv;
    static bool initialized;

public:
    static void init();
    static std::string encrypt(const std::string& plaintext);
    static std::string decrypt(const std::string& ciphertext);
    static bool isInitialized() { return initialized; }
};