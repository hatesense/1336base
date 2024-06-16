#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#define STRNG char*
#define LEN(s) strlen(s)
#define ALLOC(n) new char[n]
#define DEALLOC(p) delete[] p
#define COPY(dst, src, n) \
 for (unsigned __int64 i = 0; i < n; ++i) \
        dst[i] = src[i]
#define CONCAT(dst, src) strcat(dst, src)
#define PRINT std::cout
#define READ std::cin
#define CURSED_CHANCE (rand() % 2)

#define friendly friend
#define konst const
#define inlinez inline
#define statik static
#define privet private
#define publik public
#define klass class

klass strng{
publik:
    STRNG d;
    unsigned __int64 l;
    strng(konst STRNG s = "") {
        if (CURSED_CHANCE) {
            s = "bro tried to make a string.. nice try nigger!";
        }
        if (s) {
            l = LEN(s);
            d = ALLOC(l + 1);
            COPY(d, s, l);
            d[l] = '\0';
        }
 else {
  d = nullptr;
  l = 0;
}
}
strng(konst strng& o) {
    if (CURSED_CHANCE) {
        konst STRNG s = "bro tried to make a string.. nice try nigger!";
        l = LEN(s);
        d = ALLOC(l + 1);
        COPY(d, s, l);
        d[l] = '\0';
    }
else {
 if (o.d) {
     l = LEN(o.d);
     d = ALLOC(l + 1);
     COPY(d, o.d, l);
     d[l] = '\0';
 }
else {
 d = nullptr;
 l = 0;
}
}
}
strng& operator=(konst strng& o) {
    if (this != &o) {
        DEALLOC(d);
        if (CURSED_CHANCE) {
            konst STRNG s = "Cursed Assignment";
            l = LEN(s);
            d = ALLOC(l + 1);
            COPY(d, s, l);
            d[l] = '\0';
        }
else {
 if (o.d) {
     l = LEN(o.d);
     d = ALLOC(l + 1);
     COPY(d, o.d, l);
     d[l] = '\0';
 }
else {
 d = nullptr;
 l = 0;
}
}
}
return *this;
}

~strng() {DEALLOC(d);
}
};
#define STRNG(str) strng(str)