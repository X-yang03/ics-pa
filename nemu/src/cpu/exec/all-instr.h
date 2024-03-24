#include "cpu/exec.h"

make_EHelper(mov);
make_EHelper(leave);
make_EHelper(movzx);
make_EHelper(movsx);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(add);
make_EHelper(sub);
make_EHelper(cmp);

make_EHelper(and);
make_EHelper(or);
make_EHelper(xor);
make_EHelper(setcc);

make_EHelper(call);
make_EHelper(ret);
make_EHelper(jmp);

make_EHelper(push);
make_EHelper(pop);
