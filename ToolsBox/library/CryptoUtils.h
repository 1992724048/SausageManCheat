#pragma once
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <random>

class CryptoUtils {
public:
    static auto hardware_random_key(std::vector<unsigned char>& key_buffer, const size_t key_size) -> bool {
        std::random_device rd;

        key_buffer.clear();
        key_buffer.reserve(key_size);

        for (size_t i = 0; i < key_size; ++i) {
            key_buffer.push_back(static_cast<unsigned char>(rd()));
        }

        return true;
    }

    /**
     * AES加密 (CBC模式)
     * @param plaintext 明文
     * @param key 密钥(16/24/32字节对应AES-128/192/256)
     * @param iv 初始化向量(16字节)
     * @return 密文
     */
    static auto aes_encrypt(const std::vector<unsigned char>& plaintext,
                            const std::vector<unsigned char>& key,
                            const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
            throw std::runtime_error("Invalid AES key size (must be 16, 24 or 32 bytes)");
        }
        if (iv.size() != AES_BLOCK_SIZE) {
            throw std::runtime_error("Invalid IV size (must be 16 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher;
        if (key.size() == 16) {
            cipher = EVP_aes_128_cbc();
        } else if (key.size() == 24) {
            cipher = EVP_aes_192_cbc();
        } else {
            cipher = EVP_aes_256_cbc();
        }

        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
        int len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }

    /**
     * AES解密 (CBC模式)
     * @param ciphertext 密文
     * @param key 密钥(16/24/32字节对应AES-128/192/256)
     * @param iv 初始化向量(16字节)
     * @return 明文
     */
    static auto aes_decrypt(const std::vector<unsigned char>& ciphertext,
                            const std::vector<unsigned char>& key,
                            const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
            throw std::runtime_error("Invalid AES key size (must be 16, 24 or 32 bytes)");
        }
        if (iv.size() != AES_BLOCK_SIZE) {
            throw std::runtime_error("Invalid IV size (must be 16 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher;
        if (key.size() == 16) {
            cipher = EVP_aes_128_cbc();
        } else if (key.size() == 24) {
            cipher = EVP_aes_192_cbc();
        } else {
            cipher = EVP_aes_256_cbc();
        }

        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptInit_ex failed");
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptUpdate failed");
        }
        int plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptFinal_ex failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        plaintext.resize(plaintext_len);
        return plaintext;
    }

    // ==================== DES/3DES ====================

    /**
     * DES加密 (CBC模式)
     * @param plaintext 明文
     * @param key 密钥(8字节)
     * @param iv 初始化向量(8字节)
     * @return 密文
     */
    static auto des_encrypt(const std::vector<unsigned char>& plaintext,
                            const std::vector<unsigned char>& key,
                            const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 8) {
            throw std::runtime_error("Invalid DES key size (must be 8 bytes)");
        }
        if (iv.size() != 8) {
            throw std::runtime_error("Invalid IV size (must be 8 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        if (EVP_EncryptInit_ex(ctx, EVP_des_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        std::vector<unsigned char> ciphertext(plaintext.size() + 8);
        int len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }

    /**
     * DES解密 (CBC模式)
     * @param ciphertext 密文
     * @param key 密钥(8字节)
     * @param iv 初始化向量(8字节)
     * @return 明文
     */
    static auto des_decrypt(const std::vector<unsigned char>& ciphertext,
                            const std::vector<unsigned char>& key,
                            const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 8) {
            throw std::runtime_error("Invalid DES key size (must be 8 bytes)");
        }
        if (iv.size() != 8) {
            throw std::runtime_error("Invalid IV size (must be 8 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_des_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptInit_ex failed");
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptUpdate failed");
        }
        int plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptFinal_ex failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        plaintext.resize(plaintext_len);
        return plaintext;
    }

    /**
     * 3DES加密 (CBC模式)
     * @param plaintext 明文
     * @param key 密钥(24字节)
     * @param iv 初始化向量(8字节)
     * @return 密文
     */
    static auto triple_des_encrypt(const std::vector<unsigned char>& plaintext,
                                   const std::vector<unsigned char>& key,
                                   const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 24) {
            throw std::runtime_error("Invalid 3DES key size (must be 24 bytes)");
        }
        if (iv.size() != 8) {
            throw std::runtime_error("Invalid IV size (must be 8 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        if (EVP_EncryptInit_ex(ctx, EVP_des_ede3_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        std::vector<unsigned char> ciphertext(plaintext.size() + 8);
        int len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }

    /**
     * 3DES解密 (CBC模式)
     * @param ciphertext 密文
     * @param key 密钥(24字节)
     * @param iv 初始化向量(8字节)
     * @return 明文
     */
    static auto triple_des_decrypt(const std::vector<unsigned char>& ciphertext,
                                   const std::vector<unsigned char>& key,
                                   const std::vector<unsigned char>& iv) -> std::vector<unsigned char> {
        if (key.size() != 24) {
            throw std::runtime_error("Invalid 3DES key size (must be 24 bytes)");
        }
        if (iv.size() != 8) {
            throw std::runtime_error("Invalid IV size (must be 8 bytes)");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_des_ede3_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptInit_ex failed");
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptUpdate failed");
        }
        int plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptFinal_ex failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        plaintext.resize(plaintext_len);
        return plaintext;
    }

    /**
     * 生成RSA密钥对
     * @param bits 密钥长度(通常为1024, 2048, 4096等)
     * @return pair<私钥PEM字符串, 公钥PEM字符串>
     */
    static auto generate_rsa_key_pair(const int bits = 2048) -> std::pair<std::string, std::string> {
        RSA* rsa = RSA_new();
        BIGNUM* bne = BN_new();

        if (!BN_set_word(bne, RSA_F4) || !RSA_generate_key_ex(rsa, bits, bne, nullptr)) {
            BN_free(bne);
            RSA_free(rsa);
            throw std::runtime_error("RSA key generation failed");
        }

        BIO* pri = BIO_new(BIO_s_mem());
        if (!PEM_write_bio_RSAPrivateKey(pri, rsa, nullptr, nullptr, 0, nullptr, nullptr)) {
            BIO_free(pri);
            BN_free(bne);
            RSA_free(rsa);
            throw std::runtime_error("Failed to write private key");
        }

        BIO* pub = BIO_new(BIO_s_mem());
        if (!PEM_write_bio_RSAPublicKey(pub, rsa)) {
            BIO_free(pub);
            BIO_free(pri);
            BN_free(bne);
            RSA_free(rsa);
            throw std::runtime_error("Failed to write public key");
        }

        const size_t pri_len = BIO_pending(pri);
        std::vector<char> pri_buf(pri_len + 1);
        BIO_read(pri, pri_buf.data(), pri_len);
        std::string private_key(pri_buf.begin(), pri_buf.end());

        const size_t pub_len = BIO_pending(pub);
        std::vector<char> pub_buf(pub_len + 1);
        BIO_read(pub, pub_buf.data(), pub_len);
        std::string public_key(pub_buf.begin(), pub_buf.end());

        BIO_free(pri);
        BIO_free(pub);
        BN_free(bne);
        RSA_free(rsa);

        return {private_key, public_key};
    }

    /**
     * RSA公钥加密
     * @param plaintext 明文
     * @param public_key PEM格式的公钥
     * @return 密文
     */
    static auto rsa_encrypt(const std::vector<unsigned char>& plaintext,
                            const std::string& public_key) -> std::vector<unsigned char> {
        BIO* bio = BIO_new_mem_buf(public_key.data(), public_key.size());
        if (!bio) {
            throw std::runtime_error("BIO_new_mem_buf failed");
        }

        RSA* rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!rsa) {
            throw std::runtime_error("PEM_read_bio_RSAPublicKey failed");
        }

        const int rsa_size = RSA_size(rsa);
        std::vector<unsigned char> ciphertext(rsa_size);

        const int result = RSA_public_encrypt(plaintext.size(),
                                              plaintext.data(),
                                              ciphertext.data(),
                                              rsa,
                                              RSA_PKCS1_PADDING);
        RSA_free(rsa);

        if (result == -1) {
            throw std::runtime_error("RSA_public_encrypt failed");
        }

        ciphertext.resize(result);
        return ciphertext;
    }

    /**
     * RSA私钥解密
     * @param ciphertext 密文
     * @param private_key PEM格式的私钥
     * @return 明文
     */
    static auto rsa_decrypt(const std::vector<unsigned char>& ciphertext,
                            const std::string& private_key) -> std::vector<unsigned char> {
        BIO* bio = BIO_new_mem_buf(private_key.data(), private_key.size());
        if (!bio) {
            throw std::runtime_error("BIO_new_mem_buf failed");
        }

        RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!rsa) {
            throw std::runtime_error("PEM_read_bio_RSAPrivateKey failed");
        }

        const int rsa_size = RSA_size(rsa);
        if (ciphertext.size() > static_cast<size_t>(rsa_size)) {
            RSA_free(rsa);
            throw std::runtime_error("Ciphertext too large for RSA key size");
        }

        std::vector<unsigned char> plaintext(rsa_size);

        const int result = RSA_private_decrypt(ciphertext.size(),
                                               ciphertext.data(),
                                               plaintext.data(),
                                               rsa,
                                               RSA_PKCS1_PADDING);
        RSA_free(rsa);

        if (result == -1) {
            throw std::runtime_error("RSA_private_decrypt failed");
        }

        plaintext.resize(result);
        return plaintext;
    }

    /**
     * 生成ECC密钥对
     * @param curve_name 曲线名称(如"prime256v1", "secp384r1", "secp521r1")
     * @return pair<私钥PEM字符串, 公钥PEM字符串>
     */
    static auto generate_ecc_key_pair(const std::string& curve_name = "prime256v1") -> std::pair<std::string, std::string> {
        EC_KEY* ec_key = EC_KEY_new_by_curve_name(OBJ_txt2nid(curve_name.c_str()));
        if (!ec_key) {
            throw std::runtime_error("EC_KEY_new_by_curve_name failed");
        }

        if (!EC_KEY_generate_key(ec_key)) {
            EC_KEY_free(ec_key);
            throw std::runtime_error("EC_KEY_generate_key failed");
        }

        // 写入私钥到内存
        BIO* pri = BIO_new(BIO_s_mem());
        if (!PEM_write_bio_ECPrivateKey(pri, ec_key, nullptr, nullptr, 0, nullptr, nullptr)) {
            BIO_free(pri);
            EC_KEY_free(ec_key);
            throw std::runtime_error("Failed to write ECC private key");
        }

        // 写入公钥到内存
        BIO* pub = BIO_new(BIO_s_mem());
        if (!PEM_write_bio_EC_PUBKEY(pub, ec_key)) {
            BIO_free(pub);
            BIO_free(pri);
            EC_KEY_free(ec_key);
            throw std::runtime_error("Failed to write ECC public key");
        }

        // 获取私钥字符串
        const size_t pri_len = BIO_pending(pri);
        std::vector<char> pri_buf(pri_len + 1);
        BIO_read(pri, pri_buf.data(), pri_len);
        std::string private_key(pri_buf.begin(), pri_buf.end());

        // 获取公钥字符串
        const size_t pub_len = BIO_pending(pub);
        std::vector<char> pub_buf(pub_len + 1);
        BIO_read(pub, pub_buf.data(), pub_len);
        std::string public_key(pub_buf.begin(), pub_buf.end());

        // 释放资源
        BIO_free(pri);
        BIO_free(pub);
        EC_KEY_free(ec_key);

        return {private_key, public_key};
    }

    /**
     * ECC签名
     * @param data 待签名数据
     * @param private_key PEM格式的私钥
     * @return 签名结果
     */
    static auto ecc_sign(const std::vector<unsigned char>& data,
                         const std::string& private_key) -> std::vector<unsigned char> {
        BIO* bio = BIO_new_mem_buf(private_key.data(), private_key.size());
        if (!bio) {
            throw std::runtime_error("BIO_new_mem_buf failed");
        }

        EC_KEY* ec_key = PEM_read_bio_ECPrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!ec_key) {
            throw std::runtime_error("PEM_read_bio_ECPrivateKey failed");
        }

        ECDSA_SIG* sig = ECDSA_do_sign(data.data(), data.size(), ec_key);
        EC_KEY_free(ec_key);
        if (!sig) {
            throw std::runtime_error("ECDSA_do_sign failed");
        }

        // 将签名转换为DER格式
        unsigned char* der = nullptr;
        const int der_len = i2d_ECDSA_SIG(sig, &der);
        if (der_len <= 0) {
            ECDSA_SIG_free(sig);
            throw std::runtime_error("i2d_ECDSA_SIG failed");
        }

        std::vector<unsigned char> signature(der, der + der_len);
        OPENSSL_free(der);
        ECDSA_SIG_free(sig);

        return signature;
    }

    /**
     * ECC验证签名
     * @param data 原始数据
     * @param signature 签名
     * @param public_key PEM格式的公钥
     * @return 验证是否通过
     */
    static auto ecc_verify(const std::vector<unsigned char>& data,
                           const std::vector<unsigned char>& signature,
                           const std::string& public_key) -> bool {
        BIO* bio = BIO_new_mem_buf(public_key.data(), public_key.size());
        if (!bio) {
            throw std::runtime_error("BIO_new_mem_buf failed");
        }

        EC_KEY* ec_key = PEM_read_bio_EC_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!ec_key) {
            throw std::runtime_error("PEM_read_bio_EC_PUBKEY failed");
        }

        // 从DER格式解析签名
        const unsigned char* p = signature.data();
        ECDSA_SIG* sig = d2i_ECDSA_SIG(nullptr, &p, signature.size());
        if (!sig) {
            EC_KEY_free(ec_key);
            throw std::runtime_error("d2i_ECDSA_SIG failed");
        }

        const int result = ECDSA_do_verify(data.data(), data.size(), sig, ec_key);
        ECDSA_SIG_free(sig);
        EC_KEY_free(ec_key);

        if (result == -1) {
            throw std::runtime_error("ECDSA_do_verify failed");
        }

        return result == 1;
    }

    /**
     * MD5哈希
     * @param data 输入数据
     * @return 哈希结果(16字节)
     */
    static auto md5_hash(const std::vector<unsigned char>& data) -> std::vector<unsigned char> {
        std::vector<unsigned char> hash(MD5_DIGEST_LENGTH);
        MD5(data.data(), data.size(), hash.data());
        return hash;
    }

    /**
     * SHA1哈希
     * @param data 输入数据
     * @return 哈希结果(20字节)
     */
    static auto sha1_hash(const std::vector<unsigned char>& data) -> std::vector<unsigned char> {
        std::vector<unsigned char> hash(SHA_DIGEST_LENGTH);
        SHA1(data.data(), data.size(), hash.data());
        return hash;
    }

    /**
     * SHA256哈希
     * @param data 输入数据
     * @return 哈希结果(32字节)
     */
    static auto sha256_hash(const std::vector<unsigned char>& data) -> std::vector<unsigned char> {
        std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
        SHA256(data.data(), data.size(), hash.data());
        return hash;
    }

    /**
     * SHA384哈希
     * @param data 输入数据
     * @return 哈希结果(48字节)
     */
    static auto sha384_hash(const std::vector<unsigned char>& data) -> std::vector<unsigned char> {
        std::vector<unsigned char> hash(SHA384_DIGEST_LENGTH);
        SHA384(data.data(), data.size(), hash.data());
        return hash;
    }

    /**
     * SHA512哈希
     * @param data 输入数据
     * @return 哈希结果(64字节)
     */
    static auto sha512_hash(const std::vector<unsigned char>& data) -> std::vector<unsigned char> {
        std::vector<unsigned char> hash(SHA512_DIGEST_LENGTH);
        SHA512(data.data(), data.size(), hash.data());
        return hash;
    }

    /**
     * Base64编码
     * @param data 原始数据
     * @return Base64编码字符串
     */
    static auto base64_encode(const std::vector<unsigned char>& data) -> std::string {
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO* mem = BIO_new(BIO_s_mem());
        BIO_push(b64, mem);

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(b64, data.data(), data.size());
        BIO_flush(b64);

        BUF_MEM* bptr;
        BIO_get_mem_ptr(b64, &bptr);

        std::string result(bptr->data, bptr->length);
        BIO_free_all(b64);

        return result;
    }

    /**
     * Base64解码
     * @param encoded Base64编码字符串
     * @return 原始数据
     */
    static auto base_64decode(const std::string& encoded) -> std::vector<unsigned char> {
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO* mem = BIO_new_mem_buf(encoded.data(), encoded.size());
        BIO_push(b64, mem);

        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

        std::vector<unsigned char> decoded(encoded.size());
        const int len = BIO_read(b64, decoded.data(), decoded.size());
        BIO_free_all(b64);

        if (len <= 0) {
            throw std::runtime_error("Base64 decode failed");
        }

        decoded.resize(len);
        return decoded;
    }

    /**
     * 生成随机字节
     * @param size 字节数
     * @return 随机字节数组
     */
    static auto random_bytes(const size_t size) -> std::vector<unsigned char> {
        std::vector<unsigned char> bytes(size);
        if (RAND_bytes(bytes.data(), size) != 1) {
            throw std::runtime_error("RAND_bytes failed");
        }
        return bytes;
    }

    /**
     * 将十六进制字符串转换为字节数组
     * @param hex 十六进制字符串
     * @return 字节数组
     */
    static auto hex_to_bytes(const std::string& hex) -> std::vector<unsigned char> {
        if (hex.size() % 2 != 0) {
            throw std::runtime_error("Hex string must have even length");
        }

        std::vector<unsigned char> bytes;
        bytes.reserve(hex.size() / 2);

        for (size_t i = 0; i < hex.size(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }

        return bytes;
    }

    /**
     * 将字节数组转换为十六进制字符串
     * @param bytes 字节数组
     * @return 十六进制字符串
     */
    static auto bytes_to_hex(const std::vector<unsigned char>& bytes) -> std::string {
        static constexpr char hex_digits[] = "0123456789abcdef";

        std::string hex;
        hex.reserve(bytes.size() * 2);

        for (const unsigned char byte : bytes) {
            hex.push_back(hex_digits[byte >> 4]);
            hex.push_back(hex_digits[byte & 0xF]);
        }

        return hex;
    }

    /**
     * TEA加密
     * @param data 明文数据(必须是8字节的倍数)
     * @param key 密钥(16字节)
     * @return 密文
     */
    static auto tea_encrypt(const std::vector<unsigned char>& data,
                            const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("TEA key must be 16 bytes");
        }
        if (data.size() % 8 != 0) {
            throw std::runtime_error("TEA data size must be multiple of 8 bytes");
        }

        std::vector<unsigned char> ciphertext(data.size());
        const auto k = reinterpret_cast<const uint32_t*>(key.data());

        for (size_t i = 0; i < data.size(); i += 8) {
            uint32_t v0 = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
            uint32_t v1 = (data[i + 4] << 24) | (data[i + 5] << 16) | (data[i + 6] << 8) | data[i + 7];
            uint32_t sum = 0;

            for (int j = 0; j < 32; j++) {
                constexpr uint32_t delta = 0x9e3779b9;
                sum += delta;
                v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
                v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
            }

            ciphertext[i] = (v0 >> 24) & 0xFF;
            ciphertext[i + 1] = (v0 >> 16) & 0xFF;
            ciphertext[i + 2] = (v0 >> 8) & 0xFF;
            ciphertext[i + 3] = v0 & 0xFF;
            ciphertext[i + 4] = (v1 >> 24) & 0xFF;
            ciphertext[i + 5] = (v1 >> 16) & 0xFF;
            ciphertext[i + 6] = (v1 >> 8) & 0xFF;
            ciphertext[i + 7] = v1 & 0xFF;
        }

        return ciphertext;
    }

    /**
     * TEA解密
     * @param data 密文数据(必须是8字节的倍数)
     * @param key 密钥(16字节)
     * @return 明文
     */
    static auto tea_decrypt(const std::vector<unsigned char>& data,
                            const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("TEA key must be 16 bytes");
        }
        if (data.size() % 8 != 0) {
            throw std::runtime_error("TEA data size must be multiple of 8 bytes");
        }

        std::vector<unsigned char> plaintext(data.size());
        const auto k = reinterpret_cast<const uint32_t*>(key.data());

        for (size_t i = 0; i < data.size(); i += 8) {
            uint32_t v0 = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
            uint32_t v1 = (data[i + 4] << 24) | (data[i + 5] << 16) | (data[i + 6] << 8) | data[i + 7];
            uint32_t sum = 0xC6EF3720; // delta * 32

            for (int j = 0; j < 32; j++) {
                constexpr uint32_t delta = 0x9e3779b9;
                v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
                v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
                sum -= delta;
            }

            plaintext[i] = (v0 >> 24) & 0xFF;
            plaintext[i + 1] = (v0 >> 16) & 0xFF;
            plaintext[i + 2] = (v0 >> 8) & 0xFF;
            plaintext[i + 3] = v0 & 0xFF;
            plaintext[i + 4] = (v1 >> 24) & 0xFF;
            plaintext[i + 5] = (v1 >> 16) & 0xFF;
            plaintext[i + 6] = (v1 >> 8) & 0xFF;
            plaintext[i + 7] = v1 & 0xFF;
        }

        return plaintext;
    }

    /**
     * 计算CRC32校验值
     * @param data 输入数据
     * @return CRC32值
     */
    static auto crc32(const std::vector<unsigned char>& data) -> uint32_t {
        uint32_t crc = 0xFFFFFFFF;
        static constexpr uint32_t crc_table[256] = {
            0x00000000,
            0x77073096,
            0xEE0E612C,
            0x990951BA,
            0x076DC419,
            0x706AF48F,
            0xE963A535,
            0x9E6495A3,
            0x0EDB8832,
            0x79DCB8A4,
            0xE0D5E91E,
            0x97D2D988,
            0x09B64C2B,
            0x7EB17CBD,
            0xE7B82D07,
            0x90BF1D91,
            0x1DB71064,
            0x6AB020F2,
            0xF3B97148,
            0x84BE41DE,
            0x1ADAD47D,
            0x6DDDE4EB,
            0xF4D4B551,
            0x83D385C7,
            0x136C9856,
            0x646BA8C0,
            0xFD62F97A,
            0x8A65C9EC,
            0x14015C4F,
            0x63066CD9,
            0xFA0F3D63,
            0x8D080DF5,
            0x3B6E20C8,
            0x4C69105E,
            0xD56041E4,
            0xA2677172,
            0x3C03E4D1,
            0x4B04D447,
            0xD20D85FD,
            0xA50AB56B,
            0x35B5A8FA,
            0x42B2986C,
            0xDBBBC9D6,
            0xACBCF940,
            0x32D86CE3,
            0x45DF5C75,
            0xDCD60DCF,
            0xABD13D59,
            0x26D930AC,
            0x51DE003A,
            0xC8D75180,
            0xBFD06116,
            0x21B4F4B5,
            0x56B3C423,
            0xCFBA9599,
            0xB8BDA50F,
            0x2802B89E,
            0x5F058808,
            0xC60CD9B2,
            0xB10BE924,
            0x2F6F7C87,
            0x58684C11,
            0xC1611DAB,
            0xB6662D3D,
            0x76DC4190,
            0x01DB7106,
            0x98D220BC,
            0xEFD5102A,
            0x71B18589,
            0x06B6B51F,
            0x9FBFE4A5,
            0xE8B8D433,
            0x7807C9A2,
            0x0F00F934,
            0x9609A88E,
            0xE10E9818,
            0x7F6A0DBB,
            0x086D3D2D,
            0x91646C97,
            0xE6635C01,
            0x6B6B51F4,
            0x1C6C6162,
            0x856530D8,
            0xF262004E,
            0x6C0695ED,
            0x1B01A57B,
            0x8208F4C1,
            0xF50FC457,
            0x65B0D9C6,
            0x12B7E950,
            0x8BBEB8EA,
            0xFCB9887C,
            0x62DD1DDF,
            0x15DA2D49,
            0x8CD37CF3,
            0xFBD44C65,
            0x4DB26158,
            0x3AB551CE,
            0xA3BC0074,
            0xD4BB30E2,
            0x4ADFA541,
            0x3DD895D7,
            0xA4D1C46D,
            0xD3D6F4FB,
            0x4369E96A,
            0x346ED9FC,
            0xAD678846,
            0xDA60B8D0,
            0x44042D73,
            0x33031DE5,
            0xAA0A4C5F,
            0xDD0D7CC9,
            0x5005713C,
            0x270241AA,
            0xBE0B1010,
            0xC90C2086,
            0x5768B525,
            0x206F85B3,
            0xB966D409,
            0xCE61E49F,
            0x5EDEF90E,
            0x29D9C998,
            0xB0D09822,
            0xC7D7A8B4,
            0x59B33D17,
            0x2EB40D81,
            0xB7BD5C3B,
            0xC0BA6CAD,
            0xEDB88320,
            0x9ABFB3B6,
            0x03B6E20C,
            0x74B1D29A,
            0xEAD54739,
            0x9DD277AF,
            0x04DB2615,
            0x73DC1683,
            0xE3630B12,
            0x94643B84,
            0x0D6D6A3E,
            0x7A6A5AA8,
            0xE40ECF0B,
            0x9309FF9D,
            0x0A00AE27,
            0x7D079EB1,
            0xF00F9344,
            0x8708A3D2,
            0x1E01F268,
            0x6906C2FE,
            0xF762575D,
            0x806567CB,
            0x196C3671,
            0x6E6B06E7,
            0xFED41B76,
            0x89D32BE0,
            0x10DA7A5A,
            0x67DD4ACC,
            0xF9B9DF6F,
            0x8EBEEFF9,
            0x17B7BE43,
            0x60B08ED5,
            0xD6D6A3E8,
            0xA1D1937E,
            0x38D8C2C4,
            0x4FDFF252,
            0xD1BB67F1,
            0xA6BC5767,
            0x3FB506DD,
            0x48B2364B,
            0xD80D2BDA,
            0xAF0A1B4C,
            0x36034AF6,
            0x41047A60,
            0xDF60EFC3,
            0xA867DF55,
            0x316E8EEF,
            0x4669BE79,
            0xCB61B38C,
            0xBC66831A,
            0x256FD2A0,
            0x5268E236,
            0xCC0C7795,
            0xBB0B4703,
            0x220216B9,
            0x5505262F,
            0xC5BA3BBE,
            0xB2BD0B28,
            0x2BB45A92,
            0x5CB36A04,
            0xC2D7FFA7,
            0xB5D0CF31,
            0x2CD99E8B,
            0x5BDEAE1D,
            0x9B64C2B0,
            0xEC63F226,
            0x756AA39C,
            0x026D930A,
            0x9C0906A9,
            0xEB0E363F,
            0x72076785,
            0x05005713,
            0x95BF4A82,
            0xE2B87A14,
            0x7BB12BAE,
            0x0CB61B38,
            0x92D28E9B,
            0xE5D5BE0D,
            0x7CDCEFB7,
            0x0BDBDF21,
            0x86D3D2D4,
            0xF1D4E242,
            0x68DDB3F8,
            0x1FDA836E,
            0x81BE16CD,
            0xF6B9265B,
            0x6FB077E1,
            0x18B74777,
            0x88085AE6,
            0xFF0F6A70,
            0x66063BCA,
            0x11010B5C,
            0x8F659EFF,
            0xF862AE69,
            0x616BFFD3,
            0x166CCF45,
            0xA00AE278,
            0xD70DD2EE,
            0x4E048354,
            0x3903B3C2,
            0xA7672661,
            0xD06016F7,
            0x4969474D,
            0x3E6E77DB,
            0xAED16A4A,
            0xD9D65ADC,
            0x40DF0B66,
            0x37D83BF0,
            0xA9BCAE53,
            0xDEBB9EC5,
            0x47B2CF7F,
            0x30B5FFE9,
            0xBDBDF21C,
            0xCABAC28A,
            0x53B39330,
            0x24B4A3A6,
            0xBAD03605,
            0xCDD70693,
            0x54DE5729,
            0x23D967BF,
            0xB3667A2E,
            0xC4614AB8,
            0x5D681B02,
            0x2A6F2B94,
            0xB40BBE37,
            0xC30C8EA1,
            0x5A05DF1B,
            0x2D02EF8D
        };

        for (const unsigned char byte : data) {
            crc = (crc >> 8) ^ crc_table[(crc ^ byte) & 0xFF];
        }

        return crc ^ 0xFFFFFFFF;
    }

    /**
     * RC4加密/解密 (对称算法)
     * @param data 输入数据
     * @param key 密钥(1-256字节)
     * @return 处理后的数据
     */
    static auto rc4(const std::vector<unsigned char>& data,
                    const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.empty() || key.size() > 256) {
            throw std::runtime_error("RC4 key size must be between 1 and 256 bytes");
        }

        std::vector<unsigned char> s(256);
        for (int i = 0; i < 256; i++) {
            s[i] = i;
        }

        uint8_t j = 0;
        for (int i = 0; i < 256; i++) {
            j = j + s[i] + key[i % key.size()];
            std::swap(s[i], s[j]);
        }

        std::vector<unsigned char> result(data.size());
        uint8_t i = 0;
        j = 0;

        for (size_t k = 0; k < data.size(); k++) {
            i++;
            j = j + s[i];
            std::swap(s[i], s[j]);
            const uint8_t t = s[i] + s[j];
            result[k] = data[k] ^ s[t];
        }

        return result;
    }

    /**
     * SM4加密 (ECB模式)
     * @param plaintext 明文(必须是16字节的倍数)
     * @param key 密钥(16字节)
     * @return 密文
     */
    static auto sm4_encrypt(const std::vector<unsigned char>& plaintext,
                            const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("SM4 key must be 16 bytes");
        }
        if (plaintext.size() % 16 != 0) {
            throw std::runtime_error("SM4 plaintext size must be multiple of 16 bytes");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher = EVP_sm4_ecb();
        if (!cipher) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("SM4 not supported in this OpenSSL version");
        }

        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        std::vector<unsigned char> ciphertext(plaintext.size() + 16);
        int len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }

    /**
     * SM4解密 (ECB模式)
     * @param ciphertext 密文(必须是16字节的倍数)
     * @param key 密钥(16字节)
     * @return 明文
     */
    static auto sm4_decrypt(const std::vector<unsigned char>& ciphertext,
                            const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("SM4 key must be 16 bytes");
        }
        if (ciphertext.size() % 16 != 0) {
            throw std::runtime_error("SM4 ciphertext size must be multiple of 16 bytes");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher = EVP_sm4_ecb();
        if (!cipher) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("SM4 not supported in this OpenSSL version");
        }

        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key.data(), nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptInit_ex failed");
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptUpdate failed");
        }
        int plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptFinal_ex failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        plaintext.resize(plaintext_len);
        return plaintext;
    }

    /**
     * XOR加密/解密
     * @param data 输入数据
     * @param key 密钥
     * @return 处理后的数据
     */
    static auto XOR(std::vector<unsigned char>& data, const unsigned char key) -> void {
        for (unsigned char& i : data) {
            i ^= key;
        }
    }

    /**
     * SEED加密 (ECB模式)
     * @param plaintext 明文(必须是16字节的倍数)
     * @param key 密钥(16字节)
     * @return 密文
     */
    static auto seed_encrypt(const std::vector<unsigned char>& plaintext,
                             const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("SEED key must be 16 bytes");
        }
        if (plaintext.size() % 16 != 0) {
            throw std::runtime_error("SEED plaintext size must be multiple of 16 bytes");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher = EVP_seed_ecb();
        if (!cipher) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("SEED not supported in this OpenSSL version");
        }

        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        std::vector<unsigned char> ciphertext(plaintext.size() + 16);
        int len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }

    /**
     * SEED解密 (ECB模式)
     * @param ciphertext 密文(必须是16字节的倍数)
     * @param key 密钥(16字节)
     * @return 明文
     */
    static auto seed_decrypt(const std::vector<unsigned char>& ciphertext,
                             const std::vector<unsigned char>& key) -> std::vector<unsigned char> {
        if (key.size() != 16) {
            throw std::runtime_error("SEED key must be 16 bytes");
        }
        if (ciphertext.size() % 16 != 0) {
            throw std::runtime_error("SEED ciphertext size must be multiple of 16 bytes");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        const EVP_CIPHER* cipher = EVP_seed_ecb();
        if (!cipher) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("SEED not supported in this OpenSSL version");
        }

        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key.data(), nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptInit_ex failed");
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptUpdate failed");
        }
        int plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("EVP_DecryptFinal_ex failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        plaintext.resize(plaintext_len);
        return plaintext;
    }
};
