/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Wed Mar 17 17:37:24 2021
 * by pycrc v0.9.2, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x011b
 *  - XorIn         = 0x0000
 *  - ReflectIn     = False
 *  - XorOut        = 0x0000
 *  - ReflectOut    = False
 *  - Algorithm     = table-driven
 */

#include "../../App/Inc/CRC.hh"

/**
 * Static table used for the table_driven implementation.
 */
static const crc_t crc_table[16] = {0x0000,
                                    0x011b,
                                    0x0236,
                                    0x032d,
                                    0x046c,
                                    0x0577,
                                    0x065a,
                                    0x0741,
                                    0x08d8,
                                    0x09c3,
                                    0x0aee,
                                    0x0bf5,
                                    0x0cb4,
                                    0x0daf,
                                    0x0e82,
                                    0x0f99};

crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
  const unsigned char *d = (const unsigned char *)data;
  unsigned int         tbl_idx;

  while (data_len--)
  {
    tbl_idx = (crc >> 12) ^ (*d >> 4);
    crc     = crc_table[tbl_idx & 0x0f] ^ (crc << 4);
    tbl_idx = (crc >> 12) ^ (*d >> 0);
    crc     = crc_table[tbl_idx & 0x0f] ^ (crc << 4);
    d++;
  }
  return crc & 0xffff;
}
