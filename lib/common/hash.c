
#include "hash.h"

/**
 * @note 计算字符串的hash
 * @todo 目标是30字节以内的字符串可以得到唯一的hash值，无需再次进行字符串比较
 * @todo 以下提供了三种HASH方法，尚未验证哪一种最适合本项目，验证工作需进行，可参考：
 *        http://www.cnblogs.com/uvsjoh/archive/2012/03/27/2420120.html
 *
 */

unsigned int Calc_binhash(unsigned char *buf, int len) {
#ifdef BDKHASH
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (len--)
	{
		hash = hash * seed + (*buf++);
	}

	return (hash & 0x7FFFFFFF);
#endif
}

unsigned int Calc_hash(char *str) {
#ifdef BDKHASH
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
#endif

#ifdef APHASH
	unsigned int hash = 0;
	int i;

	for (i=0; *str; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}

	return (hash & 0x7FFFFFFF);
#endif

#ifdef RSHASH
	unsigned int b = 378551;
	unsigned int a = 63689;
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * a + (*str++);
		a *= b;
	}

	return (hash & 0x7FFFFFFF);
#endif
}
