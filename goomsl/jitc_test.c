#include "jitc_x86.h"
#include <stdio.h>

int main(int c, char **v)
{
    int i;
    int j;
    JitcX86Env *jitc = jitc_x86_env_new(0xffff);
    JitcFunc    func = jitc_prepare_func(jitc);

    JITC_LOAD_REG_IMM32(jitc,EAX,&i);
    JITC_LOAD_REG_IMM32(jitc,EBX,1);
    JITC_LOAD_pREG_REG(jitc,EAX,EBX);
    JITC_FLD_pIMM32(jitc,&i);
    JITC_FSTP_pIMM32(jitc,&j);
    
    jitc_validate_func(jitc);
    func();

    printf("i = %d\n", i);
    
    jitc_x86_delete(jitc);
    return 0;
}
