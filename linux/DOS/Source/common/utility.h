#if !defined(UTILITY_H)
#define UTILITY_H

#define ERROR_UNIMPLEMENTED(s)  error("Unimplemented: " s)

void sanitise_path(const char* src, char* dst);

#endif