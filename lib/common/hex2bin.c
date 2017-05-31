

int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;

  return -1;
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
int hex2bin(const char* src, char* target)
{
  while(*src && src[1])
  {
    int i = char2int(src[1]);
    int j = char2int(*src);

    if(i == -1 || j == -1)
        return -1;

    *(target++) = j*16 + i;
    src += 2;
  }

  return 0;
}

void bin2hex(const char* src, int length, char* target)
{
    int i = 0;
    while(i++ < length) {
        
        unsigned char lb = src[i] & 0x0f;
        unsigned char hb = (src[i] & 0xf0) >> 4;

        target[2*i] = (hb>=10) ? (hb - 10) + 'a': (hb + '0');
        target[2*i + 1] = (lb>=10) ? (lb - 10) + 'a': (lb + '0');
    }
}

