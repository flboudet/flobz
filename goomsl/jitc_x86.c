#include "jitc_x86.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define PARAM_INT      1
#define PARAM_FLOAT    2
#define PARAM_REG      3
#define PARAM_dispREG  4
#define PARAM_DISP32   5
#define PARAM_NONE     666

typedef struct {
  int    id;
  int    i;
  double f;
  int    reg;
  int    disp;
} IParam;

struct {
  char *name;
  int   reg;
} RegsName[] = {
  {"eax",EAX}, {"ebx",EBX}, {"ecx",ECX}, {"edx",EDX},
  {"edi",EDI}, {"esi",ESI}, {"ebp",EBP}, {"esp",ESP},
  {"st(0)",0}, {"st(1)",1}, {"st(2)",2}, {"st(3)",3},
  {"st(4)",4}, {"st(5)",5}, {"st(6)",6}, {"st(7)",7},
  {"mm0",0}, {"mm1",1}, {"mm2",2}, {"mm3",3},
  {"mm4",4}, {"mm5",5}, {"mm6",6}, {"mm7",7}, {NULL,0}
};

static void modrm(JitcX86Env *jitc, int opcode, IParam *iparam)
{
  int dest = 0;
  int src  = 1;
  int direction = 0x0;
  unsigned int byte   = 666;
  unsigned int int32  = 0;
  unsigned int need32 = 0;

  if ((iparam[0].id == PARAM_REG) && (iparam[1].id != PARAM_REG)) {
    dest = 1;
    src  = 0;
    direction = 0x02;
  }

  if (iparam[src].id != PARAM_REG) {
    fprintf(stderr, "JITC_x86: Invalid Instruction Parameters: %d %d.\n", iparam[0].id, iparam[1].id);
    exit(1);
  }

  if (iparam[dest].id == PARAM_REG) {
    byte = ((int)JITC_MOD_REG_REG << 6) | (iparam[src].reg << 3) | (iparam[0].reg);
  }

  else if (iparam[dest].id == PARAM_dispREG)
  {
    if (iparam[dest].disp == 0)
      byte = ((int)JITC_MOD_pREG_REG << 6) | (iparam[src].reg << 3) | (iparam[dest].reg);
  }

  else if (iparam[dest].id == PARAM_DISP32)
  {
    byte   = ((int)JITC_MOD_pREG_REG << 6) | (iparam[src].reg << 3) | JITC_RM_DISP32;
    need32 = 1;
    int32  = iparam[dest].disp;
  }

  if (byte == 666) {
    fprintf(stderr, "JITC_x86: Invalid Instruction Parameters: %d %d.\n", iparam[0].id, iparam[1].id);
    exit(1);
  }
  else {
    JITC_ADD_UCHAR(jitc, opcode + direction);
    JITC_ADD_UCHAR(jitc, byte);
    if (need32)
      JITC_ADD_UINT(jitc, int32);
  }
}

/* 1 byte encoded opcode including register... imm32 parameter */
#define INSTR_1bReg_IMM32(opcode,dest,src) { \
      JITC_ADD_UCHAR(jitc, opcode + iparam[dest].reg); \
      JITC_ADD_UINT (jitc, (int)iparam[src].i); }

/**
 * Check all kind of known instruction... perform special optimisations..
 */
static void jitc_add_op(JitcX86Env *jitc, char *op, IParam *iparam, int nbParams)
{
  if (strcmp(op,"mov") == 0)
  {
    if ((iparam[0].id == PARAM_REG) && (iparam[1].id == PARAM_INT)) {
      INSTR_1bReg_IMM32(0xb8,0,1);
    }
    else if ((iparam[0].id == PARAM_DISP32) && (iparam[1].id == PARAM_INT)) {
      JITC_ADD_UCHAR(jitc, 0xc7);
      JITC_MODRM(jitc, 0x00, 0x00, 0x05);
      JITC_ADD_UINT(jitc, iparam[0].disp);
      JITC_ADD_UINT(jitc, iparam[1].i);
    }
    else
      modrm(jitc, 0x89, iparam);
  }
  else if (strcmp(op,"add") == 0)
  {
    if ((iparam[0].id == PARAM_REG) && (iparam[1].id == PARAM_INT)) {
      if (iparam[0].reg == EAX) {
        JITC_ADD_UCHAR(jitc, 0x05);
        JITC_ADD_UINT(jitc,  iparam[1].i);
      }
      else {
        JITC_ADD_UCHAR(jitc, 0x81);
        JITC_MODRM(jitc,     0x03, 0x00, iparam[0].reg);
        JITC_ADD_UINT(jitc,  iparam[1].i);
      }
    }
    else {
      modrm(jitc, 0x01, iparam);
    }
  }
  else {
    fprintf(stderr, "JITC_x86: Invalid Operation\n");
    exit(1);
  }
}

/**
 * Adds a new instruction to the just in time compiled function
 */
void jitc_add(JitcX86Env *jitc, const char *_instr, ...)
{
  char instr[256];
  char *op;
  char *sparam[16]; int nbParam=0; int i;
  IParam iparam[16];
  va_list ap;
  strcpy(instr,_instr);
  op = strtok(instr, " ,");
  if (!op) return;

  /* decoupage en tokens */
  while ((sparam[nbParam] = strtok(NULL, " ,")) != NULL) if (strlen(sparam[nbParam])>0) nbParam++;

  /* Reconnaissance des parametres */
  va_start(ap, _instr);
  for (i=0;i<nbParam;++i)
  {
    int r;
    char regname[256];
    iparam[i].id = PARAM_NONE;
    if (strcmp(sparam[i], "$d") == 0) {
      iparam[i].id = PARAM_INT;
      iparam[i].i  = va_arg(ap, int);
    }
    else if (strcmp(sparam[i], "$f") == 0) {
      iparam[i].id = PARAM_FLOAT;
      iparam[i].f  = va_arg(ap, double);
    }
    else if (strcmp(sparam[i], "[$d]") == 0) {
      iparam[i].id   = PARAM_DISP32;
      iparam[i].disp = va_arg(ap, int);
    }
    else
    for (r=0;RegsName[r].name;r++) {
      if (strcmp(sparam[i], RegsName[r].name) == 0) {
        iparam[i].id  = PARAM_REG;
        iparam[i].reg = RegsName[r].reg;
      }
      if (sscanf(sparam[i], "$d[%s]", regname) > 0) {
        if (strcmp(regname, RegsName[r].name) == 0) {
          iparam[i].id   = PARAM_dispREG;
          iparam[i].reg  = RegsName[r].reg;
          iparam[i].disp = va_arg(ap, int);
        }
      }
      if (sscanf(sparam[i], "[%3s]", regname) > 0) {
        if (strcmp(regname, RegsName[r].name) == 0) {
          iparam[i].id   = PARAM_dispREG;
          iparam[i].reg  = RegsName[r].reg;
          iparam[i].disp = 0;
        }
      }
    }
    if (iparam[i].id == PARAM_NONE) {
      fprintf(stderr, "JITC_x86: Unrecognized parameter '%s'\n", sparam[i]);
      exit(1);
    }
  }
  va_end(ap);

  jitc_add_op(jitc, op, &(iparam[0]), nbParam);
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
