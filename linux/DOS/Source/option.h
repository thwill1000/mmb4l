// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(OPTION_H)
#define OPTION_H

struct option_s {
    char Tab;
    char Listcase;
    int  Height;
    int  Width;
    int  ProgFlashSize;
    int  Autorun;
};

extern struct option_s Option;

#endif
