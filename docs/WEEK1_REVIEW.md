# 📑 Week 1 复盘笔记：MIPS 漏洞挖掘与自动化 ROP 实战

> **时间跨度**：Day 1 - Day 7  
> **作者**：Yao12333  
> **项目仓库**：https://github.com/Yao12333/IoT-Rop-Advanced  
> **目标架构**：MIPS32 Little-Endian (mipsel)  
> **目标设备**：Cisco 路由器 (nobd 后门删除后的遗留漏洞)

---

## 一、阵地搭建篇：交叉编译与环境部署

### 1.1 交叉编译环境 (MIPS32 小端序)

在 Debian 本地虚拟机环境下，通过安装 GNU 交叉编译器，将含有栈溢出缺陷的 C 源码编译为 MIPS 架构的静态链接二进制文件：

```bash
# 安装交叉编译器
sudo apt-get install gcc-mipsel-linux-gnu

# 静态编译目标靶机，关闭所有保护
mipsel-linux-gnu-gcc vuln_nobd.c -o vuln_nobd -static -fno-stack-protector -z execstack
```

**编译选项说明：**

| 选项 | 作用 |
|------|------|
| `-static` | 静态链接，将 libc 直接编入二进制，便于本地 gadget 检索 |
| `-fno-stack-protector` | 关闭栈保护（canary），使栈溢出可直接覆盖返回地址 |
| `-z execstack` | 标记栈段可执行，允许栈上的 shellcode 直接运行 |

### 1.2 模拟运行与远程调试生态

通过 QEMU 用户态模拟 MIPS 运行环境，并暴露 `1234` 端口，打通 `gdb-multiarch` 的远程协议握手：

```bash
# 开启远程调试挂起程序
qemu-mipsel-static -g 1234 ./vuln_nobd

# 另一终端连接 GDB
gdb-multiarch ./vuln_nobd
(gdb) set architecture mips
(gdb) target remote :1234
```

---

## 二、战术突袭篇：内存特征检索与栈劫持

### 2.1 栈特征检索法 (周二夜袭 / 周三核心)

通过双开终端，GDB 连上 QEMU。在 `vuln` 函数下断点并测算栈内存结构。

**操作流程：**

1. **断点定位**：`(gdb) b vuln` → `(gdb) c`
2. **特征输入**：输入特征字符串（如 `cyclic 200` 或 200 个测序字符）
3. **崩溃触发**：程序触发 `SIGSEGV`，控制流被劫持
4. **栈内存倾倒**：`(gdb) x/50wx $sp`
5. **特征定位**：在十六进制数据中精准捕获 `0x61616161` (`'aaaa'`)

### 2.2 栈结构与偏移量精确弹道

通过分析 `jr $ra` 前的汇编指令以及栈顶动态，得出以下关键数据：

| 内存关键锚点 | 对应的绝对内存地址 / 相对偏移 | 作用说明 |
|---|---|---|
| **缓冲区起始点** | `$sp + 0` | 溢出输入的源头 |
| **安全填充区 (Padding)** | **68 字节** | 填满缓冲区到返回地址之间的空隙 |
| **返回地址寄存器 ($ra)** | **`$sp + 92`** | 决定程序下一步执行哪里的核心寄存器 |
| **后门函数 (backdoor)** | **`0x004009c0`** | `info functions` 检索出的隐藏合规跳转点 |

### 2.3 手工内存劫持验证

在 GDB 中通过 `set` 命令强行改写内存，成功拦截控制流并跳转至后门：

```text
(gdb) set {int}($sp + 92) = 0x004009c0
(gdb) c
# 结果：成功改变寄存器弹道，程序完成非预期跳转！
```

**验证结论**：68 字节填充 + 后门地址覆盖 `$ra` = 控制流劫持成功。

---

## 三、军火库扩容篇：网络高墙下的标准仓库布局

### 3.1 精美规范工程目录

为后续持续打入真实路由器固件（如 D-Link、Tenda），项目严格按照工业级安全武器库标准进行归类：

```
📂 IoT-Rop-Advanced
├── 📂 src/                    ← 【敌人】漏洞程序、编译规则
│   ├── 📂 cisco_vulnerability/
│   │   ├── vuln_nobd.c
│   │   └── Makefile
│   └── 📂 dlink_dir605/       ← 下周扩展
├── 📂 exploits/               ← 【我的武器】攻击脚本
│   └── exp_nobd.py
├── 📂 artifacts/              ← 【情报与垫脚石】中间数据
├── 📝 README.md
├── 📝 PROJECT_REPORT.md
├── 📝 .gitignore
└── 📝 LICENSE
```

**三大管理铁律：**

| 文件夹 | 角色 | 放什么 | 命名规范 |
|--------|------|--------|----------|
| `src/` | 敌人 | 漏洞程序源码、编译规则、固件提取的二进制 | 按设备建子目录 |
| `exploits/` | 我的武器 | Python 攻击脚本、Shellcode 生成器 | `exp_<目标名>.py` |
| `artifacts/` | 情报与垫脚石 | ROP 快照、固件 `.so` 库、偏移分析笔记 | 不能直接运行的中间产物 |

### 3.2 GitHub Token 权限与网络突围

在国内网络连接频繁遭遇 `Connection reset`（TCP RST 阻断）及 `403 Access Denied` 的情况下，实施了以下技术突围：

**问题根因分析：**

- `ping github.com` 延迟 48ms、0% 丢包 → ICMP 层面连通
- `git push` 走 HTTPS (443端口) → TLS 握手时 SNI 明文暴露 `github.com` → GFW 精准阻断 → TCP RST

**解决方案：**

1. **细粒度令牌调教**：在 GitHub 生成 Fine-grained PAT 时，必须显式在 `+ Add permissions` 中将 **`Contents`** 权限修改为 **`Read and write`**
2. **SSH 强加密隧道**：全面废弃不稳定的 HTTPS (443端口) 传输，改用强加密、无明文 SNI 特征的 SSH (22端口) 协议绕过审查阻断：

```bash
# 生成 SSH 密钥
ssh-keygen -t ed25519 -C "3188542309@example.com"

# 将公钥添加到 GitHub: Settings → SSH and GPG keys → New SSH key

# 切换 remote 为 SSH
git remote remove origin
git remote add origin git@github.com:Yao12333/IoT-Rop-Advanced.git

# 推送
git push -u origin main
```

**排查过程中发现并修复的问题：**

- Git 全局配置了失效的 HTTPS 代理 `http://127.0.0.1:7897`（本地无此服务）
- 执行 `git config --global --unset https.proxy` 清除后，错误信息从"连接被重置"变为"443端口无法连接"，确认是 GFW 阻断而非代理问题

---

## 四、后续日常管理指令（肌肉记忆指南）

### 4.1 日常三连击

```bash
cd /home/debian/git
git add .
git commit -m "描述你改了什么"
git push
```

### 4.2 异地同步

```bash
git clone git@github.com:Yao12333/IoT-Rop-Advanced.git
```

### 4.3 拉取最新代码

```bash
git pull  # 每次写新代码前必做
```

### 4.4 清理误传的大文件

```bash
git rm --cached <文件路径>
git commit -m "Clean up: remove binary file"
git push
```

---

## 五、下周作战计划

| 优先级 | 目标 | 具体任务 |
|--------|------|----------|
| P0 | D-Link DIR-605 真实固件 | 从固件中提取漏洞二进制和 libc，填入 `src/dlink_dir605/` |
| P1 | 动态基址泄露 | 真实路由器开启 ASLR，需通过信息泄露获取 libc 基址 |
| P2 | 自动化 ROP 链生成 | 编写通用 gadget 搜索 + 链自动拼接工具 |

---

## 六、关键指标总结

| 指标 | 数值 |
|------|------|
| 栈溢出偏移量 | **68 字节** |
| 返回地址寄存器位置 | `$sp + 92` |
| 后门函数地址 | `0x004009c0` |
| libc 基址（实验环境） | `0x77f00000` |
| ROP gadget: `li $v0, 0xfa1` | `libc_base + 0x259e4` |
| ROP gadget: `addiu $a0, $sp, 0x18` | `libc_base + 0x1bc24` |
| ROP gadget: `syscall` | `libc_base + 0x26e64` |
| 系统调用号 (execve) | 4097 (`0xfa1`) |
