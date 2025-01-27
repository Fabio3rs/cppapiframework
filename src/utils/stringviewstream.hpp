#pragma once

#include <sstream>
#include <string_view>
#include <utility>

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_stringviewbuf : public std::basic_streambuf<CharT, Traits> {
  public:
    // Types:
    using char_type = CharT;
    using traits_type = Traits;
    using int_type = typename traits_type::int_type;
    using pos_type = typename traits_type::pos_type;
    using off_type = typename traits_type::off_type;
    using streambuf_type = std::basic_streambuf<char_type, traits_type>;
    using stringview_type = std::basic_string_view<char_type, traits_type>;
    using size_type = typename stringview_type::size_type;

  private:
    std::ios_base::openmode MMode;
    stringview_type M_string; // Backing string_view buffer

  public:
    // Constructors:
    constexpr basic_stringviewbuf()
        : streambuf_type(), MMode(std::ios_base::in), M_string() {}

    explicit constexpr basic_stringviewbuf(std::ios_base::openmode mode)
        : streambuf_type(), MMode(mode), M_string() {}

    explicit constexpr basic_stringviewbuf(
        const stringview_type &in_str,
        std::ios_base::openmode inMode = std::ios_base::in)
        : streambuf_type(), MMode(inMode), M_string(in_str) {
        M_initialize_buffer(inMode);
    }

    // Move constructor and assignment
    constexpr basic_stringviewbuf(basic_stringviewbuf &&other) noexcept
        : streambuf_type(std::move(other)), MMode(other.MMode),
          M_string(std::move(other.M_string)) {
        swap(other);
    }

    constexpr auto operator=(basic_stringviewbuf &&other) noexcept
        -> basic_stringviewbuf & {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

    void swap(basic_stringviewbuf &other) noexcept {
        std::swap(MMode, other.MMode);
        std::swap(M_string, other.M_string);
        streambuf_type::swap(other);
    }

    // Deleted copy constructor and assignment operator
    basic_stringviewbuf(const basic_stringviewbuf &) = delete;
    auto operator=(const basic_stringviewbuf &)
        -> basic_stringviewbuf & = delete;

    ~basic_stringviewbuf() override = default;

    // Getters and setters:
    [[nodiscard]] auto str() const -> stringview_type {
        return M_string.substr(0, this->pptr() - this->pbase());
    }

    void str(const stringview_type &new_str) {
        M_string = new_str;
        M_initialize_buffer(MMode);
    }

  protected:
    void M_initialize_buffer(std::ios_base::openmode mode) {
        MMode = mode;
        const char_type *data = M_string.data();
        size_type len = M_string.size();

        if ((MMode & (std::ios_base::ate | std::ios_base::app)) != 0) {
            this->setp(const_cast<char_type *>(data),
                       const_cast<char_type *>(data + len));
        }

        if ((MMode & std::ios_base::in) != 0) {
            this->setg(const_cast<char_type *>(data),
                       const_cast<char_type *>(data),
                       const_cast<char_type *>(data + len));
        }
    }

    constexpr auto overflow([[maybe_unused]] int_type ch = traits_type::eof())
        -> int_type override {
        return traits_type::eof();
    }

    auto underflow() -> int_type override {
        if ((MMode & std::ios_base::in) != 0 && this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }
        return traits_type::eof();
    }

    auto pbackfail([[maybe_unused]] int_type ch = traits_type::eof())
        -> int_type override {
        if (this->eback() < this->gptr()) {
            this->gbump(-1);
            if (!traits_type::eq_int_type(ch, traits_type::eof())) {
                *this->gptr() = traits_type::to_char_type(ch);
            }
            return ch;
        }
        return traits_type::eof();
    }

    auto seekoff(off_type off, std::ios_base::seekdir way,
                 std::ios_base::openmode mode) -> pos_type override {
        if ((mode & MMode) == 0) {
            return pos_type(off_type(-1));
        }

        char_type *base =
            (mode & std::ios_base::in) != 0 ? this->eback() : this->pbase();
        char_type *current =
            (mode & std::ios_base::in) != 0 ? this->gptr() : this->pptr();
        char_type *end =
            (mode & std::ios_base::in) != 0 ? this->egptr() : this->epptr();

        off_type newpos = (way == std::ios_base::cur)   ? (current - base + off)
                          : (way == std::ios_base::beg) ? off
                          : (way == std::ios_base::end) ? (end - base + off)
                                                        : -1;

        if (newpos < 0 || newpos > (end - base)) {
            return pos_type(off_type(-1));
        }

        if ((mode & std::ios_base::in) != 0) {
            this->setg(this->eback(), this->eback() + newpos, this->egptr());
        }

        if ((mode & std::ios_base::out) != 0) {
            this->setp(this->pbase(), this->epptr());
            this->pbump(static_cast<int>(newpos));
        }

        return pos_type(newpos);
    }

    auto seekpos(pos_type sp, std::ios_base::openmode mode)
        -> pos_type override {
        return seekoff(off_type(sp), std::ios_base::beg, mode);
    }
};

using stringstream_view = basic_stringviewbuf<char>;
using wstringstream_view = basic_stringviewbuf<wchar_t>;
