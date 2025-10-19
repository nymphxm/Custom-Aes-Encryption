#include <cstdint>
#include <vector>
#include <cstring>
#include <memory>

class c_aes
{
private:
    static constexpr int I_NB = 4;
    static constexpr int I_NK = 8;
    static constexpr int I_NR = 14;
    static constexpr int I_BLOCK_SIZE = 16;
    static constexpr int I_KEY_SIZE = 32;

    static const uint8_t auc_sbox[256];
    static const uint8_t auc_inv_sbox[256];
    static const uint8_t auc_rcon[11];

    static uint8_t uc_gmul(uint8_t uc_a, uint8_t uc_b)
    {
        uint8_t uc_p = 0;
        for (int i = 0; i < 8; i++)
        {
            if (uc_b & 1)
                uc_p ^= uc_a;

            bool b_hi_bit_set = (uc_a & 0x80) != 0;
            uc_a <<= 1;
            if (b_hi_bit_set)
                uc_a ^= 0x1B;

            uc_b >>= 1;
        }
        return uc_p;
    }

    static void key_expansion(const uint8_t* puc_key, uint8_t* puc_round_key)
    {
        uint8_t auc_temp[4];
        int i = 0;

        while (i < I_NK)
        {
            puc_round_key[i * 4 + 0] = puc_key[i * 4 + 0];
            puc_round_key[i * 4 + 1] = puc_key[i * 4 + 1];
            puc_round_key[i * 4 + 2] = puc_key[i * 4 + 2];
            puc_round_key[i * 4 + 3] = puc_key[i * 4 + 3];
            i++;
        }

        i = I_NK;
        while (i < I_NB * (I_NR + 1))
        {
            auc_temp[0] = puc_round_key[(i - 1) * 4 + 0];
            auc_temp[1] = puc_round_key[(i - 1) * 4 + 1];
            auc_temp[2] = puc_round_key[(i - 1) * 4 + 2];
            auc_temp[3] = puc_round_key[(i - 1) * 4 + 3];

            if (i % I_NK == 0)
            {
                uint8_t uc_k = auc_temp[0];
                auc_temp[0] = auc_sbox[auc_temp[1]] ^ auc_rcon[i / I_NK];
                auc_temp[1] = auc_sbox[auc_temp[2]];
                auc_temp[2] = auc_sbox[auc_temp[3]];
                auc_temp[3] = auc_sbox[uc_k];
            }
            else if (i % I_NK == 4)
            {
                auc_temp[0] = auc_sbox[auc_temp[0]];
                auc_temp[1] = auc_sbox[auc_temp[1]];
                auc_temp[2] = auc_sbox[auc_temp[2]];
                auc_temp[3] = auc_sbox[auc_temp[3]];
            }

            puc_round_key[i * 4 + 0] = puc_round_key[(i - I_NK) * 4 + 0] ^ auc_temp[0];
            puc_round_key[i * 4 + 1] = puc_round_key[(i - I_NK) * 4 + 1] ^ auc_temp[1];
            puc_round_key[i * 4 + 2] = puc_round_key[(i - I_NK) * 4 + 2] ^ auc_temp[2];
            puc_round_key[i * 4 + 3] = puc_round_key[(i - I_NK) * 4 + 3] ^ auc_temp[3];
            i++;
        }
    }

    static void add_round_key(int i_round, uint8_t auc_state[4][4], const uint8_t* puc_round_key)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                auc_state[i][j] ^= puc_round_key[i_round * I_NB * 4 + i * I_NB + j];
            }
        }
    }

    static void sub_bytes(uint8_t auc_state[4][4])
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                auc_state[i][j] = auc_sbox[auc_state[i][j]];
            }
        }
    }

    static void shift_rows(uint8_t auc_state[4][4])
    {
        uint8_t uc_temp;

        uc_temp = auc_state[1][0];
        auc_state[1][0] = auc_state[1][1];
        auc_state[1][1] = auc_state[1][2];
        auc_state[1][2] = auc_state[1][3];
        auc_state[1][3] = uc_temp;

        uc_temp = auc_state[2][0];
        auc_state[2][0] = auc_state[2][2];
        auc_state[2][2] = uc_temp;
        uc_temp = auc_state[2][1];
        auc_state[2][1] = auc_state[2][3];
        auc_state[2][3] = uc_temp;

        uc_temp = auc_state[3][3];
        auc_state[3][3] = auc_state[3][2];
        auc_state[3][2] = auc_state[3][1];
        auc_state[3][1] = auc_state[3][0];
        auc_state[3][0] = uc_temp;
    }

    static void mix_columns(uint8_t auc_state[4][4])
    {
        uint8_t auc_temp[4];
        for (int i = 0; i < 4; i++)
        {
            auc_temp[0] = auc_state[0][i];
            auc_temp[1] = auc_state[1][i];
            auc_temp[2] = auc_state[2][i];
            auc_temp[3] = auc_state[3][i];

            auc_state[0][i] = uc_gmul(auc_temp[0], 2) ^ uc_gmul(auc_temp[1], 3) ^ auc_temp[2] ^ auc_temp[3];
            auc_state[1][i] = auc_temp[0] ^ uc_gmul(auc_temp[1], 2) ^ uc_gmul(auc_temp[2], 3) ^ auc_temp[3];
            auc_state[2][i] = auc_temp[0] ^ auc_temp[1] ^ uc_gmul(auc_temp[2], 2) ^ uc_gmul(auc_temp[3], 3);
            auc_state[3][i] = uc_gmul(auc_temp[0], 3) ^ auc_temp[1] ^ auc_temp[2] ^ uc_gmul(auc_temp[3], 2);
        }
    }

    static void inv_sub_bytes(uint8_t auc_state[4][4])
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                auc_state[i][j] = auc_inv_sbox[auc_state[i][j]];
            }
        }
    }

    static void inv_shift_rows(uint8_t auc_state[4][4])
    {
        uint8_t uc_temp;

        uc_temp = auc_state[1][3];
        auc_state[1][3] = auc_state[1][2];
        auc_state[1][2] = auc_state[1][1];
        auc_state[1][1] = auc_state[1][0];
        auc_state[1][0] = uc_temp;

        uc_temp = auc_state[2][0];
        auc_state[2][0] = auc_state[2][2];
        auc_state[2][2] = uc_temp;
        uc_temp = auc_state[2][1];
        auc_state[2][1] = auc_state[2][3];
        auc_state[2][3] = uc_temp;

        uc_temp = auc_state[3][0];
        auc_state[3][0] = auc_state[3][1];
        auc_state[3][1] = auc_state[3][2];
        auc_state[3][2] = auc_state[3][3];
        auc_state[3][3] = uc_temp;
    }

    static void inv_mix_columns(uint8_t auc_state[4][4])
    {
        uint8_t auc_temp[4];
        for (int i = 0; i < 4; i++)
        {
            auc_temp[0] = auc_state[0][i];
            auc_temp[1] = auc_state[1][i];
            auc_temp[2] = auc_state[2][i];
            auc_temp[3] = auc_state[3][i];

            auc_state[0][i] = uc_gmul(auc_temp[0], 14) ^ uc_gmul(auc_temp[1], 11) ^ uc_gmul(auc_temp[2], 13) ^ uc_gmul(auc_temp[3], 9);
            auc_state[1][i] = uc_gmul(auc_temp[0], 9) ^ uc_gmul(auc_temp[1], 14) ^ uc_gmul(auc_temp[2], 11) ^ uc_gmul(auc_temp[3], 13);
            auc_state[2][i] = uc_gmul(auc_temp[0], 13) ^ uc_gmul(auc_temp[1], 9) ^ uc_gmul(auc_temp[2], 14) ^ uc_gmul(auc_temp[3], 11);
            auc_state[3][i] = uc_gmul(auc_temp[0], 11) ^ uc_gmul(auc_temp[1], 13) ^ uc_gmul(auc_temp[2], 9) ^ uc_gmul(auc_temp[3], 14);
        }
    }

    static void cipher_block(const uint8_t* puc_input, uint8_t* puc_output, const uint8_t* puc_round_key)
    {
        uint8_t auc_state[4][4];

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                auc_state[i][j] = puc_input[i + 4 * j];
            }
        }

        add_round_key(0, auc_state, puc_round_key);

        for (int i_round = 1; i_round < I_NR; i_round++)
        {
            sub_bytes(auc_state);
            shift_rows(auc_state);
            mix_columns(auc_state);
            add_round_key(i_round, auc_state, puc_round_key);
        }

        sub_bytes(auc_state);
        shift_rows(auc_state);
        add_round_key(I_NR, auc_state, puc_round_key);

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                puc_output[i + 4 * j] = auc_state[i][j];
            }
        }
    }

    static void inv_cipher_block(const uint8_t* puc_input, uint8_t* puc_output, const uint8_t* puc_round_key)
    {
        uint8_t auc_state[4][4];

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                auc_state[i][j] = puc_input[i + 4 * j];
            }
        }

        add_round_key(I_NR, auc_state, puc_round_key);

        for (int i_round = I_NR - 1; i_round > 0; i_round--)
        {
            inv_shift_rows(auc_state);
            inv_sub_bytes(auc_state);
            add_round_key(i_round, auc_state, puc_round_key);
            inv_mix_columns(auc_state);
        }

        inv_shift_rows(auc_state);
        inv_sub_bytes(auc_state);
        add_round_key(0, auc_state, puc_round_key);

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                puc_output[i + 4 * j] = auc_state[i][j];
            }
        }
    }

public:
    inline std::vector<uint8_t> encrypt(const std::vector<uint8_t>& vec_data, const std::vector<uint8_t>& vec_key)
    {
        if (vec_key.size() != I_KEY_SIZE)
        {
            return std::vector<uint8_t>();
        }

        uint8_t auc_round_key[240];
        key_expansion(vec_key.data(), auc_round_key);

        uint8_t auc_iv[I_BLOCK_SIZE] = { 0 };

        size_t sz_padded_len = vec_data.size();
        size_t sz_padding = I_BLOCK_SIZE - (vec_data.size() % I_BLOCK_SIZE);
        if (sz_padding != I_BLOCK_SIZE)
        {
            sz_padded_len += sz_padding;
        }
        else
        {
            sz_padded_len += I_BLOCK_SIZE;
            sz_padding = I_BLOCK_SIZE;
        }

        std::vector<uint8_t> vec_padded(sz_padded_len);
        std::memcpy(vec_padded.data(), vec_data.data(), vec_data.size());

        for (size_t i = vec_data.size(); i < sz_padded_len; i++)
        {
            vec_padded[i] = static_cast<uint8_t>(sz_padding);
        }

        std::vector<uint8_t> vec_encrypted(sz_padded_len);
        uint8_t auc_prev_block[I_BLOCK_SIZE];
        std::memcpy(auc_prev_block, auc_iv, I_BLOCK_SIZE);

        for (size_t i = 0; i < sz_padded_len; i += I_BLOCK_SIZE)
        {
            uint8_t auc_block[I_BLOCK_SIZE];
            for (int j = 0; j < I_BLOCK_SIZE; j++)
            {
                auc_block[j] = vec_padded[i + j] ^ auc_prev_block[j];
            }

            cipher_block(auc_block, &vec_encrypted[i], auc_round_key);
            std::memcpy(auc_prev_block, &vec_encrypted[i], I_BLOCK_SIZE);
        }

        return vec_encrypted;
    }

    inline std::vector<uint8_t> decrypt(const std::vector<uint8_t>& vec_data, const std::vector<uint8_t>& vec_key)
    {
        if (vec_key.size() != I_KEY_SIZE || vec_data.size() % I_BLOCK_SIZE != 0)
        {
            return std::vector<uint8_t>();
        }

        uint8_t auc_round_key[240];
        key_expansion(vec_key.data(), auc_round_key);

        uint8_t auc_iv[I_BLOCK_SIZE] = { 0 };

        std::vector<uint8_t> vec_decrypted(vec_data.size());
        uint8_t auc_prev_block[I_BLOCK_SIZE];
        std::memcpy(auc_prev_block, auc_iv, I_BLOCK_SIZE);

        for (size_t i = 0; i < vec_data.size(); i += I_BLOCK_SIZE)
        {
            uint8_t auc_block[I_BLOCK_SIZE];
            inv_cipher_block(&vec_data[i], auc_block, auc_round_key);

            for (int j = 0; j < I_BLOCK_SIZE; j++)
            {
                vec_decrypted[i + j] = auc_block[j] ^ auc_prev_block[j];
            }

            std::memcpy(auc_prev_block, &vec_data[i], I_BLOCK_SIZE);
        }

        if (!vec_decrypted.empty())
        {
            uint8_t uc_padding = vec_decrypted.back();
            if (uc_padding > 0 && uc_padding <= I_BLOCK_SIZE)
            {
                bool b_valid_padding = true;
                for (size_t i = vec_decrypted.size() - uc_padding; i < vec_decrypted.size(); i++)
                {
                    if (vec_decrypted[i] != uc_padding)
                    {
                        b_valid_padding = false;
                        break;
                    }
                }

                if (b_valid_padding)
                {
                    vec_decrypted.resize(vec_decrypted.size() - uc_padding);
                }
            }
        }

        return vec_decrypted;
    }
};

inline const uint8_t c_aes::auc_sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

inline const uint8_t c_aes::auc_inv_sbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

inline const uint8_t c_aes::auc_rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};


inline std::unique_ptr<c_aes> g_aes = std::make_unique<c_aes>();
