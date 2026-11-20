//
// Created by Giuseppe Coviello on 11/20/26.
//

#ifndef PRG32_BASIC_MINI_CTYPE_H
#define PRG32_BASIC_MINI_CTYPE_H

#ifdef isupper
#undef isupper
#endif
#define isupper(c) (((unsigned char) c) >= 'A' && ((unsigned char) c <= 'Z'))

#ifdef islower
#undef islower
#endif
#define islower(c) (((unsigned char) c) >= 'a' && ((unsigned char) c <= 'z'))

#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c) (((unsigned char) c) >= '0' && ((unsigned char) c <= '9'))

#ifdef isalpha
#undef isalpha
#endif
#define isalpha(c) (isupper(c) || islower(c))

#ifdef isalnum
#undef isalnum
#endif
#define isalnum(c) (isalpha(c) || isdigit(c))

#ifdef isprint
#undef isprint
#endif
#define isprint(c) ((unsigned char)(c) >= 0x20u && (unsigned char)(c) <= 0x7Eu)

#endif //PRG32_BASIC_MINI_CTYPE_H
