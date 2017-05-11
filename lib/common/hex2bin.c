

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
