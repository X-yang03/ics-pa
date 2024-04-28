#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  

  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  rtl_xor(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t3,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t3);

  rtl_update_ZFSF(&t3,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  rtl_sar(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  rtl_shl(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  rtl_shr(&t2,&id_dest->val,&id_src->val);
  //  if(decoding.seq_eip >= 0x4000000){
  //   printf("========\n");
  //   printf("t : %d\n",t2);
  //   printf("val: %d\n",*(&id_dest->val));
  //   printf("src_val: %d\n",*(&id_src->val));
  //   printf("eax: %d\n",cpu.eax);
  //    printf("----------\n");
  // }
  operand_write(id_dest,&t2);
  // if(decoding.seq_eip >= 0x4000000){
  //   printf("val: %d\n",*(&id_dest->val));
  //   printf("reg: %d\n",*(&id_src->reg));
  //   printf("eax: %d\n",cpu.eax);
  //    printf("----------\n");
  // }
  rtl_update_ZFSF(&t2,id_dest->width);
  // if(decoding.seq_eip >= 0x4000000){
  //   printf("========\n");
  //   printf("ZF : %d\n",cpu.eflags.ZF);
  //   printf("t : %d\n",t2);
  //   printf("val: %d\n",*(&id_dest->val));
  //   printf("eax: %d\n",cpu.eax);
  //    printf("========\n");
  // }
  

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_mv(&t1,&id_dest->val);
  rtl_not(&t1);
  operand_write(id_dest,&t1);


  print_asm_template1(not);
}
