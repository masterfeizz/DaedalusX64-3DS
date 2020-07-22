.arm

//The top two defines needs to be adjusted depending on how gCPUState struct is formated in CPU.h!! //Corn
//
#define _C0_Count	(0x100 + 9 * 4)	//CPU_Control_base + 9*8(64bit regs) or 9*4(32bit regs)
#define _AuxBase	0x280	//Base pointer to Aux regs
#define _CurrentPC	(_AuxBase + 0x00)
#define _TargetPC	(_AuxBase + 0x04)
#define _Delay		(_AuxBase + 0x08)
#define _StuffToDo	(_AuxBase + 0x0c)
#define _Events		(_AuxBase + 0x30)

.extern CPU_UpdateCounter
.extern CPU_HANDLE_COUNT_INTERRUPT
.align 4
.global _EnterDynaRec
.global _DirectExitCheckNoDelay
.global _DirectExitCheckDelay
.global _IndirectExitCheck
.type _EnterDynaRec, %function
.type _DirectExitCheckNoDelay, %function
.type _DirectExitCheckDelay, %function
.type _IndirectExitCheck, %function

_DirectExitCheckNoDelay:
    ldr r4, [r12, #_C0_Count]	// COUNT register
    ldr r5, [r12, #_Events]		// Events[0].mCount
	
	add r4, r4, r0				// COUNT + ops_executed
	str r4, [r12, #_C0_Count]	// COUNT = COUNT + ops_executed
	
	str r1, [r12, #_CurrentPC]	// Current PC
	mov r1, #0
	str r1, [r12, #_Delay]		// Delay = NO_DELAY
	
	sub r5, r5, r0				// Events[0].mCount - ops_executed
	cmp r5, #0
	str r5, [r12, #_Events]
	ble	_DirectExitCheckCheckCount
	bx  lr

_DirectExitCheckDelay:
    ldr r4, [r12, #_C0_Count]	// COUNT register
    ldr r5, [r12, #_Events]		// Events[0].mCount
	
	add r4, r4, r0				// COUNT + ops_executed
	str r4, [r12, #_C0_Count]	// COUNT = COUNT + ops_executed
	
	str r1, [r12, #_CurrentPC]	// Current PC
	str r2, [r12, #_TargetPC]	// Target PC
	mov r1, #1					// EXEC_DELAY
	str r1, [r12, #_Delay]		// Delay = NO_DELAY
	
	sub r5, r5, r0				// Events[0].mCount - ops_executed
	cmp r5, #0
	str r5, [r12, #_Events]
	ble	_DirectExitCheckCheckCount
	bx  lr

#######################################################################################
#	Utility routine for _DirectExitCheckXX.
#
_DirectExitCheckCheckCount:
	mov		r4, lr		// Keep track of return addresss
	mov		r5, r12
	bl		CPU_HANDLE_COUNT_INTERRUPT
	mov		lr,  r4
	mov		r12, r5
	ldr		r0, [r12, #_StuffToDo]		// StuffToDo
	cmp		r0, #0
	popne	{r4-r11, pc}		// Exit the DynaRec
	bx		lr					// Return back to caller

#######################################################################################
#	Update counter. If StuffToDo flags is clear on return,
#	r0 - instructions executed
#	r1 - CIndirectExitMap pointer
#	r2 - exit pc (exit delay is always NO_DELAY)
_IndirectExitCheck:
	mov		r4, r1		// Keep track of map pointer
	mov		r5, r2		// and the exit pc
	mov		r6, r12
	# Can avoid these until Return From DynaRec (pop)?
	str		r2, [r12,#_CurrentPC] 	// CurrentPC
	bl		CPU_UpdateCounter		// a0 holds instructions executed
	mov		r0, #0
	str		r0, [r6, #_Delay]		// Delay (NO_DELAY)

	ldr		r0, [r6, #_StuffToDo]	//  StuffToDo
	cmp		r0, #0
	popne {r4-r11,pc}				// Exit the DynaRec

	mov		r0, r4					//	 p_map
	mov		r1, r5					// exit_pc
	bl		IndirectExitMap_Lookup


	# r0 holds pointer to indirect target. If it's 0, it means it's not compiled yet
	cmp		r0, #0
	popeq   {r4-r11,pc} 			// Exit the DynaRec
	mov     r12,r6					// Restore the CPUState pointer
	bx		r0						// branch to the looked up fragment

_EnterDynaRec:
    push {r4-r11, lr}
    mov r12, r1
    mov r10, r2
	mov r9,  r3
    bx  r0