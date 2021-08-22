//  TODO: should 'src' and 'dst' be otherway round to match strcpy() ?
/** Copies 'src' to 'dst' replacing '\' with '/'. */
void sanitise_path(const char* src, char* dst) {
    const char *psrc = src;
    char *pdst = dst;
    for (;;) {
        *pdst = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == 0) break;
        psrc++;
        pdst++;
    }
}
