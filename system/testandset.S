// uint32 test_and_set(uint32 *ptr, uint32 new_value) {  
//    uint32 old_value = *ptr;  
//    *ptr = new_value;  
//    return old_value;  
// }

        .text
        .globl test_and_set

test_and_set:
        pushl %ebp        // Push EBP on stack
        movl  %esp, %ebp  // Reloads EBP

        movl  8(%ebp), %edx  // &ptr adress is in edx
        movl  12(%ebp), %eax // new_value is in eax

        xchgl %eax, (%edx)   // both values are exchanged
        
        popl  %ebp
        ret