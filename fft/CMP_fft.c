/*
    Fast real-input integer fft code from Eyal Lebedinsky, public domain 
    release 4 September 1991 (see file gen/fftsm.c which implements the
    interpreted condensed table technique). Original code from Eyal Lebedinsky
    is kept in archive fftgen.tar.gz.

    Changed only minor things: Gave the fft-function arguments to avoid
    global static memory. Removed NEAR and FAR specifications. Changed 
    ultimate rightshift from >>16 to >>12 to gain 16 times as much output
    in squared magnitude (for 1024-point!!). (TODO: change to 32 bit output?)
    Added 0.5 (0x00000800 for 12 bit shift) before rounding squared output.

    Part of ComParser version 1.26, Pieter Suurmond, may 29, 2004.
    Latest version available at: http://kmt.hku.nl/~pieter/SOFT/CMP/
    
    Info about the 1024-point FFT:
    -------------------------------------------------------------------------------
    Reads 1024 signed 16 bit input samples, writes 513 squared output values.          
        0     1     2     3     4     5     6     7     8                             
     Load Store Shift   Add  Mult   Adj                         x[257] >>= 1 (10-0)   
     6714  6714  4977 12804  3586     1     0     0     0                             
    -------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>

#include "CMP_fft_1024.h"      /* Import Lebedinsky's tables. Hopefully, all these */
#include "CMP_fft_2048.h"      /* large tables fit into one single objectfile.     */
#include "CMP_fft_4096.h"

#define ENTRY_TYPE      0x2000
#define INDEX_MASK      (ENTRY_TYPE-1)
#define ENTRY_MASK      (~INDEX_MASK)
#define X(i)            *(short*)(((char*)input)+i)              /* Into argument. */
#define QF(i)           *(short*)(((char*)output)+i)             /* Into global.   */
#define mulf(x,y)       ((short)(((short)(x) * (long)(short)(y)) >> 16))

#define kCMPsqrtShift (12)         /* Originally 16, 12 to gain 4 more bits resolution. */
#define kCMPsqrtRound (0x00000800) /* With 1024 points, squared magnitude from 1 single */
                                   /* band never exceeds 4096 (sometimes 4099!). With   */
                                   /* reduced rightshift of 14 this max becomes 16384.  */
                                   /* A 16 bit hamming-windowed input-sinewave, however,*/
                                   /* only gives a squared output maximum about 4880 !! */
                                   /* The old CMP.audio.c wants it to be 14, for the    */
                                   /* new CMP.audioVD.c we now try 12 now (max=19521).  */

short fft(short* input, short* output, short input_size)
{
    register const short *p;
    short                p0;
    switch (input_size)
        {
        case 1024: p = LebedinskyTable1024; break;
        case 2048: p = LebedinskyTable2048; break;
        case 4096: p = LebedinskyTable4096; break;
        default: return 1; /* Error: invalid input_size. */
        }
    for (;;)
        {
        p0 = *p & INDEX_MASK;
        switch (*(unsigned short *)p & ENTRY_MASK)
            {
            case 1*ENTRY_TYPE:
                {
                X(p0) >>= 1;
                }
                p += 1;
                break;
            case 2*ENTRY_TYPE:
                {
                short t0, t1;
                t0 = X(p0)   >> 1;
                t1 = X(p[1]) >> 1;
                X(p0)   = t0 + t1;
                X(p[1]) = t0 - t1;
                }
                p += 2;
                break;
            case 3*ENTRY_TYPE:
                {
                short t1, t3, t4, tt;

                t1 = X(p0)   >> 1;
                t3 = X(p[1]) >> 1;
                t4 = X(p[2]) >> 1;
                tt      = (t4 + t3) >> 1;
                X(p[2]) = (t4 - t3) >> 1;
                X(p[1]) =  t1 - tt;
                X(p0)   =  t1 + tt;
                }
                p += 3;
                break;
            case 4*ENTRY_TYPE:
                {
                short t1, t2, t3, t4, tt1, tt2;
                t1 = X(p0) >> 2;
                t2 = X(p[1]) >> 1;
                t3 = mulf (X(p[2]), 0x2d41);
                t4 = mulf (X(p[3]), 0x2d41);
                tt1 = t3 + t4;
                tt2 = t3 - t4;
                X(p[3]) =  t2 - tt1;
                X(p[2]) = -t2 - tt1;
                X(p[1]) =  t1 - tt2;
                X(p0)   =  t1 + tt2;
                }
                p += 4;
                break;
            case 5*ENTRY_TYPE:
                {
                short t1, t2, t3, t4, t5, t6;
                t5 = X(p[6]);
                t1 = mulf (t5, p[8]);
                t6 = X(p[2]);
                if (p[14])
                    t6 >>= 1;
                t5 += t6;
                t2 = mulf (t6, p[9]);
                t5 = mulf (t5, p[10]);
                t1 += t5;
                t2 += t5;   /*----*/
                t5 = X(p[7]);
                t3 = mulf (t5, p[11]);
                t6 = X(p[3]);
                if (p[14])
                    t6 >>= 1;
                t5 += t6;
                t4 = mulf (t6, p[12]);
                t5 = mulf (t5, p[13]);
                t3 += t5;
                t4 += t5;   /*----*/
                t5    = t3 - t1;
                t6    =  X(p[1]) >> 1;
                X(p[6]) = -t6 + t5;
                X(p[3]) =  t6 + t5;
                t5    = t2 + t4;
                t6    =  X(p[5]) >> 1;
                X(p[2]) = -t6 + t5;
                X(p[7]) =  t6 + t5;
                t5    = t1 + t3;
                t6    =  X(p0) >> 1;
                X(p[5]) =  t6 - t5;
                X(p0) =  t6 + t5;
                t5    = t2 - t4;
                t6    =  X(p[4]) >> 1;
                X(p[4]) =  t6 - t5;
                X(p[1]) =  t6 + t5;
                }
                p += 15;
                break;
            case 6*ENTRY_TYPE:
                {
                long lt1;
                lt1 = X(p[1]);
                lt1 *= lt1;                       /* ^2. */
                QF(p0) = (short)((kCMPsqrtRound + lt1) >> kCMPsqrtShift);
                }                                 /* Was 16. */
                p += 2;
                break;
            case 7*ENTRY_TYPE:
                {
                long lt1, lt2;
                lt1 = X(p[1]);
                lt1 *= lt1;
                lt2 = X(p[2]);
                lt2 *= lt2;                       /* Re^2 + Im^2. */
                QF(p0) = (short)((kCMPsqrtRound + lt1 + lt2) >> kCMPsqrtShift);
                }                           /* With >>16, max-value was around 4096. */
                p += 3;                     /* With >>14, max is around 16384. */
                break;                      /* With window it is even less. */
            default:
                return 0;
            }
        }
}
