#include "OFC_Firmware.h"

#include <stdlib.h>

int ofc_compare_versions(const char* a, const char* b) {
    if (!a) a = "";
    if (!b) b = "";

    while (*a == 'v' || *a == 'V') a++;
    while (*b == 'v' || *b == 'V') b++;

    for (int part = 0; part < 3; ++part) {
        char* endA = nullptr;
        char* endB = nullptr;

        long va = 0;
        long vb = 0;

        if (*a) va = strtol(a, &endA, 10);
        if (*b) vb = strtol(b, &endB, 10);

        if (va < vb) return -1;
        if (va > vb) return 1;

        a = (endA && endA != a) ? endA : a;
        b = (endB && endB != b) ? endB : b;

        while (*a && *a != '.') a++;
        while (*b && *b != '.') b++;
        if (*a == '.') a++;
        if (*b == '.') b++;
    }

    return 0;
}

