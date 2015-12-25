#define OBI_CONCATENATE_IMPL(s1, s2) s1##s2
#define OBI_CONCATENATE(s1, s2) OBI_CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
    #define OBI_ANONYMOUS_VARIABLE(str) OBI_CONCATENATE(str, __COUNTER__)
#else
    #define OBI_ANONYMOUS_VARIABLE(str) OBI_CONCATENATE(str, __COUNTER__)
#endif