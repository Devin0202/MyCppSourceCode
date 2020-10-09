#include <iostream>
#include <fstream>

#include "aes.hpp"

int main(int argc, char **argv) {
  uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09,
                    0xcf, 0x4f, 0x3c};

  uint8_t iv[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
                  0x0d, 0x0e, 0x0f};

  /** Length of "model_type" string with byte. */
  uint32_t model_type_len = 9;
  char *model_type = "face_pose";

  std::ifstream infile;
  std::filebuf *pbuf;

  infile.open("../data/M_BILIBILI_FacePose_bak.model");
  if (!infile.is_open()) {
    std::cout << "Path to input-file is invalid!" << std::endl;
    return -1;
  }

  pbuf = infile.rdbuf();
  /** Length of file data string with byte. */
  uint64_t model_binary_len = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
  pbuf->pubseekpos(0, std::ios::in);
  char *p = new char[model_binary_len];
  pbuf->sgetn(p, model_binary_len);
  infile.close();

  // std::cout << sizeof(model_type_len) << std::endl;
  // std::cout << sizeof(model_binary_len) << std::endl;

  /** Add extra one for safe character '\0'. */
  int total_size = sizeof(model_type_len) + model_type_len + sizeof(model_binary_len) + model_binary_len + 1;
  char *pp = new char[total_size];
  memset(pp, 0, total_size);

  char *pp_var = pp;
  memcpy(pp_var, &model_type_len, sizeof(model_type_len));
  pp_var += sizeof(model_type_len);
  memcpy(pp_var, model_type, model_type_len);
  pp_var += model_type_len;
  memcpy(pp_var, &model_binary_len, sizeof(model_binary_len));
  pp_var += sizeof(model_binary_len);
  memcpy(pp_var, p, model_binary_len);

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  /** Length of string for encrypting with aes-128 should be divisible by 16. */
  AES_CBC_encrypt_buffer(&ctx, (uint8_t *)pp, total_size / 16 * 16);
  std::cout << "CBC encrypt: " << std::endl;

  std::ofstream outfile;
  outfile.open("../results/file.model", std::ios::out);
  if (!outfile.is_open()) {
    std::cout << "Path to output-file is invalid!" << std::endl;
    return -1;
  }
  outfile.write(pp, total_size);
  outfile.close();

  // AES_CBC_decrypt_buffer(&ctx, (uint8_t *)pp, total_size / 16 * 16);

  pp_var = nullptr;
  delete[] p;
  delete[] pp;
  p = nullptr;
  pp = nullptr;
  return 0;
}
