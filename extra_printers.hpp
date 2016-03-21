//
// Created by joshm on 3/20/2016.
//

#ifndef MVIS_EXTRA_PRINTERS_HPP
#define MVIS_EXTRA_PRINTERS_HPP

template <class T, std::size_t N>
ostream& operator<<(ostream& o, const std::array<T, N>& arr)
{
    copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, " "));
    return o;
}

#endif //MVIS_EXTRA_PRINTERS_HPP
