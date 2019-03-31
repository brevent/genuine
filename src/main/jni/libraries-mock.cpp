//
// Created by Thom on 2019/3/3.
//

#include <dlfcn.h>
#include "common.h"
#include "libraries-mock.h"

#if (defined(CHECK_XPOSED_EPIC) || defined(CHECK_SO_LIBRARY)) && defined(GENUINE_NO_STL)

#include <jni.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "plt.h"

typedef ptrdiff_t streamsize;

class ios_base {

public:
    typedef unsigned int fmtflags;
    static const fmtflags boolalpha = 0x0001;
    static const fmtflags dec = 0x0002;
    static const fmtflags fixed = 0x0004;
    static const fmtflags hex = 0x0008;
    static const fmtflags internal = 0x0010;
    static const fmtflags left = 0x0020;
    static const fmtflags oct = 0x0040;
    static const fmtflags right = 0x0080;
    static const fmtflags scientific = 0x0100;
    static const fmtflags showbase = 0x0200;
    static const fmtflags showpoint = 0x0400;
    static const fmtflags showpos = 0x0800;
    static const fmtflags skipws = 0x1000;
    static const fmtflags unitbuf = 0x2000;
    static const fmtflags uppercase = 0x4000;
    static const fmtflags adjustfield = left | right | internal;
    static const fmtflags basefield = dec | oct | hex;
    static const fmtflags floatfield = scientific | fixed;

    typedef unsigned int iostate;
    static const iostate badbit = 0x1;
    static const iostate eofbit = 0x2;
    static const iostate failbit = 0x4;
    static const iostate goodbit = 0x0;

    virtual ~ios_base();

protected:
    ios_base() {

    }

protected:
    fmtflags fmtflags_;
    streamsize precision_;
    streamsize width_;
    iostate rdstate_;
    iostate exceptions_;
    void *rdbuf_;
    void *loc_;
    void *fn_;
    int *index_;
    size_t event_size_;
    size_t event_cap_;
    long *iarray_;
    size_t iarray_size_;
    size_t iarray_cap_;
    void **parray_;
    size_t parray_size_;
    size_t parray_cap_;
};

ios_base::~ios_base() {

}

class basic_ios : public ios_base {

public:
    virtual ~basic_ios();

protected:
    basic_ios() {

    }

protected:
    void *tie_;
    int fill_;
};

basic_ios::~basic_ios() {

}

class basic_ostream : virtual public basic_ios {

public:
    basic_ostream(void *sb, void *loc);

    virtual ~basic_ostream();
};

basic_ostream::basic_ostream(void *sb, void *loc) {
    rdbuf_ = sb;
    rdstate_ = rdbuf_ ? goodbit : badbit;
    exceptions_ = goodbit;
    fmtflags_ = skipws | dec;
    width_ = 0;
    precision_ = 6;
    fn_ = nullptr;
    index_ = nullptr;
    event_size_ = 0;
    event_cap_ = 0;
    iarray_ = nullptr;
    iarray_size_ = 0;
    iarray_cap_ = 0;
    parray_ = nullptr;
    parray_size_ = 0;
    parray_cap_ = 0;
    loc_ = loc;

    tie_ = nullptr;
    fill_ = EOF;
}

basic_ostream::~basic_ostream() {

}

class locale {

public:
    locale();

    ~locale();

protected:
    void *locale_;
};

locale::locale() {
    locale_ = nullptr;
}

locale::~locale() {

}

#define SIZE 512

class basic_streambuf {

public:
    basic_streambuf();

    virtual ~basic_streambuf();

    inline char *data() {
        return strndup(bout_, nout_ - bout_);
    }

protected:
    virtual void imbue();

    virtual void setbuf();

    virtual void seekoff();

    virtual void seekpos();

    virtual void sync();

    virtual void showmanyc();

    virtual void xsgetn();

    virtual void underflow();

    virtual void uflow();

    virtual void pbackfail();

    virtual streamsize xsputn(const char *s, streamsize n);

    virtual void overflow();

protected:
    locale loc_;
    char *binp_;
    char *ninp_;
    char *einp_;
    char *bout_;
    char *nout_;
    char *eout_;
};

basic_streambuf::basic_streambuf() {
    binp_ = ninp_ = einp_ = nullptr;
    bout_ = (char *) calloc(1, SIZE);
    nout_ = bout_;
    eout_ = bout_ + (SIZE - 1);
}

basic_streambuf::~basic_streambuf() {
    free(bout_);
    bout_ = nout_ = eout_ = nullptr;
}

void basic_streambuf::imbue() {

}

void basic_streambuf::setbuf() {

}

void basic_streambuf::seekoff() {

}

void basic_streambuf::seekpos() {

}

void basic_streambuf::sync() {

}

void basic_streambuf::showmanyc() {

}

void basic_streambuf::xsgetn() {

}

void basic_streambuf::underflow() {

}

void basic_streambuf::uflow() {

}

void basic_streambuf::pbackfail() {

}

streamsize basic_streambuf::xsputn(const char *s, streamsize n) {
    auto k = (size_t) n;
    if (k > 0) {
        size_t size = nout_ - bout_;
        size_t total = eout_ - bout_;
        if (size + k >= total) {
            total += (k / SIZE + 1) * SIZE;
            bout_ = (char *) realloc(bout_, total);
            eout_ = bout_ + total;
            nout_ = bout_ + size;
        }
        memcpy(nout_, s, k);
        nout_ += k;
    }
    return n;
}

void basic_streambuf::overflow() {

}

void operator delete(void *ptr) {
    ::free(ptr);
}

static void inline fill_ZNSt3__16locale7classicEv(char v[]) {
    // _ZNSt3__16locale7classicEv
    static unsigned int m = 0;

    if (m == 0) {
        m = 23;
    } else if (m == 29) {
        m = 31;
    }

    v[0x0] = '\\';
    v[0x1] = '^';
    v[0x2] = 'K';
    v[0x3] = 'U';
    v[0x4] = 's';
    v[0x5] = ';';
    v[0x6] = 'V';
    v[0x7] = 'U';
    v[0x8] = ':';
    v[0x9] = ':';
    v[0xa] = 'a';
    v[0xb] = 'a';
    v[0xc] = 'l';
    v[0xd] = 'q';
    v[0xe] = '}';
    v[0xf] = 'w';
    v[0x10] = '$';
    v[0x11] = 'w';
    v[0x12] = 'y';
    v[0x13] = 'w';
    v[0x14] = 's';
    v[0x15] = 'r';
    v[0x16] = 'k';
    v[0x17] = '`';
    v[0x18] = 'A';
    v[0x19] = 's';
    for (unsigned int i = 0; i < 0x1a; ++i) {
        v[i] ^= ((i + 0x1a) % m);
    }
    v[0x1a] = '\0';
}

char *dump_jvm(JavaVM *jvm, void (*DumpForSigQuit)(void *, void *)) {
    char v[0x1b];
    fill_ZNSt3__16locale7classicEv(v);

    void *method = dlsym(RTLD_NEXT, v);
#ifdef DEBUG
    LOGI("std::__1::locale::classic(): %p", method);
#endif
    if (method == nullptr) {
        method = plt_dlsym(v, NULL);
        if (method == nullptr) {
            return nullptr;
        }
    }
    void **classic = ((void **(*)()) method)();

    basic_streambuf sb;
    basic_ostream os(&sb, *classic);

    DumpForSigQuit(jvm, &os);

    return sb.data();
}

#endif
