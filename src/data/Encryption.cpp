/**
 * @file Encryption.cpp
 * @brief Implementacja klasy Encryption - szyfrowanie uwierzytelnione AES-256-GCM
 * 
 * Ten plik zawiera implementację mechanizmów szyfrowania i deszyfrowania danych
 * z wykorzystaniem AES-256-GCM oraz kodowania Base64. Kod zawiera rozbudowane
 * mechanizmy debugowania, obsługi błędów, wyjątków oraz logowania operacji.
 * 
 * @author AI Assistant
 * @date 2025
 */

#include "data/Encryption.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <climits>
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

// ============================================================================
// FLAGI DEBUGOWANIA - można włączać/wyłączać poszczególne moduły
// ============================================================================
#ifndef DEBUG_ENCRYPTION
    #define DEBUG_ENCRYPTION 0          ///< Główna flaga debugowania enkrypcji
#endif

#ifndef DEBUG_XOR_OPERATIONS
    #define DEBUG_XOR_OPERATIONS 0      ///< Flag dla operacji XOR
#endif

#ifndef DEBUG_BASE64_OPERATIONS
    #define DEBUG_BASE64_OPERATIONS 0   ///< Flag dla operacji Base64
#endif

#ifndef DEBUG_KEY_GENERATION
    #define DEBUG_KEY_GENERATION 0      ///< Flag dla generowania kluczy
#endif

#ifndef DEBUG_ERROR_HANDLING
    #define DEBUG_ERROR_HANDLING 0      ///< Flag dla obsługi błędów
#endif

// ============================================================================
// MAKRA POMOCNICZE DO LOGOWANIA I DEBUGOWANIA
// ============================================================================

/**
 * @brief Makro do logowania komunikatów debugowych
 * 
 * Wyświetla komunikat tylko gdy odpowiednia flaga debugowania jest włączona.
 * Format: [DEBUG][timestamp][function] message
 */
#define ENCRYPT_DEBUG(level, msg) \
    do { \
        if (DEBUG_ENCRYPTION && level <= DEBUG_ENCRYPTION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[DEBUG][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

/**
 * @brief Makro do logowania błędów
 * 
 * Zawsze wyświetla komunikaty błędów niezależnie od flag debugowania.
 * Format: [ERROR][timestamp][function] message
 */
#define ENCRYPT_ERROR(msg) \
    do { \
        auto now = std::chrono::system_clock::now(); \
        auto time = std::chrono::system_clock::to_time_t(now); \
        std::stringstream ss; \
        ss << "[ERROR][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
           << __FUNCTION__ << ": " << msg << std::endl; \
        std::cerr << ss.str(); \
    } while(0)

/**
 * @brief Makro do logowania ostrzeżeń
 * 
 * Wyświetla komunikaty ostrzeżeń dla niekrytycznych problemów.
 */
#define ENCRYPT_WARN(msg) \
    do { \
        if (DEBUG_ENCRYPTION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[WARN][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

// ============================================================================
// STAŁE I ZMIENNE GLOBALNE
// ============================================================================

/**
 * @brief Tabela kodowania Base64
 * 
 * Zawiera wszystkie znaki używane w kodowaniu Base64 w poprawnej kolejności.
 * Jest to standardowa tabela zgodna z RFC 4648.
 */
static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// ============================================================================
// INICJALIZACJA ZMIENNYCH STATYCZNYCH KLASY ENCRYPTION
// ============================================================================

EncryptionTypes::EncryptionLogLevel Encryption::m_minimumLogLevel = EncryptionTypes::EncryptionLogLevel::INFO;
bool Encryption::m_consoleOutputEnabled = false;
EncryptionTypes::EncryptionStatistics Encryption::m_statistics;
std::mutex Encryption::m_logMutex;
std::function<void(const std::string&)> Encryption::m_errorCallback;
std::function<void(const std::string&)> Encryption::m_criticalErrorCallback;

// ============================================================================
// IMPLEMENTACJA METOD STATYSTYCZNYCH I KONFIGURACYJNYCH
// ============================================================================

void Encryption::setMinimumLogLevel(EncryptionTypes::EncryptionLogLevel level) {
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_minimumLogLevel = level;
    }
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__,
               "Poziom logowania ustawiony na: " + logLevelToString(level));
}

EncryptionTypes::EncryptionLogLevel Encryption::getMinimumLogLevel() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    return m_minimumLogLevel;
}

void Encryption::setConsoleOutputEnabled(bool enabled) {
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_consoleOutputEnabled = enabled;
    }
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__,
               "Wyjście na konsolę " + std::string(enabled ? "włączone" : "wyłączone"));
}

bool Encryption::isConsoleOutputEnabled() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    return m_consoleOutputEnabled;
}

EncryptionTypes::EncryptionStatistics Encryption::getStatistics() {
    // Tworzenie kopii statystyk z atomic load dla każdego pola
    EncryptionTypes::EncryptionStatistics stats;
    stats.encryptOperations.store(m_statistics.encryptOperations.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.decryptOperations.store(m_statistics.decryptOperations.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.keyGenerations.store(m_statistics.keyGenerations.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.base64EncodeOps.store(m_statistics.base64EncodeOps.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.base64DecodeOps.store(m_statistics.base64DecodeOps.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.xorOperations.store(m_statistics.xorOperations.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.validationErrors.store(m_statistics.validationErrors.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.memoryErrors.store(m_statistics.memoryErrors.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.otherErrors.store(m_statistics.otherErrors.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats.totalBytesProcessed.store(m_statistics.totalBytesProcessed.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return stats;
}


void Encryption::resetStatistics() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_statistics.encryptOperations.store(0, std::memory_order_relaxed);
    m_statistics.decryptOperations.store(0, std::memory_order_relaxed);
    m_statistics.keyGenerations.store(0, std::memory_order_relaxed);
    m_statistics.base64EncodeOps.store(0, std::memory_order_relaxed);
    m_statistics.base64DecodeOps.store(0, std::memory_order_relaxed);
    m_statistics.xorOperations.store(0, std::memory_order_relaxed);
    m_statistics.validationErrors.store(0, std::memory_order_relaxed);
    m_statistics.memoryErrors.store(0, std::memory_order_relaxed);
    m_statistics.otherErrors.store(0, std::memory_order_relaxed);
    m_statistics.totalBytesProcessed.store(0, std::memory_order_relaxed);
#if DEBUG_ENCRYPTION
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, "Statystyki zresetowane");
#endif
}

std::string Encryption::getStatisticsSummary() {
    std::stringstream ss;
    ss << "=== Statystyki Encryption ===" << std::endl;
    ss << "Operacje szyfrowania: " << m_statistics.encryptOperations.load() << std::endl;
    ss << "Operacje deszyfrowania: " << m_statistics.decryptOperations.load() << std::endl;
    ss << "Generowania kluczy: " << m_statistics.keyGenerations.load() << std::endl;
    ss << "Kodowań Base64: " << m_statistics.base64EncodeOps.load() << std::endl;
    ss << "Dekodowań Base64: " << m_statistics.base64DecodeOps.load() << std::endl;
    ss << "Operacji XOR: " << m_statistics.xorOperations.load() << std::endl;
    ss << "Błędów walidacji: " << m_statistics.validationErrors.load() << std::endl;
    ss << "Błędów pamięci: " << m_statistics.memoryErrors.load() << std::endl;
    ss << "Innych błędów: " << m_statistics.otherErrors.load() << std::endl;
    ss << "Przetworzonych bajtów: " << m_statistics.totalBytesProcessed.load() << std::endl;
    return ss.str();
}

void Encryption::setErrorCallback(std::function<void(const std::string&)> callback) {
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_errorCallback = std::move(callback);
    }
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, "Callback błędów zarejestrowany");
}

void Encryption::setCriticalErrorCallback(std::function<void(const std::string&)> callback) {
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_criticalErrorCallback = std::move(callback);
    }
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, "Callback błędów krytycznych zarejestrowany");
}

// ============================================================================
// IMPLEMENTACJA METOD PRYWATNYCH - LOGOWANIE
// ============================================================================

void Encryption::logMessage(EncryptionTypes::EncryptionLogLevel level,
                           const std::string& function,
                           const std::string& message) {
    std::function<void(const std::string&)> errorCallback;
    std::function<void(const std::string&)> criticalCallback;
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        if (level < m_minimumLogLevel || !m_consoleOutputEnabled) return;
        errorCallback = m_errorCallback;
        criticalCallback = m_criticalErrorCallback;
    }

    auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::stringstream ss;
    ss << "[" << logLevelToString(level) << "][ENCRYPTION]["
       << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "] "
       << function << ": " << message << std::endl;
    if (level >= EncryptionTypes::EncryptionLogLevel::ERROR) std::cerr << ss.str();
    else std::cout << ss.str();

    // User callbacks are deliberately invoked outside the logging mutex.
    try {
        if (level == EncryptionTypes::EncryptionLogLevel::CRITICAL && criticalCallback)
            criticalCallback(message);
        else if (level >= EncryptionTypes::EncryptionLogLevel::ERROR && errorCallback)
            errorCallback(message);
    } catch (...) {
        // Logging must not propagate callback exceptions.
    }
}

std::string Encryption::logLevelToString(EncryptionTypes::EncryptionLogLevel level) {
    switch (level) {
        case EncryptionTypes::EncryptionLogLevel::VERBOSE: return "VERBOSE";
        case EncryptionTypes::EncryptionLogLevel::DEBUG: return "DEBUG";
        case EncryptionTypes::EncryptionLogLevel::INFO: return "INFO";
        case EncryptionTypes::EncryptionLogLevel::WARNING: return "WARNING";
        case EncryptionTypes::EncryptionLogLevel::ERROR: return "ERROR";
        case EncryptionTypes::EncryptionLogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

EncryptionTypes::EncryptionLogLevel Encryption::stringToLogLevel(const std::string& levelStr) {
    if (levelStr == "VERBOSE") return EncryptionTypes::EncryptionLogLevel::VERBOSE;
    if (levelStr == "DEBUG") return EncryptionTypes::EncryptionLogLevel::DEBUG;
    if (levelStr == "INFO") return EncryptionTypes::EncryptionLogLevel::INFO;
    if (levelStr == "WARNING") return EncryptionTypes::EncryptionLogLevel::WARNING;
    if (levelStr == "ERROR") return EncryptionTypes::EncryptionLogLevel::ERROR;
    if (levelStr == "CRITICAL") return EncryptionTypes::EncryptionLogLevel::CRITICAL;
    throw std::invalid_argument("Nieznany poziom logowania: " + levelStr);
}

// Implementacja metod EncryptionStatistics
std::string EncryptionTypes::EncryptionStatistics::toString() const {
    std::stringstream ss;
    ss << "Encrypt: " << encryptOperations.load() 
       << ", Decrypt: " << decryptOperations.load()
       << ", Keys: " << keyGenerations.load()
       << ", B64Enc: " << base64EncodeOps.load()
       << ", B64Dec: " << base64DecodeOps.load()
       << ", XOR: " << xorOperations.load()
       << ", ValErr: " << validationErrors.load()
       << ", MemErr: " << memoryErrors.load()
       << ", OthErr: " << otherErrors.load()
       << ", Bytes: " << totalBytesProcessed.load();
    return ss.str();
}

void EncryptionTypes::EncryptionStatistics::reset() {
    encryptOperations.store(0);
    decryptOperations.store(0);
    keyGenerations.store(0);
    base64EncodeOps.store(0);
    base64DecodeOps.store(0);
    xorOperations.store(0);
    validationErrors.store(0);
    memoryErrors.store(0);
    otherErrors.store(0);
    totalBytesProcessed.store(0);
}
std::string Encryption::encrypt(const std::string& data, const std::string& key) {
    if (key.size() < 32) throw std::invalid_argument("Klucz AES musi zawierać co najmniej 32 bajty losowego materiału");
    if (data.size() > MAX_DATA_SIZE || data.size() > static_cast<std::size_t>(INT_MAX))
        throw std::invalid_argument("Dane przekraczają dopuszczalny rozmiar");
    unsigned char aesKey[32]{};
    unsigned int keyLen=0;
    if(EVP_Digest(key.data(),key.size(),aesKey,&keyLen,EVP_sha256(),nullptr)!=1||keyLen!=32) {
        OPENSSL_cleanse(aesKey,sizeof(aesKey));
        throw std::runtime_error("Nie udało się wyprowadzić klucza AES-256");
    }
    unsigned char nonce[12]{}, tag[16]{};
    if(RAND_bytes(nonce,sizeof(nonce))!=1) {
        OPENSSL_cleanse(aesKey,sizeof(aesKey));
        throw std::runtime_error("CSPRNG nie wygenerował nonce");
    }
    EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
    if(!ctx) {
        OPENSSL_cleanse(aesKey,sizeof(aesKey));
        throw std::runtime_error("Nie można utworzyć kontekstu AES-GCM");
    }
    std::vector<unsigned char> ciphertext(data.size()+16);
    int outLen=0,total=0;
    bool ok=EVP_EncryptInit_ex(ctx,EVP_aes_256_gcm(),nullptr,nullptr,nullptr)==1 &&
        EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_GCM_SET_IVLEN,sizeof(nonce),nullptr)==1 &&
        EVP_EncryptInit_ex(ctx,nullptr,nullptr,aesKey,nonce)==1 &&
        EVP_EncryptUpdate(ctx,ciphertext.data(),&outLen,
            reinterpret_cast<const unsigned char*>(data.data()),static_cast<int>(data.size()))==1;
    total=outLen;
    if(ok)ok=EVP_EncryptFinal_ex(ctx,ciphertext.data()+total,&outLen)==1;
    total+=outLen;
    if(ok)ok=EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_GCM_GET_TAG,sizeof(tag),tag)==1;
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(aesKey,sizeof(aesKey));
    if(!ok)throw std::runtime_error("Szyfrowanie AES-256-GCM nie powiodło się");
    std::vector<unsigned char> payload;
    payload.insert(payload.end(),nonce,nonce+sizeof(nonce));
    payload.insert(payload.end(),tag,tag+sizeof(tag));
    payload.insert(payload.end(),ciphertext.begin(),ciphertext.begin()+total);
    if(payload.size()>static_cast<std::size_t>(INT_MAX))throw std::runtime_error("Wynik szyfrowania jest zbyt duży");
    std::string encoded(4*((payload.size()+2)/3)+1,'\0');
    const int n=EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&encoded[0]),payload.data(),static_cast<int>(payload.size()));
    if(n<0)throw std::runtime_error("Kodowanie Base64 nie powiodło się");
    encoded.resize(static_cast<std::size_t>(n));
    m_statistics.encryptOperations.fetch_add(1);
    m_statistics.totalBytesProcessed.fetch_add(data.size());
    return "BFENC:v1:"+encoded;
}
std::string Encryption::decrypt(const std::string& encryptedData, const std::string& key) {
    static const std::string prefix="BFENC:v1:";
    if(key.size()<32)throw std::invalid_argument("Klucz AES musi zawierać co najmniej 32 bajty losowego materiału");
    if(encryptedData.compare(0,prefix.size(),prefix)!=0)
        throw std::invalid_argument("Nieobsługiwany format szyfrogramu; migracja danych legacy musi być jawna");
    const std::string encoded=encryptedData.substr(prefix.size());
    if(encoded.empty()||encoded.size()%4!=0||encoded.size()>static_cast<std::size_t>(INT_MAX))
        throw std::invalid_argument("Nieprawidłowy format Base64 szyfrogramu");
    std::vector<unsigned char> payload((encoded.size()/4)*3);
    const int decoded=EVP_DecodeBlock(payload.data(),reinterpret_cast<const unsigned char*>(encoded.data()),static_cast<int>(encoded.size()));
    if(decoded<0)throw std::invalid_argument("Nieprawidłowy Base64 szyfrogramu");
    std::size_t payloadLen=static_cast<std::size_t>(decoded);
    if(!encoded.empty()&&encoded.back()=='=')--payloadLen;
    if(encoded.size()>1&&encoded[encoded.size()-2]=='=')--payloadLen;
    if(payloadLen<28||payloadLen>MAX_DATA_SIZE+28)throw std::invalid_argument("Nieprawidłowa długość szyfrogramu");
    unsigned char aesKey[32]{}; unsigned int keyLen=0;
    if(EVP_Digest(key.data(),key.size(),aesKey,&keyLen,EVP_sha256(),nullptr)!=1||keyLen!=32) {
        OPENSSL_cleanse(aesKey,sizeof(aesKey));
        throw std::runtime_error("Nie udało się wyprowadzić klucza AES-256");
    }
    const unsigned char* nonce=payload.data();
    const unsigned char* tag=payload.data()+12;
    const unsigned char* cipher=payload.data()+28;
    const std::size_t cipherLen=payloadLen-28;
    EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
    if(!ctx){OPENSSL_cleanse(aesKey,sizeof(aesKey));throw std::runtime_error("Nie można utworzyć kontekstu AES-GCM");}
    std::vector<unsigned char> plainBuffer(cipherLen+16);
    int outLen=0,total=0;
    bool ok=EVP_DecryptInit_ex(ctx,EVP_aes_256_gcm(),nullptr,nullptr,nullptr)==1 &&
        EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_GCM_SET_IVLEN,12,nullptr)==1 &&
        EVP_DecryptInit_ex(ctx,nullptr,nullptr,aesKey,nonce)==1 &&
        EVP_DecryptUpdate(ctx,plainBuffer.data(),&outLen,cipher,static_cast<int>(cipherLen))==1;
    total=outLen;
    if(ok)ok=EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_GCM_SET_TAG,16,const_cast<unsigned char*>(tag))==1;
    if(ok)ok=EVP_DecryptFinal_ex(ctx,plainBuffer.data()+total,&outLen)==1;
    total+=outLen;
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(aesKey,sizeof(aesKey));
    if(!ok){OPENSSL_cleanse(plainBuffer.data(),plainBuffer.size());throw std::runtime_error("Weryfikacja tagu AES-256-GCM nie powiodła się");}
    std::string plaintext(reinterpret_cast<const char*>(plainBuffer.data()),static_cast<std::size_t>(total));
    OPENSSL_cleanse(plainBuffer.data(),plainBuffer.size());
    m_statistics.decryptOperations.fetch_add(1);
    m_statistics.totalBytesProcessed.fetch_add(plaintext.size());
    return plaintext;
}

std::string Encryption::generateKey(size_t length) {
    if(length==0)length=32;
    if(length>1024)throw std::invalid_argument("Żądana długość klucza przekracza limit");
    std::vector<unsigned char> bytes(length);
    if(RAND_bytes(bytes.data(),static_cast<int>(bytes.size()))!=1)
        throw std::runtime_error("CSPRNG nie wygenerował klucza");
    static const char hex[]="0123456789abcdef";
    std::string result; result.reserve(length*2);
    for(unsigned char b:bytes){result.push_back(hex[b>>4]);result.push_back(hex[b&15]);}
    m_statistics.keyGenerations.fetch_add(1);
    OPENSSL_cleanse(bytes.data(),bytes.size());
    return result;
}

// ============================================================================
// IMPLEMENTACJA METOD PRYWATNYCH
// ============================================================================

std::string Encryption::base64Encode(const std::vector<uint8_t>& input) {
    #if DEBUG_BASE64_OPERATIONS
    ENCRYPT_DEBUG(2, "Rozpoczynanie kodowania Base64");
    ENCRYPT_DEBUG(3, "Liczba bajtów wejściowych: " << input.size());
    #endif
    
    // Walidacja danych wejściowych
    if (input.empty()) {
        ENCRYPT_DEBUG(3, "Puste dane wejściowe - zwracanie pustego stringa");
        return "";  // Gentle code: puste dane -> pusty wynik
    }
    
    std::string encoded;
    
    try {
        // Rezerwacja pamięci z wyprzedzeniem (Base64 zwiększa rozmiar o ~33%)
        encoded.reserve(((input.size() + 2) / 3) * 4);
        
        int val = 0;
        int valb = -6;
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t charCount = 0;
        #endif
        
        for (uint8_t c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                charCount++;
                #endif
            }
        }
        
        if (valb > -6) {
            encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
            #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
            charCount++;
            #endif
        }
        
        // Dodawanie paddingu (znaki '=')
        size_t paddingNeeded = (4 - (encoded.size() % 4)) % 4;
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(3, "Dodawanie " << paddingNeeded << " znaków paddingu '='");
        #endif
        
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t finalCharCount = charCount;
        #endif
        
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(2, "Kodowanie Base64 zakończone");
        ENCRYPT_DEBUG(3, "Liczba zakodowanych znaków: " << encoded.length());
        #if DEBUG_ENCRYPTION >= 3
        ENCRYPT_DEBUG(3, "Znaki danych: " << finalCharCount << ", Padding: " << paddingNeeded);
        #endif
        #endif
        
        return encoded;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla kodowania Base64");
    } catch (const std::out_of_range& e) {
        ENCRYPT_ERROR("Błąd indeksu podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Błąd indeksu przy dostępie do tabeli Base64");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas kodowania Base64");
    }
}

std::vector<uint8_t> Encryption::base64Decode(const std::string& input) {
    #if DEBUG_BASE64_OPERATIONS
    ENCRYPT_DEBUG(2, "Rozpoczynanie dekodowania Base64");
    ENCRYPT_DEBUG(3, "Długość danych wejściowych: " << input.length() << " znaków");
    #endif
    
    // Walidacja danych wejściowych
    if (input.empty()) {
        ENCRYPT_DEBUG(3, "Puste dane wejściowe - zwracanie pustego wektora");
        return std::vector<uint8_t>();  // Gentle code: puste dane -> pusty wektor
    }
    
    std::vector<uint8_t> decoded;
    std::vector<int> T(256, -1);
    
    try {
        // Inicjalizacja tabeli dekodującej
        ENCRYPT_DEBUG(3, "Inicjalizacja tabeli dekodującej Base64");
        for (int i = 0; i < 64; i++) {
            T[base64_chars[i]] = i;
        }
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t byteCount = 0;
        size_t ignoredChars = 0;
        #endif
        
        int val = 0;
        int valb = -8;
        
        for (char c : input) {
            if (T[c] == -1) {
                // Ignoruj znaki inne niż Base64 (w tym '=', spacje, nowe linie)
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                ignoredChars++;
                #endif
                continue;
            }
            
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                uint8_t decodedByte = static_cast<uint8_t>((val >> valb) & 0xFF);
                decoded.push_back(decodedByte);
                valb -= 8;
                
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                byteCount++;
                if (byteCount <= 10 || byteCount % 100 == 0) {
                    ENCRYPT_DEBUG(3, "Zdekodowano bajt #" << (byteCount-1) 
                                << ": 0x" << std::hex << static_cast<int>(decodedByte) << std::dec);
                }
                #endif
            }
        }
        
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(2, "Dekodowanie Base64 zakończone");
        ENCRYPT_DEBUG(3, "Liczba zdekodowanych bajtów: " << decoded.size());
        #if DEBUG_ENCRYPTION >= 3
        ENCRYPT_DEBUG(3, "Zignorowane znaki: " << ignoredChars);
        #endif
        #endif
        
        return decoded;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla dekodowania Base64");
    } catch (const std::out_of_range& e) {
        ENCRYPT_ERROR("Błąd indeksu podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Błąd indeksu przy dostępie do tabeli dekodującej");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas dekodowania Base64");
    }
}

