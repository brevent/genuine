#!/usr/bin/env python
# encoding: utf-8
# Author: Liu DongMiao <liudongmiao@gmail.com>
# Created  : Sun 20 May 2018 12:04:18 AM CST
# Modified : Fri 22 Apr 2022 11:14:07 PM CST

import sys

k = sys.argv[1]
v = sys.argv[2]

l = len(v)

P = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137]
def find_m(x):
    p = P[:-2]
    low = 0
    high = len(p) - 1
    if x > p[high]:
        return p[high]
    elif x <= p[low]:
        return p[low]

    while low <= high:
        mid = int(low + (high - low) / 2)
        if x > p[mid]:
            if x < p[mid + 1]:
                return p[mid]
            else:
                low = mid + 1
        elif x < p[mid]:
            high = mid - 1
        else:
            return p[mid - 1]
    else:
        raise

m = find_m(l)
method = ''
if '(' in v or ';' in v:
    method = 'signature'
else:
    method = v.replace('/', '_').replace('%', '').replace(' ', '_')
print('static inline void fill_%s(char %s[]) {' % (method, k))
print('    // %s' % v)
print('    static unsigned int m = 0;')
print('''
    if (m == 0) {
        m = %d;
    } else if (m == %d) {
        m = %d;
    }
''' % (m, P[P.index(m) + 1], P[P.index(m) + 2]))

for x in range(l):
    o = ord(v[x]) ^ ((x + l) % m)
    s = repr(chr(o))
    if s == '''"'"''':
        s = "'\\''"
    print("    %s[0x%x] = %s%s;" % (k, x, o > 127 and "(char) " or "", s))

print('''    for (unsigned int i = 0; i < 0x%x; ++i) {
        %s[i] ^= ((i + 0x%x) %% m);
    }
    %s[0x%x] = '\\0';''' % (l, k, l, k, l))
print('}')

# vim: set sta sw=4 et:
