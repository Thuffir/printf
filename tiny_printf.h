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
 * @file tiny_printf.h
 *
 **********************************************************************************************************************/

#ifndef TINY_PRINTF_H_
#define TINY_PRINTF_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/// printchar function type
typedef void (*printchar_t)(void *, char);

int TinyPrintf(printchar_t printchar, void* ctx, const char *format, va_list args);
int TinySNprintf(char* s, int, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif //TINY_PRINTF_H_
