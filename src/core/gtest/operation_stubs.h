/*
 * Copyright (c) 2022-2023 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_OPERATION_STUBS_H)
#define MMB4L_OPERATION_STUBS_H

void (*mock_op_add)() = [](){};
void op_add() { mock_op_add(); }
void op_and() { }
void op_div() { }
void op_divint() { }
void op_equal() { }
void op_exp() { }
void op_gt() { }
void op_gte() { }
void op_inv() { }
void op_invalid() { }
void op_lt() { }
void op_lte() { }
void op_mod() { }
void op_mul() { }
void op_ne() { }
void op_not() { }
void op_or() { }
void op_shiftleft() { }
void op_shiftright() { }
void op_subtract() { }
void op_xor() { }

#endif // #if !defined(MMB4L_OPERATION_STUBS_H)
