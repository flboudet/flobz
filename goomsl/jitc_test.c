#include "jitc_x86.h"
#include <stdio.h>

int main(int c, char **v)
{
    int i;
    int j;
    JitcX86Env *jitc = jitc_x86_env_new(0xffff);
    JitcFunc    func = jitc_prepare_func(jitc);

    jitc_add(jitc, "mov ebx,  $d",   1);
    jitc_add(jitc, "mov [$d], ebx",  &i);
    jitc_add(jitc, "mov ecx,  [$d]", &i);
    jitc_add(jitc, "mov [$d], $d",   &i, 0xdeadbeaf);
    jitc_add(jitc, "mov eax,  $d",   1);
    jitc_add(jitc, "add eax,  $d",   2);
    jitc_add(jitc, "add eax,  eax");

    JITC_FLD_pIMM32(jitc,&i);
    JITC_FSTP_pIMM32(jitc,&j);
    
    jitc_validate_func(jitc);
    func();

    printf("i = 0x%08x\n", i);
    
    jitc_x86_delete(jitc);
    return 0;
}
