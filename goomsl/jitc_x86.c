#include "jitc_x86.h"

void jitc_add(JitcX86Env *jitc, const char *_instr, ...)
{
  char instr[256];
  char *op, *tok;
  char *param[16]; int nbParam=0;
  strcpy(instr,_instr);
  op = strtok(instr, " ");
  if (!op) return;
  while (param[nbParam++] = strtok(NULL, " "));
  /* TO BE CONTINUED */
}

JitcX86Env *jitc_x86_env_new(int memory_size) {

    JitcX86Env *jitc = (JitcX86Env*)malloc(sizeof(JitcX86Env));
    jitc->_memory = (unsigned char*)malloc(memory_size);
    jitc->used    = 0;
    jitc->memory  = (unsigned char*)((int)jitc->_memory + (32-((int)jitc->_memory)%32)%32);

    jitc->nbUsedLabel  = 0;
    jitc->nbKnownLabel = 0;

    jitc->usedLabel  = (LabelAddr*)malloc(sizeof(LabelAddr) * JITC_MAXLABEL);
    jitc->knownLabel = (LabelAddr*)malloc(sizeof(LabelAddr) * JITC_MAXLABEL);
    
    return jitc;
}

void jitc_x86_delete(JitcX86Env *jitc) {
    
    free(jitc->usedLabel);
    free(jitc->knownLabel);
    free(jitc->_memory);
    free(jitc);
}

JitcFunc jitc_prepare_func(JitcX86Env *jitc) {

    JitcFunc ptr = 0;
    jitc->used = (32 - jitc->used%32)%32;
    ptr = (JitcFunc)&(jitc->memory[jitc->used]);

    /* save the state */
    JITC_PUSH_REG(jitc,EBP);
    JITC_LOAD_REG_REG(jitc,EBP,ESP);
    JITC_SUB_REG_IMM8(jitc,ESP,8);
    JITC_PUSH_ALL(jitc);
    return ptr;
}

void jitc_validate_func(JitcX86Env *jitc) {

    /* restore the state */
    JITC_POP_ALL(jitc);
    JITC_LEAVE(jitc);
    JITC_RETURN_FUNCTION(jitc);
    jitc_resolve_labels(jitc);
}

void jitc_add_used_label(JitcX86Env *jitc, char *label, int where) {

    strncpy(jitc->usedLabel[jitc->nbUsedLabel].label, label, JITC_LABEL_SIZE);
    jitc->usedLabel[jitc->nbUsedLabel].address = where;
    jitc->nbUsedLabel++;
}

void jitc_add_known_label(JitcX86Env *jitc, char *label, int where) {

    strncpy(jitc->knownLabel[jitc->nbKnownLabel].label, label, JITC_LABEL_SIZE);
    jitc->knownLabel[jitc->nbKnownLabel].address = where;
    jitc->nbKnownLabel++;
}

void jitc_resolve_labels(JitcX86Env *jitc) {

    int i,j;
    for (i=jitc->nbUsedLabel;i-->0;) {

        LabelAddr used = jitc->usedLabel[i];
        for (j=jitc->nbKnownLabel;j-->0;) {

            LabelAddr known = jitc->knownLabel[j];
            if (strcmp(known.label, used.label) == 0) {
                int *offset = (int*)&(jitc->memory[used.address]);
                *offset = known.address - used.address - 4; /* always using long offset */
                break;
            }
        }
    }
    jitc->nbUsedLabel = jitc->nbKnownLabel = 0;
}
