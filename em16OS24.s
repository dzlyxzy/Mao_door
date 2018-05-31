
;***********************************************************
;*Copyright (c)   
;*  ɽ����ѧ���ܽ�������ܼ����о���  
;*All right reserved
;*�ļ���em16RTOS.c
;*��ǰ�汾��4.0
;*���ߣ��� ��
;*������ڣ�2007��9�� 13 ��
;*�����ֵ��ȷ�ʽ����Ϣ�������ж����������Ѿ�����ͨ����
;**************************************************************/


.include "p24FJ64GA002.inc" 

.text
.macro OS_REGS_SAVE                                                        ; ѹջ
    push PSVPAG
    push RCOUNT
    push CORCON
    push TBLPAG
    push WREG0
    push WREG1
    push WREG2
    push WREG3
    push WREG4
    push WREG5
    push WREG6
    push WREG7
    push WREG8
    push WREG9
    push WREG10
    push WREG11
    push WREG12
    push WREG13
    push WREG14      
.endm                                                                   ; End of Macro


.macro OS_REGS_RESTORE                                                     ; ��ջ
      pop WREG14
      pop WREG13
      pop WREG12
      pop WREG11
      pop WREG10
      pop WREG9
      pop WREG8
      pop WREG7
      pop WREG6
      pop WREG5
      pop WREG4
      pop WREG3
      pop WREG2
      pop WREG1
      pop WREG0
      pop TBLPAG
      pop CORCON
      pop RCOUNT
      pop PSVPAG
.endm                                                                   ; End of Macro


     .global _SchedTask
  ;   .global __T1Interrupt
 ;    .global __T3Interrupt
 ;    .global __U1RXInterrupt

     
     
  _SchedTask:
        
        call _DisInt
        mov _CurrentTaskSPRoot,w0
        mov _CurrentTaskPCL,w1
        mov w1,[w0++]
        mov _CurrentTaskPCH,w1
        mov w1,[w0]
        call _FindMaxPrioTask
        ulnk
        mov _CurrentTaskSPRoot,w0
        mov [W0++],[w15++]  ;PCL
        mov [W0++],[w15++]  ;PCH
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;
        mov [W0++],[w15++]  ;w14
                       
        OS_REGS_RESTORE
        call _EnInt
        return
