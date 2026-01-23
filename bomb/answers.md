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