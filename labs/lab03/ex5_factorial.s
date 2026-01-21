.globl factorial

.data
n: .word 8

.text
# Don't worry about understanding the code in main
# You'll learn more about function calls in lecture soon
main:
    la t0, n
    lw a0, 0(t0)
    jal ra, factorial

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

# factorial takes one argument:
# a0 contains the number which we want to compute the factorial of
# The return value should be stored in a0
factorial:
    # t0 = result = 1, t1 = n, t2 = 1
    mv t1, a0
    li t0, 1    
    li t2, 1   
    ble t1, t2, done    # if (n <= 1) goto done

loop:
    mul t0, t0, t1      # result *= n
    addi t1, t1, -1     # n--  
    bgt t1, t2, loop    # if (n > 1) continue

done:
    mv a0, t0
    # This is how you return from a function. You'll learn more about this later.
    # This should be the last line in your program.
    jr ra
