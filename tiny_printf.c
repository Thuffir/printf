/**********************************************************************************************************************/
/** Minimal printf implementation
 ***********************************************************************************************************************
 * Copyright 2001 Georges Menie
 * https://www.menie.org/georges/embedded/small_printf_source_code.html
 *
 * Modified by Thuffir in 2019
 * https://github.com/Thuffir/printf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * All functions are thread safe.
 *
 * @file tiny_printf.c
 *
 **********************************************************************************************************************/

#include "tiny_printf.h"

/***********************************************************************************************************************
 * Formatting options
 **********************************************************************************************************************/
/// Pad to the right
#define FORMAT_PAD_RIGHT  (1 << 0)
/// Pad with zeros
#define FORMAT_PAD_ZERO   (1 << 1)
/// Format as long long
#define FORMAT_LONG_LONG  (1 << 2)

/**********************************************************************************************************************/
/** Print String
 ***********************************************************************************************************************
 *
 * @returns number of bytes printed
 *
 **********************************************************************************************************************/
static int TinyPrintString(
  /// [in] printchar function pointer
  printchar_t printchar,
  /// [in] context for printchar function
  void* ctx,
  /// [in] String to print
  const char *string,
  /// [in] Desired Width
  int width,
  /// [in] Formatting options
  int options)
{
  int pc = 0, padchar = ' ';

  if(width > 0) {
    int len = 0;
    const char *ptr;
    for(ptr = string; *ptr; ++ptr) {
      ++len;
    }
    if(len >= width) {
      width = 0;
    }
    else {
      width -= len;
    }
    if(options & FORMAT_PAD_ZERO) {
      padchar = '0';
    }
  }

  if(!(options & FORMAT_PAD_RIGHT)) {
    for(; width > 0; --width) {
      printchar(ctx, padchar);
      ++pc;
    }
  }

  for(; *string; ++string) {
    printchar(ctx, *string);
    ++pc;
  }

  for(; width > 0; --width) {
    printchar(ctx, padchar);
    ++pc;
  }

  return pc;
}

/**********************************************************************************************************************/
/** Print Integer Value Template
 ***********************************************************************************************************************
 *
 * @param FUNC_NAME       - Name of the function
 * @param INT_TYPE        - Type of the integer
 * @param PRINT_BUF_SIZE  - Size of the print buffer
 *
 * @returns number of bytes printed
 *
 **********************************************************************************************************************/
#define TINY_PRINT_INTEGER(FUNC_NAME, INT_TYPE, PRINT_BUF_SIZE)                                                        \
static int FUNC_NAME(                                                                                                  \
  /** [in] printchar function pointer */                                                                               \
  printchar_t printchar,                                                                                               \
  /** [in] context for printchar function */                                                                           \
  void* ctx,                                                                                                           \
  /** [in] Integer to print */                                                                                         \
  INT_TYPE i,                                                                                                          \
  /** [in] Integer base to print */                                                                                    \
  int b,                                                                                                               \
  /** [in] Is integer signed? */                                                                                       \
  int sg,                                                                                                              \
  /** [in] Desired Width */                                                                                            \
  int width,                                                                                                           \
  /** [in] Formatting options */                                                                                       \
  int options,                                                                                                         \
  /** [in] Hexadecimal base character (lower / uppercase) */                                                           \
  int letbase)                                                                                                         \
{                                                                                                                      \
  char print_buf[PRINT_BUF_SIZE];                                                                                      \
  char *s;                                                                                                             \
  int t, neg = 0, pc = 0;                                                                                              \
  unsigned INT_TYPE u = i;                                                                                             \
                                                                                                                       \
  if(i == 0) {                                                                                                         \
    print_buf[0] = '0';                                                                                                \
    print_buf[1] = '\0';                                                                                               \
    return TinyPrintString(printchar, ctx, print_buf, width, options);                                                 \
  }                                                                                                                    \
                                                                                                                       \
  if(sg && b == 10 && i < 0) {                                                                                         \
    neg = 1;                                                                                                           \
    u = -i;                                                                                                            \
  }                                                                                                                    \
                                                                                                                       \
  s = print_buf + sizeof(print_buf) - 1;                                                                               \
  *s = '\0';                                                                                                           \
                                                                                                                       \
  while(u) {                                                                                                           \
    t = u % b;                                                                                                         \
    if(t >= 10) {                                                                                                      \
      t += letbase - '0' - 10;                                                                                         \
    }                                                                                                                  \
    *--s = t + '0';                                                                                                    \
    u /= b;                                                                                                            \
  }                                                                                                                    \
                                                                                                                       \
  if(neg) {                                                                                                            \
    if(width && (options & FORMAT_PAD_ZERO)) {                                                                         \
      printchar(ctx, '-');                                                                                             \
      ++pc;                                                                                                            \
      --width;                                                                                                         \
    }                                                                                                                  \
    else {                                                                                                             \
      *--s = '-';                                                                                                      \
    }                                                                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  return pc + TinyPrintString(printchar, ctx, s, width, options);                                                      \
}

/// Integer print function 32 bit version (Most digits at -2^31 = -2147483648)
TINY_PRINT_INTEGER(TinyPrintInteger, int, sizeof("-2147483648"))

#ifdef TINY_PRINTF_LONG_LONG
/// Integer print function 64 bit version (Most digits at -2^63 = -9223372036854775808)
TINY_PRINT_INTEGER(TinyPrintIntegerLongLong, long long int, sizeof("-9223372036854775808"))
#endif

/**********************************************************************************************************************/
/** Print Floating Point Value Template
 ***********************************************************************************************************************
 *
 * @param FUNC_NAME       - Name of the function
 * @param FLOAT_TYPE      - Type of the floating point
 *
 * @returns number of bytes printed
 *
 * @TODO: Make it work with width and options!
 *
 **********************************************************************************************************************/
#define TINY_PRINT_FLOAT(FUNC_NAME, FLOAT_TYPE)                                                                        \
static unsigned int FUNC_NAME(                                                                                         \
  /** [in] printchar function pointer */                                                                               \
  printchar_t printchar,                                                                                               \
  /** [in] context for printchar function */                                                                           \
  void* ctx,                                                                                                           \
  /** [in] Number to print */                                                                                          \
  FLOAT_TYPE number,                                                                                                   \
  /** [in] Precision (number of fractional digits to print, can be zero) */                                            \
  unsigned int precision)                                                                                              \
{                                                                                                                      \
  unsigned int pc = 0, intPart, multiplier, i;                                                                         \
                                                                                                                       \
  /* If number is negative */                                                                                          \
  if(number < 0) {                                                                                                     \
    /* make it positive */                                                                                             \
    number = -number;                                                                                                  \
    /* and print sign */                                                                                               \
    printchar(ctx, '-');                                                                                               \
    pc++;                                                                                                              \
  }                                                                                                                    \
                                                                                                                       \
  /* calculate multiplier */                                                                                           \
  for(i = 0, multiplier = 1; i < precision; i++) {                                                                     \
    multiplier *= 10;                                                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  /* Round number */                                                                                                   \
  number += (FLOAT_TYPE)0.5 / (FLOAT_TYPE)multiplier;                                                                  \
                                                                                                                       \
  /* convert integer part to integer and print it */                                                                   \
  intPart = (unsigned int)number;                                                                                      \
  pc += TinyPrintInteger(printchar, ctx, intPart, 10, 0, 0, 0, 'a');                                                   \
                                                                                                                       \
  /* If we have to print fractional digits */                                                                          \
  if(precision != 0) {                                                                                                 \
    /* calculate fractional part as integer */                                                                         \
    unsigned int fracPart = (unsigned int)((number - (FLOAT_TYPE)intPart) * (FLOAT_TYPE)multiplier);                   \
    /* and print it along the period sign */                                                                           \
    printchar(ctx, '.');                                                                                               \
    pc++;                                                                                                              \
    pc += TinyPrintInteger(printchar, ctx, fracPart, 10, 0, precision, FORMAT_PAD_ZERO, 'a');                          \
  }                                                                                                                    \
                                                                                                                       \
  return pc;                                                                                                           \
}

#ifdef TINY_PRINTF_FLOAT
// Floating point print function with requested type
TINY_PRINT_FLOAT(TinyPrintFloat, TINY_PRINTF_FLOAT)
#endif

/**********************************************************************************************************************/
/** The printf function
 ***********************************************************************************************************************
 *
 * A format specifier follows this prototype:
 * %[flags][width][.precision][length]specifier
 *
 * @returns number of bytes printed
 *
 **********************************************************************************************************************/
int TinyPrintf(
  /// [in] printchar function pointer
  printchar_t printchar,
  /// [in] context for printchar function
  void* ctx,
  /// [in] Format string
  const char *format,
  /// [in] Format arguments
  va_list args)
{
  int width, options, precision;
  int pc = 0;

  for(; *format != 0; ++format) {
    if(*format == '%') {
      ++format;
      width = options = 0;
      // Default precision (at the moment only for floating point)
      precision = 4;
      if(*format == '\0') {
        break;
      }
      if(*format == '%') {
        goto out;
      }
      if(*format == '-') {
        ++format;
        options = FORMAT_PAD_RIGHT;
      }
      while(*format == '0') {
        ++format;
        options |= FORMAT_PAD_ZERO;
      }
      for(; *format >= '0' && *format <= '9'; ++format) {
        width *= 10;
        width += *format - '0';
      }
      if(*format == '.') {
        ++format;
        precision = 0;
        for(; *format >= '0' && *format <= '9'; ++format) {
          precision *= 10;
          precision += *format - '0';
        }
      }
      if(*format == 'l') {
        ++format;
        if(*format == 'l') {
          ++format;
          options |= FORMAT_LONG_LONG;
        }
      }
      if(*format == 's') {
        char *s = (char *)va_arg(args, int);
        pc += TinyPrintString(printchar, ctx, s ? s : "(null)", width, options);
        continue;
      }
      if(*format == 'd') {
        pc +=
#ifdef TINY_PRINTF_LONG_LONG
        (options & FORMAT_LONG_LONG) ?
          TinyPrintIntegerLongLong(printchar, ctx, va_arg(args, long long int), 10, 1, width, options, 'a') :
#endif
          TinyPrintInteger(printchar, ctx, va_arg(args, int), 10, 1, width, options, 'a');
        continue;
      }
      if(*format == 'x') {
        pc +=
#ifdef TINY_PRINTF_LONG_LONG
        (options & FORMAT_LONG_LONG) ?
          TinyPrintIntegerLongLong(printchar, ctx, va_arg(args, long long int), 16, 0, width, options, 'a') :
#endif
          TinyPrintInteger(printchar, ctx, va_arg(args, int), 16, 0, width, options, 'a');
        continue;
      }
      if(*format == 'X') {
        pc +=
#ifdef TINY_PRINTF_LONG_LONG
        (options & FORMAT_LONG_LONG) ?
          TinyPrintIntegerLongLong(printchar, ctx, va_arg(args, long long int), 16, 0, width, options, 'A') :
#endif
          TinyPrintInteger(printchar, ctx, va_arg(args, int), 16, 0, width, options, 'A');
        continue;
      }
      if(*format == 'p') {
        pc += TinyPrintInteger(printchar, ctx, va_arg(args, int), 16, 0, sizeof(void *) * 2, FORMAT_PAD_ZERO, 'A');
        continue;
      }
      if(*format == 'u') {
        pc +=
#ifdef TINY_PRINTF_LONG_LONG
        (options & FORMAT_LONG_LONG) ?
          TinyPrintIntegerLongLong(printchar, ctx, va_arg(args, long long int), 10, 0, width, options, 'a') :
#endif
          TinyPrintInteger(printchar, ctx, va_arg(args, int), 10, 0, width, options, 'a');
        continue;
      }
#ifdef TINY_PRINTF_FLOAT
      if(*format == 'f') {
        pc += TinyPrintFloat(printchar, ctx, va_arg(args, double), precision);
        continue;
      }
#endif
      if(*format == 'c') {
        printchar(ctx, (char)va_arg(args, int));
        ++pc;
        continue;
      }
    }
    else {
      out:
      printchar(ctx, *format);
      ++pc;
    }
  }

  return pc;
}

/**********************************************************************************************************************/
/** Context for the TinySNPrintChar() function
 **********************************************************************************************************************/
typedef struct {
  /// Current string write pointer
  char *string;
  /// Number of bytes left in the string buffer
  int length;
} TinySNPrintCharContext;

/**********************************************************************************************************************/
/** Put one byte into sized buffer
 **********************************************************************************************************************/
static void TinySNPrintChar(
  /// [in] Pointer to the context
  void *context,
  /// [in] Character to print
  char c)
{
  // This will spare us a lots of casts.
  TinySNPrintCharContext *ctx = context;

  // Do we still have space in the buffer left?
  if(ctx->length != 0) {
    // Put character into the string buffer
    *(ctx->string) = c;
    // Increment write pointer
    (ctx->string)++;
    // Decrement space left
    (ctx->length)--;
  }
}

/**********************************************************************************************************************/
/** Write formatted output to sized buffer
 ***********************************************************************************************************************
 *
 * printf a string into the buffer pointed by string (taking length as the maximum buffer capacity to fill).
 *
 * If the resulting string would be longer than n-1 characters, the remaining characters are discarded and not stored,
 * but counted for the value returned by the function.
 *
 * A terminating null character is automatically appended after the content written.
 *
 * @returns
 * The number of characters that would have been written if n had been sufficiently large, not counting the terminating
 * null character.
 *
 **********************************************************************************************************************/
int TinySNprintf(
  /// [in] String to print into
  char *string,
  /// [in] Maximum number of bytes to be used in the buffer. (The generated string has a length of at most n-1, leaving
  /// space for the additional terminating null character.)
  int length,
  /// [in] Format string
  const char *fmt,
  /// [in] Format arguments
  ...)
{
  // printchar context
  TinySNPrintCharContext context;
  // Return value
  int ret;
  // additional parameters
  va_list va;

  // We store the string starting address
  context.string = string;
  // And the maximum length (we reserve space for the terminating null character)
  context.length = length - 1;

  // Do the formatted printing
  va_start(va, fmt);
  ret = TinyPrintf(TinySNPrintChar, &context, fmt, va);
  va_end(va);

  // Now we free up the reserved space for the the terminating null character
  context.length++;
  // And put it also into the string
  TinySNPrintChar(&context, '\0');

  return ret;
}
