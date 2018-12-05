// Returns 1 if the strings are the same,
// considering their mask, else 0. The
// mask is a string of bytes in which 0x00
// means 'ignore' and all other values mean
// consider the character on this position).
int memcmpmask(
    char *m1,
    char *m2,
    char *mask,
    int size
) {
    int i;
    for (i = 0; i < size; i++) {
        if (mask[i] && m1[i] != m2[i])
            return 0;
    }
    return 1;
}

// Returns the address of the first occurence
// of "needle" in the "haystack", considering
// its mask (string of bytes in which 0x00
// means 'ignore' and all other values mean
// consider the character on this position).
char *memmemmask(
    char *haystack,
    int hlen,
    char *needle,
    char *mask,
    int nlen
) {
    if (!nlen)
        return NULL;

    char * hp = haystack;
    
    printf("hp: %p\n", haystack);
    printf("got to here\n");
    printf("pointer + 1: %p\n", hp + 1);
    printf("pointer + 2: %p\n", hp + 2);
    printf("*(pointer): %c\n", *haystack);
    printf("*(pointer + 3): %c\n", *(hp + 3));
    
    int i;
    for (i = 0; i < hlen - nlen + 1; i++) {
        printf("starting iteration %d\n", i);
        if (*(haystack + i) != *needle)
            continue;
        printf("bout to cmp em\n");
        if (memcmpmask(haystack + i, needle, mask, nlen))
            return haystack + i;
    }
    return NULL;
}
