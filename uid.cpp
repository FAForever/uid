#include <string>
#include <iostream>
#include <cassert>
#include <stdint.h>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#include <cryptopp/base64.h>
#include <cryptopp/integer.h>
#include <cryptopp/files.h>

#include <json/json.h>

#include "machine_info.h"

#if !defined(UID_PUBKEY_BYTES)
#  error "You must #define UID_PUBKEY_BYTES as 28 sized byte list, eg. '130,99,238,192,232,47,187,99,222,116,140,101,233,231,57,188,204,204,187,241,173,147,88,60,217,7,80,217'"
#endif

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "failure: expected the session string as first argument" << std::endl;
    return 1;
  }
  Json::Value root;
  root["session"] = argv[1];

  if (!machine_info_init())
  {
    std::cerr << "Error initialising machine_info" << std::endl;
    return 1;
  }

  root["machine"]["uuid"] = machine_info_uuid();
  root["machine"]["model"] = machine_info_model();
  root["machine"]["manufacturer"] = machine_info_manufacturer();
  root["machine"]["display"]["width"] = machine_info_display_width();
  root["machine"]["display"]["height"] = machine_info_display_height();
  root["machine"]["memory"]["serial0"] = machine_info_memory_serial0();
  root["machine"]["motherboard"]["vendor"] = machine_info_motherboard_vendor();
  root["machine"]["motherboard"]["name"] = machine_info_motherboard_name();
  root["machine"]["disks"]["controller_id"] = machine_info_disks_controllerid();
  root["machine"]["disks"]["vserial"] = machine_info_disks_vserial();
  root["machine"]["bios"]["manufacturer"] = machine_info_bios_manufacturer();
  root["machine"]["bios"]["smbbversion"] = machine_info_bios_smbbversion();
  root["machine"]["bios"]["serial"] = machine_info_bios_serial();
  root["machine"]["bios"]["description"] = machine_info_bios_description();
  root["machine"]["bios"]["date"] = machine_info_bios_date();
  root["machine"]["bios"]["version"] = machine_info_bios_version();
  root["machine"]["processor"]["name"] = machine_info_processor_name();
  root["machine"]["processor"]["id"] = machine_info_processor_id();
  root["machine"]["os"]["type"] = machine_info_os_type();
  root["machine"]["os"]["version"] = machine_info_os_version();

  machine_info_free();

  //std::cout << root << std::endl;
  std::string json_string = std::string("2") + Json::FastWriter().write(root);
  //std::cout << json_string << "<-END" << std::endl;

  try
  {
    CryptoPP::AutoSeededRandomPool rng;

#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16

    /* Generate random AES Key and Initialisation vector */
    CryptoPP::SecByteBlock aes_key( AES_KEY_SIZE );
    CryptoPP::SecByteBlock iv( AES_BLOCK_SIZE );

    rng.GenerateBlock(aes_key, AES_KEY_SIZE);
    rng.GenerateBlock(iv, AES_BLOCK_SIZE);

    /* Base64 encode IV */
    std::string iv_b64;
    {
      CryptoPP::StringSource s(iv, AES_BLOCK_SIZE, true,
          new CryptoPP::Base64Encoder(
              new CryptoPP::StringSink(iv_b64), false
          )
      );
    }
    /* the server expects 24 bytes */
    assert(iv_b64.size() == 24);

    /* create the RSA Public key from the UID_PUBKEY_BYTES #define */
    uint8_t pubkey_bytes[28] = {UID_PUBKEY_BYTES};
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::Integer modulus;
    modulus.Decode(pubkey_bytes, 28);
    publicKey.Initialize(modulus, 65537);

    /* encrypt the AES key and encode the encryptes key to base64 */
    std::string aes_key_encrypted_base64;
    {
      CryptoPP::RSAES_PKCS1v15_Encryptor e( publicKey );
      CryptoPP::StringSource s( aes_key, AES_KEY_SIZE, true,
          new CryptoPP::PK_EncryptorFilter(rng, e,
            new CryptoPP::Base64Encoder(
              new CryptoPP::StringSink(aes_key_encrypted_base64), false /*insertLineBreaks*/
            )
          )
      );
    }
    /* the server expects 40 bytes */
    assert(aes_key_encrypted_base64.size() == 40);

    /* now encrypt the JSON string with AES */
    std::string json_string_encrypted_b64;
    {
      CryptoPP::CBC_Mode< CryptoPP::AES >::Encryption e;
      e.SetKeyWithIV( aes_key, AES_KEY_SIZE, iv );
      CryptoPP::StringSource s( json_string, true,
        new CryptoPP::StreamTransformationFilter( e,
          new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink( json_string_encrypted_b64 ), false /*insertLineBreaks*/
          )
        )
      );
    }

    /* the number of padding bytes (AES can only encrypt a multiple of 16 bytes) */
    unsigned char paddingSize = 16 - (json_string.size() % 16);
    if (paddingSize == 16)
    {
      paddingSize = 0;
    }

    {
      /* the final bytearray consists of the
         1 byte paddingSize
         24 bytes IV (base64)
         X bytes encrypted JSON (base64)
         40 bytes encrypted AES key (base64) */
      CryptoPP::Base64Encoder b(
              new CryptoPP::FileSink(std::cout), false /*insertLineBreaks*/
          );
      b.Put(paddingSize);
      b.Put(reinterpret_cast<const byte*>(iv_b64.c_str()), iv_b64.size());
      b.Put(reinterpret_cast<const byte*>(json_string_encrypted_b64.c_str()), json_string_encrypted_b64.size());
      b.Put(reinterpret_cast<const byte*>(aes_key_encrypted_base64.c_str()), aes_key_encrypted_base64.size());
      b.MessageEnd();
    }
  }
  catch(std::exception& e)
  {
    std::cerr << "caught crypto exception of type " << typeid(e).name() << std::endl;
    std::cerr << "what: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
