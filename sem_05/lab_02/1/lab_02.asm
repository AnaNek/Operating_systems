.386P

descr struc
limit dw 0
base_l dw 0
base_m db 0
attr_1 db 0
attr_2 db 0
base_h db 0
descr ends

data segment use16


gdt_null descr <0,0,0,0,0,0>
gdt_data descr <data_size-1,0,0,92h, 0, 0>
gdt_code descr <code_size-1,0,0,98h,0,0>
gdt_stack descr <255,0,0,92h, 0,0>
gdt_screen descr <4095,8000h,0Bh,92h,0,0>

gdt_size=$-gdt_null

pdescr dq 0
sym db 1
attr db 1Eh
s      db ?

mes db 27, '[31; 42m Back real mode ', 27, '[0m$'
msg_real_mode db 'real mode', 13, 10, '$'
msg_real_back db 'back real mode', 13, 10, '$'
msg_prot_mode db 'protected mode'
data_size=$-gdt_null
data ends

text segment 'code' use16
	assume CS:text, DS:data
main proc
	xor eax, eax
	mov AX, data
	mov DS, AX
	shl eax, 4
	mov ebp, eax
	mov bx, offset gdt_data
	mov [bx].base_l, ax
	rol eax, 16
	mov [bx].base_m, al

	xor eax, eax
	mov ax, cs
	shl eax, 4
	mov bx, offset gdt_code
	mov [bx].base_l, ax
	rol eax, 16
	mov [bx].base_m, al

	xor eax, eax
	mov ax, ss
	shl eax, 4
	mov bx, offset gdt_stack
	mov [bx].base_l, ax
	rol eax, 16
	mov [bx].base_m, al

	mov dword ptr pdescr+2, ebp
	mov word ptr pdescr, gdt_size-1
	lgdt pdescr

	mov ah, 09h
	mov dx, offset msg_real_mode
	int 21h

	cli
	mov al, 80h
	out 70h, al

	mov eax, cr0
	or eax, 1

	mov cr0, eax
	db 0EAh
	dw offset continue
	dw 16

continue:
	mov ax, 8
	mov ds, ax

	mov ax, 24
	mov ss, ax

 	mov ax, 32
 	mov es, ax
    mov cx, 14
 	mov si, offset msg_prot_mode
 	mov di, 700
 	mov ah, attr

screen_prot_mode:
	lodsb
 	stosw
    loop screen_prot_mode


	; mov gdt_data.limit, 0FFFFh
	; mov gdt_code.limit, 0FFFFh
	; mov gdt_stack.limit,0FFFFh
	; mov gdt_screen.limit,0FFFFh

	mov ax, 8
	mov ds, ax
	mov ax, 24
	mov ss, ax
	mov ax, 32
	mov es, ax

	db 0EAh
	dw offset go
	dw 16


go:
	mov eax, CR0
	and eax, 0FFFFFFFEh
	mov CR0, eax
	db 0EAh
	dw offset return
	dw text
return:
	mov AX, data
	mov DS, AX
	mov AX, stk
	mov SS, AX

	sti
	mov AL, 0
	out 70h, AL

	mov ah, 09h
	mov dx, offset msg_real_back
	int 21h

	mov ax, 4C00h
	int 21h


main endp
code_size=$-main
text ends
stk segment stack 'stack'
	db 256 dup('^')
stk ends
	end main
