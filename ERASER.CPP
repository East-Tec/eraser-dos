/*--------------------------------------------------------------------------
 *
 *	Welcome to Eraser for MS-DOS!
 *
 *	Eraser is still under developement.
 *	Current version is 0.1 (pre-official release).
 *
 *	Eraser's main goals are super-speed and super-options.
 *	In its actual form Eraser is already one of the fastest file wipe
 *	utilities in the world !!! (courtesy of assembler code, of course).
 *
 *  Eraser97 for Windows 95 is under developement, too.
 *	Contact the author for more information.
 *
 *	ERASER.CPP: Source code
 *
 *	May 1997
 *
 *	Copyright (c) 1997 Horatiu Tanescu / East-Tec
 *	https://www.east-tec.com
 *
 *	Written and compiled under Borland C++ Version 3.1.
 *
 *  Licensed under the MIT License. See LICENSE file in the project
 *  root for full license information.
 *
 *--------------------------------------------------------------------------
 *
 *	Revision history:
 *      1.0	May 27 1997	Initial coding.
 *
 */

#include <stdio.h>
#include <dos.h>
#include <mem.h>

#pragma inline

const char* errstr_open  = "Error opening file\r\n$";
const char* errstr_write = "Error encountered while overwriting file data\r\n$";

unsigned pBuffer;
unsigned bufSize = 32768U;

#define MAKE_FP(seg, off)	((void _seg *)(seg) + (void near *)(off))
#define memL(seg, off)		*((unsigned long far*)MAKE_FP(seg, off))

unsigned long timer;
unsigned long fulltime = 0UL;
int wipedfiles = 0;

int wipe_standard(const char* filename)
{
  asm	.386

  unsigned handle;
  long size;
  long curBuf = 0;

  timer = memL(0x0040, 0x006C);

  asm	mov	dx, filename
  asm   mov	ax, 0x3D02
  asm	int	0x21
  asm	mov	handle, ax
  asm	jnc	_OpenOK
  asm	mov	dx, errstr_open
  asm   mov	ah, 0x09
  asm	int	0x21
  return (0);

_OpenOK:

  asm	xor	cx, cx
  asm	xor	dx, dx
  asm	mov	bx, handle
  asm	mov	al, 0x02
  asm	mov	ah, 0x42
  asm	int	0x21
  asm	mov	word ptr [size], ax
  asm	mov	word ptr [size + 2], dx

  long     buffers    = size/bufSize;
  unsigned lastBuffer = size%bufSize;

  asm	xor	cx, cx
  asm	xor	dx, dx
  asm	mov	bx, handle
  asm	xor	al, al
  asm	mov	ah, 0x42
  asm	int	0x21

// write loop

  asm	mov	eax, dword ptr [curBuf]
  asm	cmp	eax, dword ptr [buffers]
  asm	jnl	_EndLoopWrite

_WriteLoop:

  asm	mov	cx, bufSize
  asm	mov	bx, handle
  asm	push	ds
  asm	mov	ax, pBuffer
  asm	mov	ds, ax
  asm	xor	dx, dx
  asm	mov	ah, 0x40
  asm	int	0x21
  asm	pop	ds
  asm	jc      _WriteError
  asm   cmp	ax, cx
  asm	jb	_WriteError
  asm	inc	dword ptr [curBuf]
  asm	mov	eax, dword ptr [curBuf]
  asm	cmp	eax, dword ptr [buffers]
  asm	jl	_WriteLoop
  asm	jmp	short _EndLoopWrite

_WriteError:
  asm	mov	dx, errstr_write
  asm   mov	ah, 0x09
  asm	int	0x21
  asm	inc	dword ptr [curBuf]
  asm	mov	eax, dword ptr [curBuf]
  asm	cmp	eax, dword ptr [buffers]
  asm	jl	_WriteLoop

_EndLoopWrite:

// write the last buffer

  asm	mov	cx, lastBuffer
  asm	mov	bx, handle
  asm	push	ds
  asm	mov	ax, pBuffer
  asm	mov	ds, ax
  asm	xor	dx, dx
  asm	mov	ah, 0x40
  asm	int	0x21
  asm	pop	ds
  asm	jc      _WriteError2
  asm   cmp	ax, cx
  asm	jb	_WriteError2
  asm	jmp	short	_EndLastWrite

_WriteError2:
  asm	mov	dx, errstr_write
  asm   mov	ah, 0x09
  asm	int	0x21

// finish writting

_EndLastWrite:
  timer = memL(0x0040, 0x006C) - timer;
  return (1);
}

int main(int argc, char *argv[])
{

    printf("Eraser Fast! for MS-DOS => World's fastest freeware file wipe utility! <=\n");
    printf("Version 0.1 (pre-official release)  Copyright (c) 1997 Horatiu Tanescu / East-Tec\n\n");

    if (argc > 1)
    {
        if (allocmem(bufSize / 16, &pBuffer) >= 0)
        {
            printf("Not enough memory\n");
            return (-1);
        }
        _fmemset(MK_FP(pBuffer, 0), 0, bufSize);

        for (int i = 1; i < argc; i++)
        {
            printf("Wiping %s...\n", argv[i]);
            if (wipe_standard(argv[i]))
            {
                printf("Wiping time: %lf second(s)\n", timer / 18.2);
                fulltime += timer;
                wipedfiles++;
            }
        }
        freemem(pBuffer);
        printf("\nWiped %d file(s) in %lf second(s)\n", wipedfiles, fulltime / 18.2);
    }
    else
    {
        printf("Usage: ERASER <filename1> <filename2...>\n\nWildcards are not allowed (yet).\n");
    }

    return (1);
}