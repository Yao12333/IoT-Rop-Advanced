# 🔥 IoT-Rop-Advanced

> IoT 设备 MIPS ROP 利用研究 — 针对真实路由器固件的漏洞分析与攻击链构造

## 📂 项目结构

```
📂 IoT-Rop-Advanced
│
├── 📂 src/                          ← 【敌人】漏洞程序、编译规则
│   ├── 📂 cisco_vulnerability/
│   │   ├── vuln_nobd.c              # 删掉后门的 C 源码（缓冲区溢出）
│   │   └── Makefile                 # MIPS 交叉编译脚本
│   └── 📂 dlink_dir605/             # 🔜 下周扩展：D-Link 真实固件
│       └── README.md
│
├── 📂 exploits/                     ← 【我的武器】攻击脚本
│   └── exp_nobd.py                  # MIPS libc ROP 链利用脚本
│
├── 📂 artifacts/                    ← 【情报与垫脚石】中间数据
│   └── .gitkeep
│
├── 📝 LICENSE                       # MIT 开源许可证
└── 📝 README.md                     # 你正在看的这个文件
```

## 🎯 三大管理铁律

| 文件夹 | 角色 | 放什么 | 命名规范 |
|--------|------|--------|----------|
| `src/` | **敌人** | 漏洞程序源码、编译规则、固件提取的二进制 | 按目标设备建子目录，如 `cisco_vulnerability/` |
| `exploits/` | **我的武器** | Python 攻击脚本、Shellcode 生成器 | `exp_<目标名>.py`，与 src 子目录对应 |
| `artifacts/` | **情报与垫脚石** | ROP 汇编快照、固件抠出的 `.so` 库、偏移分析笔记 | 任何不能直接运行但对推导 ROP 链至关重要的中间产物 |

---

## 📖 WriteUp: Cisco nobd 缓冲区溢出 (MIPS ROP)

### 漏洞概述

目标程序 [`vuln_nobd.c`](src/cisco_vulnerability/vuln_nobd.c) 模拟了 Cisco 路由器 `nobd` 后门删除后的遗留漏洞：

- 栈缓冲区 `buf[64]`，但 `fgets()` 读取 200 字节 → **栈溢出**
- 编译时关闭了栈保护 (`-fno-stack-protector`) 和开启了栈可执行 (`-z execstack`)
- 目标架构：**MIPS Little-Endian**（`mipsel`），通过 QEMU 用户态模拟运行

### ROP 链构造思路

在 MIPS 架构下，由于没有直接的 `call` 指令，函数调用通过 `jalr` 跳转并依赖延迟槽，ROP 链的构造比 x86 更复杂。本利用采用 **libc gadget** 组合：

```
阶段 1: li $v0, 0xfa1 ; jr $ra ; nop     ← 设置系统调用号 4097 (execve)
阶段 2: addiu $a0, $sp, 0x18 ; jr $ra     ← 将 $a0 指向栈上的 "/bin/sh"
阶段 3: syscall                            ← 触发系统调用，获取 shell
```

**Payload 布局：**

| 偏移 | 内容 | 说明 |
|------|------|------|
| 0-67 | `A` * 68 | 填充至 $ra |
| 68-71 | gadget_v0 | 覆盖 $ra → 阶段 1 |
| 72-75 | 填充 | 延迟槽 |
| 76-79 | gadget_a0 | 阶段 2 |
| 80-83 | 填充 | 延迟槽 |
| 84-95 | 填充 | 占位垫脚石 (12B) |
| 96-99 | gadget_syscall | 阶段 3 (4B) |
| 100-107 | 填充 | 占位垫脚石 (8B) |
| 108-115 | `"/bin/sh\0"` | 终极弹药，位于 $sp+0x18 |

### 运行方式

```bash
# 1. 编译目标（需要 mips-linux-gnu-gcc 交叉编译工具链）
cd src/cisco_vulnerability && make

# 2. 运行攻击脚本（从 exploits 目录执行）
cd exploits && python3 exp_nobd.py
```

### 环境说明

- 实验环境关闭 ASLR，libc 基址固定为 `0x77f00000`
- 真实路由器中需通过信息泄露获取动态基址
- 使用 `qemu-mipsel-static` 用户态模拟，libc 路径 `/usr/mipsel-linux-gnu`

---

## 🚀 扩容指南

当你发现新的 IoT 漏洞目标时，按以下步骤扩展：

### 1. 添加新漏洞目标

```bash
# 在 src/ 下创建以设备命名的子目录
mkdir src/<device_name>/
# 将漏洞源码或固件提取的二进制放入
```

### 2. 编写对应攻击脚本

```bash
# 在 exploits/ 下创建对应脚本，命名与 src 子目录对应
touch exploits/exp_<device_name>.py
```

### 3. 存放分析情报

```bash
# 将 ROP gadget 快照、固件 libc、偏移笔记等放入 artifacts/
cp <extracted_files> artifacts/
```

### 4. 更新 README

在本文档的 WriteUp 章节下追加新漏洞的分析记录。

---

## 📜 License

本项目基于 [MIT License](LICENSE) 开源。

Copyright (c) 2026 Yao12333
