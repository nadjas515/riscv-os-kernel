.extern _ZN5RiscV20handleSupervisorTrapEPm

.align 4
.global _ZN5RiscV14supervisorTrapEv
.type _ZN5RiscV14supervisorTrapEv, @function
_ZN5RiscV14supervisorTrapEv:
    csrrw sp, sscratch, sp
    addi sp, sp, -256
    csrr t0, sscratch
    sd t0, 16(sp)
    .irp index, 0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr
    mv a0, sp
    call _ZN5RiscV20handleSupervisorTrapEPm
    la t0, _ZN5RiscV14kernelStackTopE
    ld t0, 0(t0)
    csrw sscratch, t0
    .irp index, 0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(sp)
    .endr
    ld sp, 16(sp)
    sret