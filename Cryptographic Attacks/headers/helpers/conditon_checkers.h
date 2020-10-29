#ifndef _CONDITION_CHECKERS_H

#define _CONDITION_CHECKERS_H

#define RET_CONDITION_CHECKER(condition, error_code) \
    if (condition) return error_code;

#define GOTO_CONDITION_CHECKER(condition, set_flag, exit_label) \
    if (condition){\
        set_flag = 1;\
        goto exit_label; \
    };

#endif