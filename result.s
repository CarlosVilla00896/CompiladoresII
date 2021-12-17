.data

.globl main
.text
main: 
addiu $sp, $sp, -4

sw $ra, 0($sp)


lw $ra, 0($sp)
addiu $sp, $sp, 4
jr $ra

