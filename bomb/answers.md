1. 调试：

```bash
gdb bomb
```

在 gdb 中调试；同时可以在 gdb 中查看 asm 汇编和 regs 寄存器状态，使用：

```bash
layout asm
layout regs
```

就可以在 gdb 上面实时显示

防止炸弹爆炸：

```gdb
break explode_bomb
```

然后在 phase 1 的入口处打一个断点，然后 `run` 就会运行到这里停下：


2. pahse 1
```bash
break phase_1
```

观察到

```asm
mov $0x402400,%esi
call 0x401388
```

也就是在调用 `string_not_equal` 这个函数之前，先把 `0x402400` 内存地址中的东西放到了 `esi` 寄存器中；而：
- `esi` 寄存器是 `rsi` 寄存器的低 32 位；
- 在调用规定中，
    - **第一个参数**必须放到 `rdi` 中
    - **第二个参数**必须放到 `rsi` 中
    - **第三个参数**必须放到 `rdx` 中
    而第一个参数是用户的输入；

使用 `x/s 0x402400` 查看内容如下：

> Border relations with Canada have never been better.

3. 
![alt text](./figures/image.png)

- 注意到 `read_six_numbers` 这个函数，那么上面的操作就相当于在栈上为其开辟相应的存储空间
- `(%rsp)` 相当于取出栈顶的数字，也就是输入数组中的第一个数字； `cmpl $0x1,(%rsp)` 的意思是做减法运算： `(%rsp) - 1` ，如果结果为 0 （即两个数相等），就把零标志位（ZF）设置为1； `je` 看到 1 就跳转到目标地址 `0x400f30` ，否则顺序执行，炸弹爆炸；

- `lea` ：load effective address，加载有效地址
- `0x400f40` 的指令是： `lea 0x4(%rsp),%rbx` ，就是把 `rbx` 指向栈顶往上偏移4个字节（第一个元素）
- 跳到 `0x400f17` 后， `mov -0x4(%rbx),%eax` 的意思是把 `rbx` 前面一个元素给 `eax` ；
- 然后 `add %eax,%eax` 相当于把 `rbx` 前面一个元素翻倍；
- 然后 `cmp %eax,(%rbx)` 相当于在比较 `rbx` 跟前面一个元素的两倍，如果相等的话跳转到 `0x400f25` ，否则继续往下执行，炸弹爆炸；
- `0x400f25` 处是把 `rbx` 指针继续往后移动（在不越界的情况下）；
- 所以，上面所有的事情就是在说：输入6 个数字，先看第一位是不是1，如果是的话，要求后面每个数字都是前面数字的两倍；

> 1 2 4 8 16 32

4. 