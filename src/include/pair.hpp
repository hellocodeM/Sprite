#ifndef SPRITE_PAIR_HPP
#define SPRITE_PAIR_HPP

template <class K, class V>
struct pair {
    using pair_type = pair<K, V>;

    bool operator== (const pair_type& other) const {
        return first == other.first;
    }

    bool operator!= (const pair_type& other) const {
        return !(*this == other);
    }

    K first;
    V second;
};

template <class K, class V>
pair<K, V> make_pair(K&& k, V&& v) {
    pair<K, V> res = {
        .first = k,
        .second = v
    };
    return res;
}
#endif
