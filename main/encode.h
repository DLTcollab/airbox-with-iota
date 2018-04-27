#ifndef ENCODE_HACK4U
#define ENCODE_HACK4U

char table[100][22] = {  
                       "1110101010110111001",
                       "1110101010110001111",
                       "1110101010110101101",
                       "1110101010110111000",
                       "1110101010110110000",
                       "1110101010110001100",
                       "1110101010110011000",
                       "1110101010110111101",
                       "1110101010110100101",
                       "1110101010110100010",
                       "1110101010110010000",
                       "1110101010110010111",
                       "1110101010110011001",
                       "1110101010110101100",
                       "1110101010110010110",
                       "1110101010110111111",
                       "1110101010110111100",
                       "1110101010110011111",
                       "1110101010110101111",
                       "1110101010110110100",
                       "1110101010110110101",
                       "1110101010110110111",
                       "1110101010110010011",
                       "1110101010110101011",
                       "1110101010110001001",
                       "1110101010110100001",
                       "1110101010110100000",
                       "1110101010110100110",
                       "1110101010110000011",
                       "1110101010110000010",
                       "1110101010110010100",
                       "111010101011111110",
                       "1110101010110000000",
                       "1110101010111000000",
                       "111010101011111101",
                       "111010101011111100",
                       "111010101011111010",
                       "111010101011100111",
                       "111010101011100110",
                       "111010101011100100",
                       "111010101011100010",
                       "111010101011101",
                       "1110101010101",
                       "1110101011",
                       "111010100",
                       "1110100",
                       "1111",
                       "0101",
                       "110",
                       "001",
                       "10",
                       "000",
                       "011",
                       "0100",
                       "11100",
                       "111011",
                       "11101011",
                       "11101010100",
                       "1110101010100",
                       "111010101011110",
                       "111010101011100001",
                       "111010101011100011",
                       "111010101011100101",
                       "111010101011111000",
                       "111010101011111001",
                       "111010101011111011",
                       "1110101010110110010",
                       "1110101010110110011",
                       "1110101010111000001",
                       "1110101010110000001",
                       "111010101011111111",
                       "1110101010110010101",
                       "1110101010110011010",
                       "1110101010110011011",
                       "1110101010110100111",
                       "1110101010110001000",
                       "1110101010110101010",
                       "1110101010110010010",
                       "1110101010110101110",
                       "1110101010110001010",
                       "1110101010110110001",
                       "1110101010110011100",
                       "1110101010110100100",
                       "1110101010110010001",
                       "1110101010110000111",
                       "1110101010110011110",
                       "1110101010110111110",
                       "1110101010110100011",
                       "1110101010110110110",
                       "1110101010110000110",
                       "1110101010110101001",
                       "1110101010110000101",
                       "1110101010110001101",
                       "1110101010110011101",
                       "1110101010110101000",
                       "1110101010110111011",
                       "1110101010110001011",
                       "1110101010110111010",
                       "1110101010110000100",
                       "1110101010110001110"
                       };

const int lb = -50, ub = 49, range = 50, table_size = 22;
const int symbol_bits = 6;
const char symbol[70] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int counter = 0;
int epoch[10], s_d0[10], s_d1[10], s_t0[10], s_h0[10];
char code[1024];
int code_id, symbol_id, msg_id;
int diff;

int encode(int ep, int d0, int d1, int t0, int h0, char msg[])
{
  epoch[counter] = ep;
  s_d0[counter] = d0;
  s_d1[counter] = d1;
  s_t0[counter] = t0;
  s_h0[counter] = h0;
  if (counter) {
    diff = epoch[counter] - epoch[counter-1];
    diff = diff - lass_period;
    if (diff < lb || diff > ub) {
      counter = 0;
      return 0;  
    }
    diff = s_d0[counter] - s_d0[counter-1];
    if (diff < lb || diff > ub) {
      counter = 0;
      return 0;  
    }
    diff = s_d1[counter] - s_d1[counter - 1];
    if (diff < lb || diff > ub) {
      counter = 0;
      return 0;  
    }
    diff = s_t0[counter] - s_t0[counter - 1];
    if (diff < lb || diff > ub) {
      counter = 0;
      return 0;  
    }
    diff = s_h0[counter] - s_h0[counter - 1];
    if (diff < lb || diff > ub) {
      counter = 0;
      return 0;  
    }
  }
  counter++;
  if (counter >= 10) {
    counter = 0;
    code_id = 0;
    memset(code, '\0', sizeof(code));
    
    //epoch
    code[code_id++] = (epoch[0] & (1 << 31)) ? '1' : '0'; //sign bit
    for (int i = 1, bits = 32, mask = 1 << (bits - 2); i < bits; i++, mask >>= 1)
        code[code_id++] = (epoch[0] & mask) ? '1' : '0'; 
    for (int i = 1; i < 10; i++) {
        diff = epoch[i] - epoch[i - 1];
        diff = diff - lass_period;
        for (int j = 0; j < table_size && table[diff + range][j] != '\0'; j++)
            code[code_id++] = (table[diff + range][j] == '1') ? '1' : '0';
    }
    
    //pm25
    for (int i = 0, bits = 16, mask = 1 << (bits - 1); i < bits; i++, mask >>= 1)
        code[code_id++] = (s_d0[0] & mask) ? '1' : '0';     
    for (int i = 1; i < 10; i++) {
        diff = s_d0[i] - s_d0[i - 1];
        for (int j = 0; j < table_size && table[diff + range][j] != '\0'; j++)
            code[code_id++] = (table[diff + range][j] == '1') ? '1' : '0';
    }

    //pm10
    for (int i = 0, bits = 16, mask = 1 << (bits - 1); i < bits; i++, mask >>= 1)
        code[code_id++] = (s_d1[0] & mask) ? '1' : '0';    
    for (int i = 1; i < 10; i++) {
        diff = s_d1[i] - s_d1[i - 1];
        for (int j = 0; j < table_size && table[diff + range][j] != '\0'; j++)
            code[code_id++] = (table[diff + range][j] == '1') ? '1' : '0';
    }

    //bme280_t
    code[code_id++] = (s_t0[0] & (1 << 31)) ? '1' : '0'; //sign bit
    for (int i = 1, bits = 8, mask = 1 << (bits - 2); i < bits; i++, mask >>= 1)
        code[code_id++] = (s_t0[0] & mask) ? '1' : '0';  
    for (int i = 1; i < 10; i++) {
        diff = s_t0[i] - s_t0[i - 1];
        for (int j = 0; j < table_size && table[diff + range][j] != '\0'; j++)
            code[code_id++] = (table[diff + range][j] == '1') ? '1' : '0';
    }

    //bme280_h
    for (int i = 0, bits = 8, mask = 1 << (bits - 1); i < bits; i++, mask >>= 1)
        code[code_id++] = (s_h0[0] & mask) ? '1' : '0';     
    for (int i = 1; i < 10; i++) {
        diff = s_h0[i] - s_h0[i - 1];
        for (int j = 0; j < table_size && table[diff + range][j] != '\0'; j++)
            code[code_id++] = (table[diff + range][j] == '1') ? '1' : '0';
    }
    //Serial.println(code);
    //generate msg
    msg_id = 0;
    for (int i = 0; i < code_id; i += symbol_bits) {
        symbol_id = 0;
        for (int j = i; j < i + symbol_bits; j++) {
            symbol_id <<= 1;
            symbol_id += (code[j] == '1') ? 1: 0;
        }
        msg[msg_id++] = symbol[symbol_id];
    }
    return 1;
  }
  return 0;  
}

#endif
