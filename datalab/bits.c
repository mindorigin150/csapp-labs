/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y)
{
  return (~(x & y)) & (~((~x) & (~y)));
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void)
{
  return (1 << 31);
}
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x)
{
  // Tmax: ~(1 << 31)
  // A ^ B 只要有位置上不同，结果的某些位上就会有1
  // ! 表示把这个数看作一个整体，如果不为0就是1
  // 1 << 31 表示 Tmax
  // IMPORTANT: 注意这里不准用移位
  // 利用 Tmax + 1 取反等于自身
  // 但是 +1 取反等于自身的还有 -1, 通过判断 (x+1) 后有没有1来排除
  return !(~(x + 1) ^ x) & !!(x + 1);
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x)
{
  // 0xAAAAAAAA 表示所有 odd 位置上都是1的数
  // 只要 x != 0xAAAAAAAA，那么 x ^ 0xAAAAAAAA 结果就非0
  // 注意题目只要求 odd numbers ，而对 even numbers 没有要求，所以先用 x & 0xAAAAAAAA 只保留这些位置上的
  // 注意不能直接使用超过 255 位的常量！
  int mask = 0xAA;
  mask = mask | (mask << 8);  // mask = 0xAAAA
  mask = mask | (mask << 16); // mask = 0xAAAAAAAA
  return !((x & mask) ^ mask);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x)
{
  // -x = x按位取反 + 1
  return ~x + 1;
}
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x)
{
  // acsii digit: 只有最低8位表示数字，高位都是0
  // 涉及到跟某个数比较：移位变成跟0比 -> 相减变成取反+1，跟0比较利用移位比较最高位符号位
  // 翻译一下就是满足三个条件：
  // 1. x >= 0x30 -> x - 0x30 >= 0 -> (x+~0x30+1) >= 0 -> ((x+~0x30+1) >> 31) == 0
  // 2. x <= 0x39 -> ((x+~0x3A+1) >> 31) == 1
  // 3. (x >> 8) == 0
  return (!((x + ~0x30 + 1) >> 31)) & ((x + ~0x3A + 1) >> 31) & (!(x >> 8));
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z)
{
  // 翻译一下：
  // 1. x 作为 condition
  // 2. 关键在于构造 mask: 利用移位:
  //    0xFFFFFFFF: ~1 + 1
  //    0x00000000: ~mask

  // mask = ~!!x + 1, 注意这里使用 !!x 而不是 x 的原因是 x 可能是任意值，而这里只希望其非0即1
  // (y & (mask)) | (z & (~mask)), mask 是由 condition 拓展而来
  int mask = (!!x << 31) >> 31;
  return (y & mask) | (z & ~mask);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y)
{
  // 注意，这里要考虑溢出问题！如果 x 和 y 符号不同相减容易溢出，但是直接判断很方便
  // x <= y -> y - x >= 0 -> y + ~x + 1 >= 0
  int flag_x = x >> 31;
  int flag_y = y >> 31;
  int mask = flag_x ^ flag_y;
  // mask 全1: flag_x, 1表示负数，也就更小; else: flag_2
  int flag_2 = (y + ~x + 1) >> 31;
  // flag_x ^ flag_y 全1时表示不同符号，全0表示同号， flag 起作用
  return !!((~mask & ~flag_2) | (mask & flag_x));
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x)
{
  // 1. 计算 ~x + 1
  // 2. 关键： x | (~x + 1) 可以只留下符号位
  // 3. >> 31 只保留符号位，此时为 0 / -1 ，将其转换为 1 / 0
  int flag = x | (~x + 1);
  return (flag >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x)
{
  // 思路：
  // 对正数，找最高位的 1；
  // 对负数，找最高位的 0然后 + 1，实际上等同于取反变成正数然后找最高位1
  // 搜索方法：二分
  // 注意每次搜索完后，如果有东西要移位！

  int b16, b8, b4, b2, b1, b0, sign;

  // 判断是否是负数，如果是负数按位取反
  sign = x >> 31;
  x = (~x & sign) | (x & ~sign);

  // if x & 1111 1111 0000 0000 is true: num_bits += 8 (0xFF00FF00)
  // if x & 1111 0000 1111 0000 is true: num_bits += 4 (0xF0F0F0F0)
  // if x & 1100 1100 1100 1100 is true: num_bits += 2 (0xCCCCCCCC)
  // if x & 1010 1010 1010 1010 is true: num_bits += 1 (0xAAAAAAAA)

  // 二分查找最高位的 1
  b16 = (!!(x >> 16)) << 4;
  x = x >> b16;
  b8 = (!!(x >> 8)) << 3;
  x = x >> b8;
  b4 = (!!(x >> 4)) << 2;
  x = x >> b4;
  b2 = (!!(x >> 2)) << 1;
  x = x >> b2;
  b1 = (!!(x >> 1));
  x = x >> b1;
  b0 = x;
  return b16 + b8 + b4 + b2 + b1 + b0 + 1;
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf)
{
  // float: 1 + 8 + 23 = sign + exp + frac

  // for normalized (exp != 0)
  // 1. 使用 移位 + 0xff mask 只保留 exp 部分
  // 2. 如果 exp 部分全 1 则溢出
  // 3. 否则 exp 部分直接 + 1
  // IMPORTANT: for denormalized!!!
  int sign_exp = uf >> 23;
  int exp = sign_exp & 0xff;
  int frac = uf - (sign_exp << 23);
  // for nan or infinity
  if (exp == 0xff)
  {
    return uf;
  }
  // for denormalized
  else if (!exp)
  {
    return (sign_exp << 23) + (frac << 1);
  }
  return uf + (1 << 23);
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf)
{
  // 注意： E = exp - bias!!
  int sign = (uf >> 31) & 1;
  int sign_exp = uf >> 23;
  int exp = sign_exp & 0xff;
  int E = exp - 127;
  int frac = uf - (sign_exp << 23);
  int num;

  // 1. 如果 E < 0, 表示原来这个数字就 < 1，转换为 int 后直接返回0
  if (E < 0)
  {
    return 0;
  }

  // 2. 溢出：大于 int 能表示的最大范围，即 2^31 - 1
  if (E >= 31)
  {
    return 0x80000000;
  }

  // 3. 需要判断 E 与 23 大小，因为涉及到是左移还是右移；
  // 加上 1 在最前面
  num = frac + (1 << 23);
  if (E < 23)
  {
    num = num >> (23 - E);
  }
  else
  {
    num = num << (E - 23);
  }

  if (sign)
  {
    return -num;
  }
  return num;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x)
{
  // 1. if too small, return 0
  // too small: x < -127
  if (x < -127)
  {
    return 0;
  }

  // 2. if too large, return +inf (0 1111 1111 00000000000000) (0x7F800000)
  // too large: x > 128
  if (x > 128)
  {
    return 0x7f800000;
  }

  // 3. denorm: x == -127, exp = x + 127 == 0
  if (x == -127)
  {
    return 1;
  }

  // 4. norm
  return (x + 127) << 23;
}
