/* Name: spminterface.h
 * Project: USBaspLoader
 * Author: Stephan Baerwolf
 * Creation Date: 2012-08-01
 * Copyright: (c) 2012 by Stephan Baerwolf
 * License: GNU GPL v2 (see License.txt)
 */

#ifndef SPMINTERFACE_H_f70ba6adf7624275947e859bdbff0599
#define SPMINTERFACE_H_f70ba6adf7624275947e859bdbff0599

/*
 * spminterface.h offers a lightweight interface by inserting
 * an small machine-subroutine into the bootsection. (right 
 * after the interrupt-vector-table)
 * This subroutine can be called by normal code in order to 
 * enable it to program the flash (and depending on BLB11-lockbit
 * also the bootsection itself). Since SPM-calls from RWW-sections
 * will fail to work. The Routine will be called "bootloader__do_spm".
 * Its principle assembler-code is depicted below (real code is a
 * little machinedependen).
 * Interfaces will be the 8-bit registers r10..r13, for details 
 * also see below. As also the pageaddress-registes (Z and rampZ)
 * are interfaced via different registers, it is possible to call
 * this routine via indirect call (icall).
 * Traditionally it is also possible to rcall, therefore you can
 * define "bootloader__do_spm" for your normal code via defsym at 
 * linking time.
 * Example for an atmega8: "-Wl,--defsym=transfer_point=0x1826"
 * (since BOOTLOADER_ADDRESS is 0x1800 and there are 
 * 2x19 = 38 = 0x26 byte for interrupts)
 * 

bootloader__do_spm:
;disable interrupts (if enabled) before calling!
;you may also want to disable wdt, since this routine may busy-loop
;==================================================================
;-->INPUT:
;spmcr (spmcrval determines SPM action) will be register:	r18
;MCU dependend RA(MPZ should be transfered within register:	r11
;lo8(Z) should be transfered within register:			r12
;hi8(Z) should be transfered within register:			r13
;( as definition of SPM low8bit of dataword are stored within	r0 )
;( as definition of SPM hi8bit  of dataword are stored within	r1 )

;<-->USED/CHANGED:
;temp0 will be register:					r11
;temp1 will be register:					r12
;temp2 will be register:					r13
;spmcrval (r18) may also be changed due to rww reenable-phase	r18
;Z (r31:r30) wil be changed during operation

;<--OUT:
;==================================================================
; TODO: waitA and waitB could be merged to subroutine saving 2 opc
;==================================================================

;load pageaddress (Z) from (r11:)r13:12 since it may was used for icall
mov	rampZ,	r11
mov	r30,	r12
mov	r31,	r13

waitA:			;check for pending SPM complete
in	temp0, SPMCR
sbrc	temp0, SPMEN
rjmp	waitA

out	SPMCR, spmcrval	;SPM timed sequence
spm

waitB:			;check for previous SPM complete
in	temp0, SPMCR
sbrc	temp0, SPMEN
rjmp	waitB

;avoid crash of userapplication
ldi	spmcrval, ((1<<RWWSRE) | (1<<SPMEN)) 
in	temp0,	  SPMCR
sbrc	temp0,	  RWWSB
rjmp	waitA

ret

*
*/ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#ifndef BOOTLOADER_ADDRESS
// this header is the interface for user-code




#else /*ifndef BOOTLOADER_ADDRESS*/
// this header is used directly within bootloader_do_spm
#include "bootloaderconfig.h"

#if HAVE_SPMINTEREFACE


/*
 * insert architecture dependend "bootloader_do_spm"-code
 * 
 * try to make this array as big as possible
 * (so bootloader always uses 2kbytes flash)
 */
#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega8HVA__)
//assume  SPMCR==0x37, SPMEN==0x0, RWWSRE=0x4, RWWSB=0x6
const uint16_t bootloader__do_spm[19] PROGMEM = {0x0000, 0x2dec, 0x2dfd, 0xb6b7, 0xfcb0, 0xcffd, 0xbf27, 0x95e8, 0xb6b7,
						 0xfcb0, 0xcffd, 0xe121, 0xb6b7, 0xfcb6, 0xcff4, 0x9508, 0xFFFF, 0xFFFF,
						 0xFFFF};
/*
00001826 <bootloader__do_spm>:
    1826:	00 00       	nop
    1828:	ec 2d       	mov	r30, r12
    182a:	fd 2d       	mov	r31, r13

0000182c <waitA>:
    182c:	b7 b6       	in	r11, 0x37	; 55
    182e:	b0 fc       	sbrc	r11, 0
    1830:	fd cf       	rjmp	.-6      	; 0x182c <waitA>
    1832:	27 bf       	out	0x37, r18	; 55
    1834:	e8 95       	spm

00001836 <waitB>:
    1836:	b7 b6       	in	r11, 0x37	; 55
    1838:	b0 fc       	sbrc	r11, 0
    183a:	fd cf       	rjmp	.-6      	; 0x1836 <waitB>
    183c:	21 e1       	ldi	r18, 0x11	; 17
    183e:	b7 b6       	in	r11, 0x37	; 55
    1840:	b6 fc       	sbrc	r11, 6
    1842:	f4 cf       	rjmp	.-24     	; 0x182c <waitA>
    1844:	08 95       	ret
*/





#elif defined (__AVR_ATmega48__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__)
//assume  SPMCR:=SPMCSR==0x37, SPMEN:=SELFPRGEN==0x0, RWWSRE=0x4, RWWSB=0x6
const uint16_t bootloader__do_spm[19] PROGMEM = {0x0000, 0x2dec, 0x2dfd, 0xb6b7, 0xfcb0, 0xcffd, 0xbf27, 0x95e8, 0xb6b7,
						 0xfcb0, 0xcffd, 0xe121, 0xb6b7, 0xfcb6, 0xcff4, 0x9508, 0xFFFF, 0xFFFF,
						 0xFFFF};
/*
00001826 <bootloader__do_spm>:
    1826:	00 00       	nop
    1828:	ec 2d       	mov	r30, r12
    182a:	fd 2d       	mov	r31, r13

0000182c <waitA>:
    182c:	b7 b6       	in	r11, 0x37	; 55
    182e:	b0 fc       	sbrc	r11, 0
    1830:	fd cf       	rjmp	.-6      	; 0x182c <waitA>
    1832:	27 bf       	out	0x37, r18	; 55
    1834:	e8 95       	spm

00001836 <waitB>:
    1836:	b7 b6       	in	r11, 0x37	; 55
    1838:	b0 fc       	sbrc	r11, 0
    183a:	fd cf       	rjmp	.-6      	; 0x1836 <waitB>
    183c:	21 e1       	ldi	r18, 0x11	; 17
    183e:	b7 b6       	in	r11, 0x37	; 55
    1840:	b6 fc       	sbrc	r11, 6
    1842:	f4 cf       	rjmp	.-24     	; 0x182c <waitA>
    1844:	08 95       	ret
*/





#elif defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
//assume  SPMCR:=SPMCSR==0x37, SPMEN:=SELFPRGEN==0x0, RWWSRE=0x4, RWWSB=0x6
const uint16_t bootloader__do_spm[19] PROGMEM = {0x0000, 0x2dec, 0x2dfd, 0xb6b7, 0xfcb0, 0xcffd, 0xbf27, 0x95e8, 0xb6b7,
						 0xfcb0, 0xcffd, 0xe121, 0xb6b7, 0xfcb6, 0xcff4, 0x9508, 0xFFFF, 0xFFFF,
						 0xFFFF};
/*
00001826 <bootloader__do_spm>:
    1826:	00 00       	nop
    1828:	ec 2d       	mov	r30, r12
    182a:	fd 2d       	mov	r31, r13

0000182c <waitA>:
    182c:	b7 b6       	in	r11, 0x37	; 55
    182e:	b0 fc       	sbrc	r11, 0
    1830:	fd cf       	rjmp	.-6      	; 0x182c <waitA>
    1832:	27 bf       	out	0x37, r18	; 55
    1834:	e8 95       	spm

00001836 <waitB>:
    1836:	b7 b6       	in	r11, 0x37	; 55
    1838:	b0 fc       	sbrc	r11, 0
    183a:	fd cf       	rjmp	.-6      	; 0x1836 <waitB>
    183c:	21 e1       	ldi	r18, 0x11	; 17
    183e:	b7 b6       	in	r11, 0x37	; 55
    1840:	b6 fc       	sbrc	r11, 6
    1842:	f4 cf       	rjmp	.-24     	; 0x182c <waitA>
    1844:	08 95       	ret
*/





#elif defined (__AVR_ATmega164A__) || defined (__AVR_ATmega164PA__) || defined (__AVR_ATmega324A__) || defined (__AVR_ATmega324PA__) || defined (__AVR_ATmega644A__) || defined (__AVR_ATmega644PA__) || defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
//assume  SPMCR:=SPCSR==0x37, SPMEN==0x0, RWWSRE=0x4, RWWSB=0x6
const uint16_t bootloader__do_spm[19] PROGMEM = {0x0000, 0x2dec, 0x2dfd, 0xb6b7, 0xfcb0, 0xcffd, 0xbf27, 0x95e8, 0xb6b7,
						 0xfcb0, 0xcffd, 0xe121, 0xb6b7, 0xfcb6, 0xcff4, 0x9508, 0xFFFF, 0xFFFF,
						 0xFFFF};
/*
00001826 <bootloader__do_spm>:
    1826:	00 00       	nop
    1828:	ec 2d       	mov	r30, r12
    182a:	fd 2d       	mov	r31, r13

0000182c <waitA>:
    182c:	b7 b6       	in	r11, 0x37	; 55
    182e:	b0 fc       	sbrc	r11, 0
    1830:	fd cf       	rjmp	.-6      	; 0x182c <waitA>
    1832:	27 bf       	out	0x37, r18	; 55
    1834:	e8 95       	spm

00001836 <waitB>:
    1836:	b7 b6       	in	r11, 0x37	; 55
    1838:	b0 fc       	sbrc	r11, 0
    183a:	fd cf       	rjmp	.-6      	; 0x1836 <waitB>
    183c:	21 e1       	ldi	r18, 0x11	; 17
    183e:	b7 b6       	in	r11, 0x37	; 55
    1840:	b6 fc       	sbrc	r11, 6
    1842:	f4 cf       	rjmp	.-24     	; 0x182c <waitA>
    1844:	08 95       	ret
*/





#else
  #error "bootloader__do_spm has to be adapted, since there is no architecture code, yet"
#endif  


#endif
#endif /*ifdef BOOTLOADER_ADDRESS*/

#endif
						 