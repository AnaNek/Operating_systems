.386p

descr struc
limit dw 0
base_l dw 0
base_m db 0
attr_1 db 0
attr_2 db 0
base_h db 0
descr ends

intr struc
offs_l dw 0
sel dw 16
cntr db 0
attr db 8Fh
offs_h dw 0
intr ends

PM_seg segment para public 'CODE' use32
    assume cs:PM_seg

    ; Таблица дескрипторов сегметов GDT
  	GDT		label	byte

  	; нулевой дескриптор
  	gdt_null	descr <>

  	; 32-битный 4-гигабайтный сегмент с базой = 0
  	gdt_flatDS	descr <0FFFFh,0,0,92h,11001111b,0>	; 92h = 10010010b

  	; 16-битный 64-килобайтный сегмент кода с базой RM_seg
  	gdt_16bitCS	descr <RM_seg_size-1,0,0,98h,0,0>	; 98h = 10011010b

  	; 32-битный 4-гигабайтный сегмент кода с базой PM_seg
  	gdt_32bitCS	descr <PM_seg_size-1,0,0,98h,01000000b,0>

  	; 32-битный 4-гигабайтный сегмент данных с базой PM_seg
  	gdt_32bitDS	descr <PM_seg_size-1,0,0,92h,01000000b,0>

  	; 32-битный 4-гигабайтный сегмент данных с базой stack_seg
  	gdt_32bitSS	descr <stack_l-1,0,0, 92h, 01000000b,0>

    gdt_screen descr <4095,8000h,0Bh,92h,0,0>

gdt_size = $-GDT
gdtr	df 0

SEL_flatDS equ 001000b
SEL_16bitCS equ 010000b
SEL_32bitCS equ 011000b
SEL_32bitDS equ 100000b
SEL_32bitSS equ 101000b

IDT label byte
    intr 32 dup (<0, SEL_32bitCS,0, 8Eh, 0>)

; дескриптор прерывания от таймера
    int08 intr <0, SEL_32bitCS,0, 8Eh, 0>

; дескриптор прерывания от клавиатуры
    int09 intr	<0, SEL_32bitCS,0, 8Eh, 0>

idt_size = $-IDT
idtr	df 0

idtr_real dw 3FFh, 0, 0

master	db 0					 ; маска прерываний ведущего контроллера
slave	db 0

msg1 db 'In Real Mode now. To move to Protected Mode press any key...$'
msg2 db 'Back Real Mode$'
msg_prot_mode db 'protected mode'
msg_real_back db 'back real mode', 13, 10, '$'
scan2ascii db 0,1Bh,'1','2','3','4','5','6','7','8','9','0','-','=',8
            db ' ', 'q', 'w', 'e', 'r', 't', 'y' , 'u', 'i', 'o', 'p', '[', ']', '$'
            db ' ', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ',', '""', 0
            db '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0

screen_addr dd 960
mark_08h dd 480
mark_mem dd 640
color_08h db 71h
color_09h db 1Eh
time_08h dd 0
out_position	dd 1E0h
tblhex db '0123456789ABCDEF'
at db 1Eh
at2 db 71h

PM_entry:
    mov ax, 48
    mov es, ax
    mov cx, 14
    mov esi, offset msg_prot_mode
    mov di, 60
    mov ah, attr

screen_prot_mode:
    lodsb
    stosw
    loop screen_prot_mode

    mov	ax,SEL_32bitDS
    mov	ds,ax
    mov	ax,SEL_flatDS
    mov	es,ax
    mov	ax,SEL_32bitSS
    mov	ebx,stack_l
    mov	ss,ax
    mov	esp,ebx

    sti

    jmp short $

new_08h:
    push AX
    push BX
    test time_08h, 03h
    jnz skip

    mov bx, SEL_flatDS
    mov es, bx
    mov ebx, mark_08h

    mov ECX, 10
    mov eax, time_08h
print_time:
    xor EDX, EDX
    div ECX
    add EDX, '0'
    mov DH, at
    mov ES:[ebx + 0B8130h], DX
    sub BX, 2
    cmp EAX, 0
    jnz print_time

skip:
    inc time_08h
    mov AL, 20h
    out 20h, AL
    pop BX
    pop AX
    iretd

new_09h:
    push eax
    push ebx
    push es
    push ds

    in al, 60h
    cmp al, 27h
    ja skip_translate
    cmp al, 1
    je esc_pressed
    mov bx, SEL_32bitDS
    mov ds, bx
    mov ebx, offset scan2ascii
    xlat
    mov bx, SEL_flatDS
    mov es, bx
    mov ebx, screen_addr
    cmp al, 8
    je bs_pressed
    mov es:[ebx + 0B8000h], al
    add dword ptr screen_addr, 2
    jmp skip_translate

bs_pressed:
    mov al, ' '
    sub ebx, 2
    mov es:[ebx + 0B8000h], al
    mov screen_addr, ebx

skip_translate:
    in al, 61h
    or al, 80h
    out 61h, al

    mov al, 20h
    out 20h, al

    pop ds
    pop es
    pop ebx
    pop eax

    iretd

esc_pressed:
    in al, 61h
    or al, 80h
    out 61h, al
    mov al, 20h
    out 20h, al
    pop ds
    pop es
    pop ebx
    pop eax

return_in_real_mode:
    cli

    db 0EAh
    dd offset RM_return
    dw SEL_16bitCS

PM_seg_size = $-GDT
PM_seg ends

stack_seg segment para stack 'STACK'
stack_start db 100h dup(?)
stack_l = $-stack_start
stack_seg ends

RM_seg segment para public 'CODE' use16
    assume cs:RM_seg, ds:PM_seg, ss:stack_seg

start:
    mov   ax,PM_seg
    mov   ds,ax

    mov ax, 3
    int 10h

    push PM_seg
    pop ds

    xor	eax,eax
    mov	ax,RM_seg
    shl	eax,4
    mov	word ptr gdt_16bitCS.base_l,ax
    shr	eax,16
    mov	byte ptr gdt_16bitCS.base_m,al
    mov	ax,PM_seg
    shl	eax,4
    push eax
    push eax
    mov	word ptr GDT_32bitCS.base_l,ax
    mov	word ptr GDT_32bitSS.base_l,ax
    mov	word ptr GDT_32bitDS.base_l,ax
    shr	eax,16
    mov	byte ptr GDT_32bitCS.base_m,al
    mov	byte ptr GDT_32bitSS.base_m,al
    mov	byte ptr GDT_32bitDS.base_m,al

    pop eax
    add	eax,offset GDT
    mov	dword ptr gdtr+2,eax
    mov word ptr gdtr, gdt_size-1
    lgdt	fword ptr gdtr

    pop	eax
    add	eax,offset IDT
    mov	dword ptr idtr+2,eax
    mov word ptr idtr, idt_size-1

    mov	eax, offset new_08h
    mov	int08.offs_l, ax
    shr	eax, 16
    mov	int08.offs_h, ax
    mov	eax, offset new_09h
    mov	int09.offs_l, ax
    shr	eax, 16
    mov	int09.offs_h, ax

    in	al, 21h
    mov	master, al
    in	al, 0A1h
    mov	slave, al

    mov	al, 11h
    out	20h, al
    mov	AL, 20h
    out	21h, al
    mov	al, 4

    out	21h, al
    mov	al, 1
    out	21h, al

    mov	al, 0FCh
    out	21h, al

    mov	al, 0FFh
    out	0A1h, al

    lidt	fword ptr idtr

    ; in	al,92h
    ; or	al,2
    ; out	92h,al
    mov	al, 0D1h		; открыть линию А20
    out	64, al
    mov	al, 0DFh
    out	60h, al

    cli
    in	al,70h
    or	al,80h
    out	70h,al

    mov	eax,cr0
    or	al,1
    mov	cr0,eax

    db	66h
    db	0EAh
    dd	offset PM_entry
    dw	SEL_32bitCS

RM_return:
    mov eax, cr0
    and al, 0FEh
    mov cr0, eax

    db 0EAh
    dw $+4
    dw RM_seg

    mov ax, PM_seg
    mov ds, ax
    mov es, ax
    mov ax, stack_seg
    mov bx, stack_l
    mov ss, ax
    mov sp, bx

    mov	al, 11h
    out	20h, al
    mov	al, 8
    out	21h, al
    mov	al, 4
    out	21h, al
    mov	al, 1
    out	21h, al

    mov	al, master
    out	21h, al
    mov	al, slave
    out	0A1h, al

    lidt fword ptr idtr_real

    in al, 70h
    and al, 07Fh
    out 70h, al

    sti

    mov	ax,3
    int	10h

    mov ah, 09h
    mov edx, offset msg_real_back
    int 21h

    mov	ax,4C00h
    int	21h
RM_seg_size = $-start
RM_seg ends

end start
