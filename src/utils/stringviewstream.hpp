#pragma once

#include <limits>
#include <sstream>
#include <string_view>

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_stringviewbuf : public std::basic_streambuf<CharT, Traits> {
    struct xfer_bufptrs;

  public:
    // Types:
    using char_type = CharT;
    using traits_type = Traits;
    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 251. basic_stringviewbuf missing allocator_type
    using int_type = typename traits_type::int_type;
    using pos_type = typename traits_type::pos_type;
    using off_type = typename traits_type::off_type;

    using streambuf_type = std::basic_streambuf<char_type, traits_type>;
    using stringview_type = std::basic_string_view<char_type, Traits>;
    using size_type = typename stringview_type::size_type;

  protected:
    /// Place to stash in || out || in | out settings for current stringviewbuf.
    std::ios_base::openmode MMode;

    // Data Members:
    stringview_type M_string;

  public:
    // Constructors:

    /**
     *  @brief  Starts with an empty string buffer.
     *
     *  The default constructor initializes the parent class using its
     *  own default ctor.
     */
    basic_stringviewbuf()
        : streambuf_type(), MMode(std::ios_base::in), M_string() {}

    /**
     *  @brief  Starts with an empty string buffer.
     *  @param  mode  Whether the buffer can read, or write, or both.
     *
     *  The default constructor initializes the parent class using its
     *  own default ctor.
     */
    explicit basic_stringviewbuf(std::ios_base::openmode mode)
        : streambuf_type(), MMode(mode), M_string() {}

    /**
     *  @brief  Starts with an existing string buffer.
     *  @param  in_str  A string to copy as a starting buffer.
     *  @param  inMode  Whether the buffer can read, or write, or both.
     *
     *  This constructor initializes the parent class using its
     *  own default ctor.
     */
    explicit basic_stringviewbuf(
        const stringview_type &in_str,
        std::ios_base::openmode inMode = std::ios_base::in)
        : streambuf_type(), MMode(), M_string(in_str.data(), in_str.size()) {
        M_stringviewbuf_init(inMode);
    }

    ~basic_stringviewbuf() override = default;

#if __cplusplus >= 201103L
    basic_stringviewbuf(const basic_stringviewbuf &) = delete;

    basic_stringviewbuf(basic_stringviewbuf &&i_rhs) noexcept
        : basic_stringviewbuf(std::move(i_rhs), xfer_bufptrs(i_rhs, this)) {
        i_rhs.M_sync(const_cast<char_type *>(i_rhs.M_string.data()), 0, 0);
    }
    // 27.8.2.2 Assign and swap:

    auto operator=(const basic_stringviewbuf &)
        -> basic_stringviewbuf & = delete;

    auto operator=(basic_stringviewbuf &&i_rhs) noexcept
        -> basic_stringviewbuf & {
        xfer_bufptrs i_st{i_rhs, this};
        const streambuf_type &i_base = i_rhs;
        streambuf_type::operator=(i_base);
        this->pubimbue(i_rhs.getloc());
        MMode = i_rhs.MMode;
        M_string = std::move(i_rhs.M_string);
        i_rhs.M_sync(const_cast<char_type *>(i_rhs.M_string.data()), 0, 0);
        return *this;
    }

    void swap(basic_stringviewbuf &i_rhs) noexcept {
        xfer_bufptrs i_l_st{*this, std::addressof(i_rhs)};
        xfer_bufptrs i_r_st{i_rhs, this};
        streambuf_type &i_base = i_rhs;
        streambuf_type::swap(i_base);
        i_rhs.pubimbue(this->pubimbue(i_rhs.getloc()));
        std::swap(MMode, i_rhs.MMode);
        std::swap(M_string, i_rhs.M_string); // XXX not exception safe
    }
#endif // C++11

    // Getters and setters:

    /**
     *  @brief  Copying out the string buffer.
     *  @return  A copy of one of the underlying sequences.
     *
     *  <em>If the buffer is only created in input mode, the underlying
     *  character sequence is equal to the input sequence; otherwise, it
     *  is equal to the output sequence.</em> [27.7.1.2]/1
     */
    auto str() const -> stringview_type {
        stringview_type i_ret(M_string.getAllocator());
        if (char_type *i_hi = M_highMark()) {
            i_ret.assign(this->pbase(), i_hi);
        } else {
            i_ret = M_string;
        }
        return i_ret;
    }
    /**
     *  @brief  Setting a new buffer.
     *  @param  i_s  The string to use as a new sequence.
     *
     *  Deallocates any previous stored sequence, then copies @a s to
     *  use as a new one.
     */
    void str(const stringview_type &i_s) {
        // Cannot use M_string = i_s, since v3 strings are COW
        // (not always true now but assign() always works).
        M_string.assign(i_s.data(), i_s.size());
        M_stringviewbuf_init(MMode);
    }

#if __cplusplus > 201703L && _GLIBCXX_USE_CXX11_ABI
    void str(stringview_type &&i_s) {
        M_string = std::move(i_s);
        M_stringviewbuf_init(MMode);
    }
#endif

  protected:
    // Common initialization code goes here.
    void M_stringviewbuf_init(std::ios_base::openmode inMode) {
        MMode = inMode;
        size_type i_len = 0;
        if ((MMode & (std::ios_base::ate | std::ios_base::app)) != 0) {
            i_len = M_string.size();
        }
        M_sync(const_cast<char_type *>(M_string.data()), 0, i_len);
    }

    auto showmanyc() -> std::streamsize override {
        std::streamsize i_ret = -1;
        if ((MMode & std::ios_base::in) != 0) {
            M_update_egptr();
            i_ret = this->egptr() - this->gptr();
        }
        return i_ret;
    }

    auto underflow() -> int_type override {
        int_type ret = traits_type::eof();
        const bool testin = this->MMode & std::ios_base::in;
        if (testin) {
            // Update egptr() to match the actual string end.
            M_update_egptr();

            if (this->gptr() < this->egptr()) {
                ret = traits_type::to_int_type(*this->gptr());
            }
        }
        return ret;
    }

    auto pbackfail(int_type i_c = traits_type::eof()) -> int_type override {
        int_type ret = traits_type::eof();
        if (this->eback() < this->gptr()) {
            // Try to put back __c into input sequence in one of three ways.
            // Order these tests done in is unspecified by the standard.
            const bool testeof = traits_type::eq_int_type(i_c, ret);
            if (!testeof) {
                const bool testeq = traits_type::eq(
                    traits_type::to_char_type(i_c), this->gptr()[-1]);
                const bool testout = this->MMode & std::ios_base::out;
                if (testeq || testout) {
                    this->gbump(-1);
                    if (!testeq) {
                        *this->gptr() = traits_type::to_char_type(i_c);
                    }
                    ret = i_c;
                }
            } else {
                this->gbump(-1);
                ret = traits_type::not_eof(i_c);
            }
        }
        return ret;
    }

    auto overflow([[maybe_unused]] int_type i_c = traits_type::eof())
        -> int_type override {
        return traits_type::eof();
    }

    /**
     *  @brief  Manipulates the buffer.
     *  @param  i_s  Pointer to a buffer area.
     *  @param  i_n  Size of @a i_s.
     *  @return  @c this
     *
     *  If no buffer has already been created, and both @a i_s and @a i_n are
     *  non-zero, then @c i_s is used as a buffer; see
     *  https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html#io.streambuf.buffering
     *  for more.
     */
    auto setbuf(char_type *i_s, std::streamsize i_n)
        -> streambuf_type * override {
        if (i_s && i_n >= 0) {
            // This is implementation-defined behavior, and assumes
            // that an external char_type array of length i_n exists
            // and has been pre-allocated. If this is not the case,
            // things will quickly blow up.

            // Step 1: Destroy the current internal array.
            M_string = {};

            // Step 2: Use the external array.
            M_sync(i_s, static_cast<size_t>(i_n), 0);
        }
        return this;
    }

    auto seekoff(off_type off, std::ios_base::seekdir way,
                 std::ios_base::openmode mode) -> pos_type override {
        pos_type ret = pos_type(off_type(-1));
        bool testin = (std::ios_base::in & MMode & mode) != 0;
        bool testout = (std::ios_base::out & MMode & mode) != 0;
        const bool testboth = testin && testout && way != std::ios_base::cur;
        testin &= (mode & std::ios_base::out) == 0;
        testout &= (mode & std::ios_base::in) == 0;

        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // 453. basic_stringbuf::seekoff need not always fail for an empty
        // stream.
        const char_type *beg = testin ? this->eback() : this->pbase();
        if ((beg || !off) && (testin || testout || testboth)) {
            M_update_egptr();

            off_type newoffi = off;
            off_type newoffo = newoffi;
            if (way == std::ios_base::cur) {
                newoffi += this->gptr() - beg;
                newoffo += this->pptr() - beg;
            } else if (way == std::ios_base::end) {
                newoffo = newoffi += this->egptr() - beg;
            }

            if ((testin || testboth) && newoffi >= 0 &&
                this->egptr() - beg >= newoffi) {
                this->setg(this->eback(), this->eback() + newoffi,
                           this->egptr());
                ret = pos_type(newoffi);
            }
            if ((testout || testboth) && newoffo >= 0 &&
                this->egptr() - beg >= newoffo) {
                M_pbump(this->pbase(), this->epptr(), newoffo);
                ret = pos_type(newoffo);
            }
        }
        return ret;
    }

    auto seekpos(pos_type sp,
                 std::ios_base::openmode inMode = std::ios_base::in)
        -> pos_type override {
        pos_type ret = pos_type(off_type(-1));
        const bool testin = (std::ios_base::in & this->MMode & inMode) != 0;
        const bool testout = (std::ios_base::out & this->MMode & inMode) != 0;

        const char_type *beg = testin ? this->eback() : this->pbase();
        if ((beg || !off_type(sp)) && (testin || testout)) {
            M_update_egptr();

            const off_type pos(sp);
            const bool testpos = (0 <= pos && pos <= this->egptr() - beg);
            if (testpos) {
                if (testin) {
                    this->setg(this->eback(), this->eback() + pos,
                               this->egptr());
                }
                if (testout) {
                    M_pbump(this->pbase(), this->epptr(), pos);
                }
                ret = sp;
            }
        }
        return ret;
    }

    // Internal function for correctly updating the internal buffer
    // for a particular M_string, due to initialization or re-sizing
    // of an existing M_string.
    void M_sync(char_type *i_base, size_type i_i, size_type i_o) {
        (void)i_o;
        const bool testin = (MMode & std::ios_base::in) != 0;
        char_type *endg = i_base + M_string.size();
        char_type *endp = i_base + M_string.size();

        if (i_base != M_string.data()) {
            // setbuf: __i == size of buffer area (_M_string.size() == 0).
            endg += i_i;
            i_i = 0;
            endp = endg;
        }

        if (testin) {
            this->setg(i_base, i_base + i_i, endg);
        }
    }

    // Internal function for correctly updating egptr() to the actual
    // string end.
    void M_update_egptr() {
        if (char_type *i_pptr = this->pptr()) {
            char_type *i_egptr = this->egptr();
            if (!i_egptr || i_pptr > i_egptr) {
                if ((MMode & std::ios_base::in) != 0) {
                    this->setg(this->eback(), this->gptr(), i_pptr);
                } else {
                    this->setg(i_pptr, i_pptr, i_pptr);
                }
            }
        }
    }

    // Works around the issue with pbump, part of the protected
    // interface of basic_streambuf, taking just an int.
    void M_pbump(char_type *i_pbeg, char_type *i_pend, off_type i_off) {
        this->setp(i_pbeg, i_pend);
        while (i_off > std::numeric_limits<int>::max()) {
            this->pbump(std::numeric_limits<int>::max());
            i_off -= std::numeric_limits<int>::max();
        }
        this->pbump(static_cast<int>(i_off));
    }

  private:
    // Return a pointer to the end of the underlying character sequence.
    // This might not be the same character as M_string.end() because
    // basic_stringviewbuf::overflow might have written to unused capacity
    // in M_string without updating its length.
    auto M_highMark() const _GLIBCXX_NOEXCEPT->char_type * {
        if (char_type *i_pptr = this->pptr()) {
            char_type *i_egptr = this->egptr();
            if (!i_egptr || i_pptr > i_egptr) {
                return i_pptr; // Underlying sequence is [pbase, pptr).
            }
            return i_egptr; // Underlying sequence is [pbase, egptr).
        }
        return 0; // Underlying character sequence is just M_string.
    }

#if __cplusplus >= 201103L
#if _GLIBCXX_USE_CXX11_ABI
    // This type captures the state of the gptr / pptr pointers as offsets
    // so they can be restored in another object after moving the string.
    struct xfer_bufptrs {
        xfer_bufptrs(const basic_stringviewbuf &i_from,
                     basic_stringviewbuf *i_to)
            : M_to{i_to}, M_goff{-1, -1, -1}, M_poff{-1, -1, -1} {
            const CharT *const in_str = i_from.M_string.data();
            const CharT *i_end = nullptr;
            if (i_from.eback()) {
                M_goff[0] = i_from.eback() - in_str;
                M_goff[1] = i_from.gptr() - in_str;
                M_goff[2] = i_from.egptr() - in_str;
                i_end = i_from.egptr();
            }
            if (i_from.pbase()) {
                M_poff[0] = i_from.pbase() - in_str;
                M_poff[1] = i_from.pptr() - i_from.pbase();
                M_poff[2] = i_from.epptr() - in_str;
                if (!i_end || i_from.pptr() > i_end) {
                    i_end = i_from.pptr();
                }
            }

            // Set M_string length to the greater of the get and put areas.
            if (i_end) {
                // The const_cast avoids changing this constructor's signature,
                // because it is exported from the dynamic library.
                auto &Mut_from = const_cast<basic_stringviewbuf &>(i_from);
                Mut_from.M_string.M_length(i_end - in_str);
            }
        }

        ~xfer_bufptrs() {
            auto *in_str = const_cast<char_type *>(M_to->M_string.data());
            if (M_goff[0] != -1) {
                M_to->setg(in_str + M_goff[0], in_str + M_goff[1],
                           in_str + M_goff[2]);
            }
            if (M_poff[0] != -1) {
                M_to->M_pbump(in_str + M_poff[0], in_str + M_poff[2],
                              M_poff[1]);
            }
        }

        basic_stringviewbuf *M_to;
        off_type M_goff[3];
        off_type M_poff[3];
    };
#else
    // This type does nothing when using Copy-On-Write strings.
    struct xfer_bufptrs {
        xfer_bufptrs(const basic_stringviewbuf &, basic_stringviewbuf *) {}
    };
#endif

    // The move constructor initializes an xfer_bufptrs temporary then
    // delegates to this constructor to performs moves during its lifetime.
    basic_stringviewbuf(basic_stringviewbuf &&i_rhs, xfer_bufptrs && /*unused*/)
        : streambuf_type(static_cast<const streambuf_type &>(i_rhs)),
          MMode(i_rhs.MMode), M_string(std::move(i_rhs.M_string)) {}
#endif // C++11
};

using stringstream_view = basic_stringviewbuf<char>;
using wstringstream_view = basic_stringviewbuf<wchar_t>;
