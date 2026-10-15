// Golden values use 80-digit trigonometry of exact binary32 inputs.
// The positive input is the observed Final B frame 17460, laser slot 84 rotation.
#include "ZunMath.hpp"
#include <stdio.h>
#include <string.h>
#if defined(__i386__)
#include <fenv.h>
#include <fpu_control.h>
#endif
static float value(unsigned word) { float result; memcpy(&result,&word,4);return result; }
static unsigned bits(float number) { unsigned result;memcpy(&result,&number,4);return result; }
int main() {
#ifdef TH08_TEST_SINGLE_PRECISION
#if defined(__i386__)
 fenv_t environment;
 if (fegetenv(&environment)) return 2;
 environment.__control_word=(environment.__control_word & ~_FPU_EXTENDED)|_FPU_SINGLE;
 if (fesetenv(&environment) || fesetround(FE_TONEAREST)) return 2;
#else
 fputs("Single-precision FPU mode requires i386\n",stderr);return 2;
#endif
#endif
 const unsigned cases[][3]={{0x3f879c1au,0x3f5f411au,0x3efa8c2bu},{0xbf879c1au,0xbf5f411au,0x3efa8c2bu},{0xbf03f921u,0xbefc68dbu,0x3f5ebab5u},{0x402738d2u,0x3f0123e8u,0xbf5d0a4eu},{0xc0350b11u,0xbe9d8d06u,0xbf739411u},{0x3f05acd4u,0x3eff5d92u,0x3f5de2a7u},{0x4003165au,0x3f635f4cu,0xbeeb44d6u},{0x3e510fd9u,0x3e4f9cd1u,0x3f7aaee6u}};
 int errors=0;
 for (unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);++i) {
  float angle=value(cases[i][0]),sine,cosine;sincos(angle,sine,cosine);
  if (bits(sine)!=cases[i][1] || bits(cosine)!=cases[i][2]) {
   fprintf(stderr,"FAIL: sincos angle %08x: expected %08x %08x; observed %08x %08x\n",cases[i][0],cases[i][1],cases[i][2],bits(sine),bits(cosine));++errors;
  }
 }
 if (!errors) puts("PASS: portable sincos rounding contracts");return errors?1:0;
}
