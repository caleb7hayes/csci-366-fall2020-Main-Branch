        global    find_max
        section   .text
find_max:
        ;; rdi has the array in it
        ;; rsi has the length in it
        mov rax, [rdi] ;; move the first value in the array into rax
        ;; you will need to loop through the array and
        ;; compare each value with rax to determine if it is greater
        ;; after the comparison, decrement the count, bump the
        ;; array pointer by 8 (long long = 64 bits = 8 bytes)
        ;; and if the counter is greater than zero, loop
        ret