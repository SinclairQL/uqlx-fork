#ifndef QL68000_H
#define QL68000_H

/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef STATIC
#define STATIC
#endif

/* needed for ntoh? functions */
#include <arpa/inet.h>
#include <stdint.h>

#undef QM_BIG_ENDIAN

#if defined(i386) || defined(i486) || defined(i586) || defined(i686) || defined(__i386__) ||defined(__i586__) || defined(__i686__)
#ifndef __i486__
#define __i486__
#endif
#endif

#ifdef USE_BUILTIN_EXPECT
#define likely(exp)   __builtin_expect((exp),1)
#define unlikely(exp) __builtin_expect((exp),0)
#else
#define likely(exp) (exp)
#define unlikely(exp) (exp)
#endif

/* 
 * QLtypes.h has been "inlined" here 'cause some typedefs are 
 * CPU dependent  now 
 */

#define true 1
#define false 0

typedef int8_t w8;
typedef int16_t w16;
typedef int32_t w32;
typedef uint8_t uw8;
typedef uint16_t uw16;
typedef uint32_t uw32;

/* should probably apply for some other RISC chips as well */
#if defined(SPARC) || defined(HPPA)
typedef int Cond;  /* load/store CPUs deal badly with char arithmetic */
#else
typedef unsigned char Cond;
#endif

/* use the wide type because otherwise gcc will promote *every*
 * arg and return value */
#if 0
typedef int ashort;
typedef signed int aw8,aw16,aw32;
typedef signed int rw8,rw16,rw32;
typedef unsigned int ruw8,ruw16,ruw32;
typedef unsigned int gshort;
typedef unsigned int rCond;
#else
typedef short ashort;
typedef w8 aw8,rw8,ruw8;
typedef w16 aw16,rw16,ruw16;
typedef w32 aw32,rw32,ruw32;
typedef unsigned short gshort;
typedef unsigned char rCond;
#endif

/* shindex is for quants that would fit into 16 bits but are used as index */
/* somewhere, most CPUs dont like 16 bit indexes  */
//#if defined(__i486__) || defined(SPARC) || defined(HPPA)
/* probably easier to tell the one which does */
#ifndef __mc68000
typedef int shindex;
#else
typedef short shindex;
#endif

/* must hold 8bit, used for comparison */
#ifdef m68k
typedef char bctype;
#else
typedef int bctype;  
#endif

typedef void* Ptr;     /* non ANSI, but convenient... */

struct qFloat
{
   uw16    exp;
   uw32    mant;
};

/* end QLtypes.h */
#define FIX_INS

extern void (**table)(void);

typedef int OSErr;
extern char *release;
extern char *uqlx_version;

#ifndef NULL
#define NULL (void *)0
#endif

extern int gKeyDown, shiftKey, controlKey, optionKey, alphaLock, altKey;

#ifdef m68k
#define G_reg
#endif

#ifndef G_reg 
extern w32              reg[16];
#endif
extern w32              usp,ssp;


#ifdef SPARC
#ifndef BROKEN_GREGS
register uw16  *pc asm("g5");
register gshort  code asm("g6");
register int   nInst asm("g7");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef m68k
// most m68k users will have my patched gcc
register w32     *reg asm("%a5");
register uw16    *pc asm("%a4");
register gshort  code asm("%d6");
register int     nInst asm("%d7");
#define GREGS
/*#define ASSGN_CODE(val) ((uw16)code=(uw16)val)*/
/* code is fixed to d6, MSW never used - set only lower 16 bit */
#define ASSGN_CODE(val) ({asm("move.w  %0,%%d6 \n" :: "m" (val) : "d6"); code;})
#endif

#ifdef __i486__
// compiler gets internal errors
#if 0
//register w32     *reg asm("ebp");
//register uw16    *pc asm("esi");
register uw16    *pc asm("ebx");
//register gshort  code asm("ebx");
register gshort  code asm("ebp");
register int     nInst asm("edi");
#define GREGS
#else
#define ASSGN_486() ({gshort tmp;                    \
                          asm("movzwl %1,%0 \n"          \
			      "rolw   $8,%0 \n"          \
                               : "=r" (tmp) : "m" (*pc++) : "cc"); \
                          code=tmp; })
#endif
#endif

#ifdef HPPA
#ifndef BROKEN_GREGS
register uw16    *pc asm("r5");
register gshort  code asm("r6");
register int     nInst asm("r7");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef MIPS
#ifndef BROKEN_GREGS
register uw16    *pc asm("$23");
register gshort  code asm("$22");
register int     nInst asm("$21");
#define GREGS
#define MANYREGS
#endif
#endif

#if defined(PPC)
#ifndef BROKEN_GREGS
register uw16    *pc asm("r14");
register gshort  code asm("r15");
register int     nInst asm("r16");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef ALPHA
#define HUGE_POINTER
#define MANYREGS
#endif

#ifndef GREGS
extern uw16 *pc;
extern gshort code;
extern int nInst;    /* dangerous - it is 'volatile' to some extent */
#endif

#ifdef DARWIN
// apple sux // really?? ;)
#define ASSGN_CODE(val) (code = (unsigned)(unsigned short)val)
#endif

#if defined(__x86_64__) || defined(__aarch64__)
#define HUGE_POINTER
#endif

#ifndef ASSGN_CODE
#define ASSGN_CODE(val) (code = val & 0xffff)
#endif

#define gPC  pc
#define gUSP usp
#define gSSP ssp

/* define the maximum amount of QL addressable memory, will wrap */
#define ADDR_MASK    0xffffff
#define ADDR_MASK_E  0xfffffe

#ifdef TRACE
extern uw16 *tracelo; 
extern uw16 *tracehi;
extern void CheckTrace(void);
extern void TraceInit(void);
extern void DoTrace(void);
#endif

extern int   nInst2;
extern Cond             trace,supervisor,xflag,negative,zero,overflow,carry;
extern char             iMask;
extern Cond             stopped;
extern volatile char    pendingInterrupt;

/* reduce register pressure */
#define NEW_AREG

#ifdef NEW_AREG
#define   aReg  (reg+8)
#define   m68k_sp    (aReg+7)
extern w32      *g_reg;
#else
extern w32              *sp;
extern w32              *aReg;
#endif

#define SETREG16(_ra_,_val_) ({w16 *dn; dn=(w16*)(RWO+(char*)&_ra_); *dn=_val_;})

#ifdef ZEROMAP
#define theROM          ((w32*)0)
#else
extern w32              *theROM;
#endif

//extern w32              *ramTop;
extern w32              RTOP;
extern short            exception;
extern w32              badAddress;
extern w16              readOrWrite;
extern w32              dummy;
extern Ptr              dest;
#if 1
#define MEA_DISP 1
#define MEA_HW 2
extern Cond mea_acc;
#else  
#ifndef VM_SCR
extern Cond             isDisplay;
#endif
extern Cond             isHW;
#endif
extern w32              lastAddr;
extern volatile Cond    extraFlag;
extern volatile w8      intReg;
extern volatile w8      theInt;

extern bctype           *RamMap;
extern char             dispScreen;
extern Cond             dispMode;
extern Cond             badCodeAddress;

#define RM_SHIFT pageshift

extern int MPROTECT(void *,long, int);

extern int schedCount;
#define INCR_SC() {schedCount++;}
#define DECR_SC() {if (schedCount>0) schedCount--;}

extern w32  displayFrom;
extern w32  displayTo;

extern int isMinerva;

#ifndef vml
#define vml static
#endif

/* pass args for selected fns through registers */
#if defined(__i486__) && (!defined(NO_REGP))
#define REGP1 __attribute__ ((regparm(3)))
#define REGP2 __attribute__ ((regparm(3)))
#ifndef NO_AREGP  /* sometimes reason actually slower ??*/
#if __GNUC__>2 || ( __GNUC__==2 && __GNUC_MINOR__>=8) 
/*#define USE_AREGP*/  /* requires explicit setting */
#endif
#endif
#else
#define REGP1
#define REGP2
#endif

/* same, but for vectored functions gets rather ugly and dangerous,
 * known to break 2.7.2.1, 2.7.2.3 and posibly all versions earlier 2.8.1 */
#ifdef USE_AREGP
/* WARN: exactly matching prototypes assumed */
#define AREGP __attribute__ ((regparm(3)))
#define ARCALL(_farray_,_index_,_args_...)  \
                  ({ typeof(_farray_[0]) AREGP _fx_ = _farray_[_index_]; \
                     (*_fx_)(_args_);})
#else
#define AREGP
#define ARCALL(_farray_,_index_,_args_...)  \
                  ( _farray_[_index_](_args_) )
#endif


extern rw32      (*GetEA[8])(ashort) /*AREGP*/;      /**/
extern rw8       (*GetFromEA_b[8])(void);
extern rw16      (*GetFromEA_w[8])(void);
extern rw32      (*GetFromEA_l[8])(void);
extern void (*PutToEA_b[8])(ashort,aw8)  /*AREGP*/;  /**/
extern void (*PutToEA_w[8])(ashort,aw16) /*AREGP*/; /**/
extern void (*PutToEA_l[8])(ashort,aw32) /*AREGP*/; /**/
extern Cond (*ConditionTrue[16])(void) ;

rw8 ReadHWByte(aw32 )  REGP1;
void WriteHWByte(aw32, aw8) REGP1;
STATIC rw8 ReadByte(aw32 ) REGP1;
STATIC rw16 ReadWord(aw32 ) REGP1;
STATIC rw32 ReadLong(aw32 ) REGP1;
STATIC void WriteByte(aw32 ,aw8 ) REGP2;
STATIC void WriteWord(aw32 ,aw16 ) REGP2;
STATIC void WriteLong(aw32 ,aw32 ) REGP2;

rw16 GetSR(void);
void PutSR(aw16 ) REGP1;
rw16 BusErrorCode(aw16 ) REGP1;
void SetPC(w32 )  REGP1;
void SetPCX(int )  REGP1;

STATIC rw8 ModifyAtEA_b(ashort ,ashort )  REGP2;
STATIC rw16 ModifyAtEA_w(ashort ,ashort ) REGP2;
STATIC rw32 ModifyAtEA_l(ashort ,ashort ) REGP2;
STATIC void RewriteEA_b(aw8 )  REGP1;
STATIC void RewriteEA_w(aw16 ) REGP1;
STATIC void RewriteEA_l(aw32 ) REGP1;

void FrameInt(void);
void WriteInt(aw8) REGP1; 

void ExceptionIn(char) REGP1;
void ExceptionOut(void);
void UpdateNowRegisters(void);

Cond IPC_Command(void);
void WriteMdvControl(aw8) REGP1;

#define LongFromByte(__d__) ((w32)((w8)(__d__)))
#define LongFromWord(__d__) ((w32)((w16)(__d__)))
#define WordFromByte(__d__) ((w16)((w8)(__d__)))

#define nil (void*)0
/*int Error;*/
#define gError Error


extern long pagesize;
extern int pageshift;
extern char *scrModTable;
extern int sct_size;
extern char * oldscr;


#ifdef NO_PSHIFT
#define PAGEI(_x_)  ((int)(_x_)/pagesize)
#define PAGEX(_x_)  ((int)(_x_)*pagesize)
#else
#define PAGEI(_x_)  ((int)(_x_)>>pageshift)
#define PAGEX(_x_)  ((int)(_x_)<<pageshift)
#endif
#include "cond.h"
#include "trace.h"
#include "iexl.h"
#include "QDOS.h"

/* QL memory types */
#define QX_NONE      0
#define QX_ROM       1
#define QX_QXM       1
#define QX_RAM       3
#define QX_SCR       7
#define QX_IO        8

#ifdef SPARC
#define HOST_ALIGN 4
#define QM_BIG_ENDIAN
#endif

#ifdef sgi
#define HOST_ALIGN 4
#define QM_BIG_ENDIAN
#endif

#if defined(HPPA) 
#define QM_BIG_ENDIAN
#define HOST_ALIGN 4
#endif

#if defined(m68k) || defined(m68000) || defined(_m68k_) || defined(mc68000)
#define QM_BIG_ENDIAN
#define HOST_ALIGN 1   /* ignore 68000 */
#endif

#ifdef PPC
#define QM_BIG_ENDIAN
#define HOST_ALIGN 1
#endif

#define WB(_addr_,_val_)(*(uw8*)(_addr_)=(_val_))
#define RB(_addr_) (*(uw8 *)(_addr_))




#ifdef QM_BIG_ENDIAN


static inline ruw16 q2hw(uw16 val)
{
  return val;
}
static inline ruw32 q2hl(uw32 val)
{
  return val;
}

static inline ruw16 h2qw(uw16 v)
{
  return v;
}
static inline ruw32 h2ql(uw32 v)
{
  return v;
}

#if (HOST_ALIGN>2)
static inline void _wl_(w32 *addr,w32 d)
{
  if ((long)addr&2)
    {
      *(uw16*)addr=d>>16;
      *(((uw16*)addr)+1)=d&0xffff;
    }
  else
    *addr=d;
}
static inline uw32 _rl_(w32 *addr)
{
  if ((long)addr&2)
    return (w32) (((*(uw16*)addr)<<16)|((*(uw16*)(2+(long)(addr)))));
  else
    return *addr;
}
#else /* HOST_ALIGN <=2 */
#define WL(_addr_,_val_) (*(uw32*)(_addr_)=(_val_))
#define RL(_addr_) (*(uw32*)(_addr_))
#endif

#else
/* little endian stuff comes here */

static inline ruw16  q2hw(uw16 val)
{
  return ((val&0xff)<<8)|((val>>8)&0xff);
}
static inline ruw32 q2hl(uw32 val)
{
  return ((val&0xff)<<24)|((val&0xff00)<<8)|((val>>8)&0xff00)|((val>>24)&0xff);
}
static inline ruw16 h2qw(uw16 v)
{
  return ((v&0xff)<<8)|((v>>8)&0xff);
}
static inline ruw32 h2ql(uw32 v)
{
  return ((v&0xff)<<24)|((v&0xff00)<<8)|((v>>8)&0xff00)|((v>>24)&0xff);
}

static inline ruw16 _rw_(uw16 *s)
{
	return ntohs(*s);
}
static inline ruw32 _rl_(uw32 *s)
{
	return ntohl(*s);
}

static inline void _ww_(uw16 *d, uw16 v)
{
	*d = htons(v);
}

static inline void _wl_(uw32 *d, uw32 v)
{
   *d = htonl(v);
}

#endif /* QM_BIG_ENDIAN */

#ifndef RW
#define RW(_r_a)         _rw_((void *)(_r_a))
#endif
#ifndef RL
#define RL(_r_al)        _rl_((void *)(_r_al))
#endif
#ifndef WW
#define WW(_r_a,_r_v)    _ww_((void *)(_r_a), (_r_v))
#endif
#ifndef WL
#define WL(_r_al,_r_vl)  _wl_((void *)(_r_al),(_r_vl))
#endif

#define dbginfo(format,args...) {printf(format, ## args);\
                                 DbgInfo();}

		     
#ifdef QM_BIG_ENDIAN
#define RWO 2
#define UW_RWO 0
#define RBO 3
#define MSB_W  2
#define MSB_L  0
#else
#define RWO 0
#define UW_RWO 2
#define RBO 0
#define MSB_W  1
#define MSB_L  3
#endif

#ifdef HUGE_POINTER
void static inline SET_POINTER(w32*_addr_,void *_val_) 
{
	uint32_t val1 = ((uintptr_t)_val_) >> 32;
	uint32_t val2 = ((uintptr_t)_val_) & 0xFFFFFFFF;

	WL(_addr_,val1);
	WL(_addr_+4,val2);
}
static inline void *GET_POINTER(w32* _addr_)  
{
	uintptr_t val=((((uintptr_t)RL(_addr_))<<32 ) | (uint32_t)RL(_addr_+4));

	return (void *)val;
}
#else
#define SET_POINTER(_addr_,_val_)  (WL(_addr_,(w32)_val_)) 
#define GET_POINTER(_addr_)  ((void *)RL(_addr_))
#endif




extern void QLtrap(int ,int ,int );
extern void QLvector(int , int );


#include "QL_screen.h"

#ifdef QM_BIG_ENDIAN
#define big_endian 1
#else
#define big_endian 0
#endif

extern int script;
extern Cond doTrace;

#define MARK_SCREEN

#define prepChangeMem

extern int verbose;
#define V1 (verbose>0)
#define V2 (verbose>1)
#define V3 (verbose>2)

#include "misdefs.h"

#endif
