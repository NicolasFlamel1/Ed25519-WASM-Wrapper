// Header files
#include <emscripten.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "crypto_sign.h"

using namespace std;


// Function prototypes

// Public key size
extern "C" size_t EMSCRIPTEN_KEEPALIVE publicKeySize();

// Public key from secret key
extern "C" bool EMSCRIPTEN_KEEPALIVE publicKeyFromSecretKey(uint8_t *publicKey, const uint8_t *secretKey, size_t secretKeySize);

// Signature size
extern "C" size_t EMSCRIPTEN_KEEPALIVE signatureSize();

// Sign
extern "C" bool EMSCRIPTEN_KEEPALIVE sign(uint8_t *signature, const uint8_t *message, size_t messageSize, const uint8_t *secretKey, size_t secretKeySize);

// Verify
extern "C" bool EMSCRIPTEN_KEEPALIVE verify(const uint8_t *message, size_t messageSize, const uint8_t *signature, size_t signatureSize, const uint8_t *publicKey, size_t publicKeySize);


// Supporting function implementation

// Public key size
size_t publicKeySize() {

	// Return public key size
	return crypto_sign_PUBLICKEYBYTES;
}

// Public key from secret key
bool publicKeyFromSecretKey(uint8_t *publicKey, const uint8_t *secretKey, size_t secretKeySize) {

	// Check if secret key is invalid
	if(secretKeySize != crypto_sign_SECRETKEYBYTES - crypto_sign_PUBLICKEYBYTES) {
	
		// Return false
		return false;
	}

	// Check if getting the public key failed
	uint8_t fullSecretKey[crypto_sign_SECRETKEYBYTES];
	memcpy(fullSecretKey, secretKey, crypto_sign_SECRETKEYBYTES - crypto_sign_PUBLICKEYBYTES);
	
	if(crypto_sign_keypair(publicKey, fullSecretKey)) {
	
		// Clear memory
		explicit_bzero(fullSecretKey, sizeof(fullSecretKey));
	
		// Return false
		return false;
	}
	
	// Clear memory
	explicit_bzero(fullSecretKey, sizeof(fullSecretKey));
	
	// Return true
	return true;
}

// Signature size
size_t signatureSize() {

	// Return signature size
	return crypto_sign_BYTES;
}


// Sign
bool sign(uint8_t *signature, const uint8_t *message, size_t messageSize, const uint8_t *secretKey, size_t secretKeySize) {

	// Check if secret key is invalid
	if(secretKeySize != crypto_sign_SECRETKEYBYTES - crypto_sign_PUBLICKEYBYTES) {
	
		// Return false
		return false;
	}

	// Check if getting the full secret key failed
	uint8_t publicKey[crypto_sign_PUBLICKEYBYTES];
	uint8_t fullSecretKey[crypto_sign_SECRETKEYBYTES];
	memcpy(fullSecretKey, secretKey, crypto_sign_SECRETKEYBYTES - crypto_sign_PUBLICKEYBYTES);
	
	if(crypto_sign_keypair(publicKey, fullSecretKey)) {
	
		// Clear memory
		explicit_bzero(fullSecretKey, sizeof(fullSecretKey));
	
		// Return false
		return false;
	}
	
	// Check if signing the message failed
	uint8_t fullSignature[crypto_sign_BYTES + messageSize];
	uint64_t signatureLength;
	
	if(crypto_sign(fullSignature, &signatureLength, message, messageSize, fullSecretKey)) {
	
		// Clear memory
		explicit_bzero(fullSecretKey, sizeof(fullSecretKey));
	
		// Return false
		return false;
	}
	
	// Copy full signature to the signature
	memcpy(signature, fullSignature, crypto_sign_BYTES);
	
	// Clear memory
	explicit_bzero(fullSecretKey, sizeof(fullSecretKey));
	
	// Return true
	return true;
}

// Verify
bool verify(const uint8_t *message, size_t messageSize, const uint8_t *signature, size_t signatureSize, const uint8_t *publicKey, size_t publicKeySize) {

	// Check if signature is invalid
	if(signatureSize != crypto_sign_BYTES) {
	
		// Return false
		return false;
	}
	
	// Check if public key is invalid
	if(publicKeySize != crypto_sign_PUBLICKEYBYTES) {
	
		// Return false
		return false;
	}

	// Get the full signature
	uint8_t fullSignature[crypto_sign_BYTES + messageSize];
	memcpy(fullSignature, signature, crypto_sign_BYTES);
	memcpy(&fullSignature[crypto_sign_BYTES], message, messageSize);
	
	// Check if verifying the message failed
	uint8_t verifiedMessage[messageSize];
	uint64_t verifiedMessageLength;
	
	if(crypto_sign_open(verifiedMessage, &verifiedMessageLength, fullSignature, sizeof(fullSignature), publicKey)) {
	
		// Return false
		return false;
	}
	
	// Return true
	return true;
}
