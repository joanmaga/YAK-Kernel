ISR_Reset:
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es
	push ds

	call YKEnterISR
	sti
	
	call vReset

	cli
	call signalEOI
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret
ISR_Tick:
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es
	push ds
	cmp word[YK_Depth], 0000
	jne label1
	
	mov si, word[running_task]	;TWO LINES ADDED LAST
	mov [si], sp			;
label1:	
	call YKEnterISR
	sti
	
	
	call vTick
	call YKTickHandler
	cli
	call signalEOI
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret

ISR_Keyboard:

	
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es
	push ds
	
	cmp word[YK_Depth], 0000
	jne label3

	mov si, word[running_task]	;TWO LINES ADDED LAST
	mov [si], sp			;
label3:
	call YKEnterISR

	sti
	
	call vKeyboard

	cli
	call signalEOI
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret
