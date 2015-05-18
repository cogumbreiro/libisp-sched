#ifndef _WINT_HPP
#define _WINT_HPP

struct WInt {
    WInt();
    WInt(int v);
    WInt(const WInt &c);
    bool operator== (const WInt &r) const;

    WInt & operator=(const WInt & other);

    bool isWildcard() const;
    int get() const;
    bool matches(const WInt &other) const;
private:
    bool wildcard;
    int value;
};

static WInt WILDCARD = WInt();

#endif
