
enable_interrupts:
	sti	
	ret

disable_interrupts:
	cli
	ret

;AX:
;	TIMES	2 db 0
;BX:
;	TIMES	2 db 0
SIP:
	TIMES	2 db 0

YKSecond:
	push si
	push bx
	mov si, word[old_task]
	add si, 2
	mov bx, sp
	add bx, 4
	mov bx, [bx]
	mov [si], bx
	;mov si, word[old_task]
	;add si, 2
	;mov ax,[si]	;MIGHT NOT NEED THIS
	mov si, [si]
	mov word[SIP], si
	pop bx
	pop si
	pushf
	push cs
	push word[SIP]			;push ip
	;push word [label]	

	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es
	push ds
	mov si, word[old_task]
	mov [si], sp
	
	mov si, word[running_task]
	mov sp, [si]
	mov ax, 0x200
	add si, 2
	mov si, [si]
	push ax
	push cs
	push si
	iret

YKFirst:
	;push bp
	;mov bp, sp
	mov si, word[running_task]
	mov sp, [si]
	mov ax, 0x200
	add si, 2
	mov si, [si]
	
	push ax
	push cs
	push si
	iret			

YKISR:
	;mov si, word[old_task]		WE DONT WANT THAT
	;mov [si], sp
	;mov word[old_task], si 		;move the current sp register value to the old TCB's sp
	mov si, word[running_task]
	mov sp, [si]	;move the new sp to the sp register			
	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret		; this assumes that the top of the stack is ip, cs, flags. iret atomically pops those three and returns instruction control to the new ip
	
YKDispHandler:
	
	;push bp
	;mov bp, sp
	push si
	push bx
	mov si, word[old_task]
	add si, 2
	mov bx, sp
	add bx, 4
	mov bx, [bx]
	mov [si], bx
	;mov si, word[old_task]
	;add si, 2
	;mov ax,[si]	;MIGHT NOT NEED THIS
	mov si, [si]
	mov word[SIP], si
	pop bx
	pop si
	pushf
	push cs
	push word[SIP]		;push ip
	;push word [label]	

	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es
	push ds
	
		
;save old sp and get new sp
	mov si, word[old_task]
	mov [si], sp
	mov si, word[running_task]
	mov sp, [si]

;	mov si, word [bp+4]	;TCB Location
;	add si, 4		;TCB->STACK Location
;	mov [si], sp		;SP 12E2 | 50CFh <-[BP-16239]
;	add si, 2		;TCB->IP Location
;	call GetIP
;	add ax, 11
;	mov [si], ax
;	mov sp, word[bp+8]
;	call word[bp+6]	
;	add sp, 2			;USED AS JUNK, SO JUST GETTING RID OF IT
;end section


	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	;mov sp, bp
	;pop bp
	iret

;label: DW Label1
;Label1:
;	mov sp, bp
;	pop bp
;	ret

GetIP:
	push bp
	mov bp, sp
	
	mov ax, word[bp+2]
	
	mov sp, bp
	pop bp
	ret

