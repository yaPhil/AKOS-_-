#include<stdio.h>

#define BITWISEPRINT(x)                                                                \
{                                                                                      \
    int i, k;                                                                          \
    for(i = (int)sizeof(x) - 1; i >= 0; --i)                                           \
    {                                                                                  \
        for(k = 7; k >= 0; --k)                                                        \
        {                                                                              \
            putc('0' + ((((unsigned char*)&x)[i] & 1<<k)>>k), stdout);                 \
            if(k == 4)                                                                 \
                putc(' ', stdout);                                                     \
        }                                                                              \
        if(i != 0)                                                                     \
        {                                                                              \
            printf(" | ");                                                             \
        }                                                                              \
                                                                                       \
    }                                                                                  \
    putc('\n', stdout);                                                                \
    for(i = (int)sizeof(x) - 1; i >= 0; --i)                                           \
    {                                                                                  \
        for(k = 1; k >= 0; --k)                                                        \
        {                                                                              \
            int hex = (((unsigned char*)&x)[i] & (15<<(4 * k)))>>(4 * k);              \
            printf("   ");                                                             \
            if(k == 0)                                                                 \
				putc(' ', stdout);                                                     \
            if(hex < 10)                                                               \
                putc('0' + hex, stdout);                                               \
            else                                                                       \
                putc('A' + hex - 10, stdout);                                          \
        }                                                                              \
        if(i != 0)                                                                     \
            printf("   ");                                                             \
    }                                                                                  \
    putc('\n', stdout);                                                                \
}                                                                         

int main()
{
        double tmp = 100378;
        BITWISEPRINT(tmp);
        return 0;
}

