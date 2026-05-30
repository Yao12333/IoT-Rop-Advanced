# 📋 IoT-Rop-Advanced 项目整理报告 & 使用说明

> 生成时间：2026-05-30  
> 仓库地址：https://github.com/Yao12333/IoT-Rop-Advanced

---

## 一、执行了哪些命令（完整操作日志）

### 1. 目录重组

```bash
# 创建子目录结构
mkdir -p /home/debian/git/src/cisco_vulnerability
mkdir -p /home/debian/git/src/dlink_dir605

# 将源码和编译脚本移入 cisco_vulnerability 子目录
mv /home/debian/git/src/vuln_nobd.c /home/debian/git/src/cisco_vulnerability/vuln_nobd.c
mv /home/debian/git/src/Makefile /home/debian/git/src/cisco_vulnerability/Makefile
```

### 2. 文件内容更新

```bash
# 更新 exp_nobd.py 中的二进制引用路径
# 旧: p = process(['qemu-mipsel-static', '-L', '/usr/mipsel-linux-gnu', './vuln_nobd'])
# 新: p = process(['qemu-mipsel-static', '-L', '/usr/mipsel-linux-gnu', '../src/cisco_vulnerability/vuln_nobd'])
```

### 3. 新建占位文件

```bash
# D-Link 扩展预留目录的说明文件
# 创建: src/dlink_dir605/README.md

# 确保 artifacts 空目录被 Git 跟踪
# 创建: artifacts/.gitkeep
```

### 4. 重写 README.md

完整重写项目文档，包含：项目介绍、目录树、三大管理铁律、Cisco nobd 漏洞 WriteUp、扩容指南。

### 5. 网络问题排查与修复

```bash
# 排查发现：Git 配置了失效的 HTTPS 代理
git config --global --get https.proxy
# 输出: http://127.0.0.1:7897 （本地无此服务，导致连接失败）

# 清除失效代理
git config --global --unset https.proxy

# 尝试 HTTPS 直连 → 失败（GFW 阻断 443 端口 TLS SNI）
# 错误: Failed to connect to github.com port 443 after 21019 ms
```

### 6. SSH 协议配置（绕过 GFW 的终极方案）

```bash
# 生成 SSH 密钥（ed25519 算法，比 RSA 更安全更短）
mkdir -p /home/debian/.ssh
ssh-keygen -t ed25519 -C "3188542309@example.com" -f /home/debian/.ssh/id_ed25519 -N ""

# 将公钥添加到 GitHub: Settings → SSH and GPG keys → New SSH key
# 公钥: ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIFOCAaYVWMpYeKwjDuDRdZ87uXP6xGm100aHdvJVf+OS

# 将 remote 从 HTTPS 切换为 SSH
git remote remove origin
git remote add origin git@github.com:Yao12333/IoT-Rop-Advanced.git

# 预存 GitHub 的 host key（避免首次连接交互提示）
ssh-keyscan github.com >> /home/debian/.ssh/known_hosts 2>/dev/null

# 推送成功！
git push -u origin main
# 输出: To github.com:Yao12333/IoT-Rop-Advanced.git
#        683154f..2c07fcf  main -> main
```

### 7. 清理临时文件

```bash
rm -f /home/debian/git/do_push.sh
rm -f /home/debian/git/setup_ssh.sh
rm -f /home/debian/git/push_to_github.sh
```

---

## 二、做了什么（变更清单）

| # | 操作 | 文件 | 说明 |
|---|------|------|------|
| 1 | 移入子目录 | `src/vuln_nobd.c` → `src/cisco_vulnerability/vuln_nobd.c` | 按漏洞目标分类 |
| 2 | 移入子目录 | `src/Makefile` → `src/cisco_vulnerability/Makefile` | 与源码同目录 |
| 3 | 更新路径 | `exploits/exp_nobd.py` 第8行 | `./vuln_nobd` → `../src/cisco_vulnerability/vuln_nobd` |
| 4 | 新建 | `src/dlink_dir605/README.md` | D-Link 扩展占位 |
| 5 | 新建 | `artifacts/.gitkeep` | Git 空目录跟踪 |
| 6 | 重写 | `README.md` | 完整项目文档 |
| 7 | 配置 | SSH 密钥 + remote 切换 | 绕过 GFW 阻断 |
| 8 | 清除 | `https.proxy = http://127.0.0.1:7897` | 移除失效代理 |

---

## 三、当前项目最终结构

```
📂 /home/debian/git/ (IoT-Rop-Advanced)
│
├── 📂 src/                              ← 【敌人】漏洞程序、编译规则
│   ├── 📂 cisco_vulnerability/
│   │   ├── vuln_nobd.c                  # 缓冲区溢出漏洞源码
│   │   └── Makefile                     # MIPS 交叉编译脚本
│   └── 📂 dlink_dir605/                 ← 下周扩展
│       └── README.md                    # 占位说明
│
├── 📂 exploits/                         ← 【我的武器】攻击脚本
│   └── exp_nobd.py                      # MIPS libc ROP 链利用脚本
│
├── 📂 artifacts/                        ← 【情报与垫脚石】中间数据
│   └── .gitkeep                         # 确保 Git 跟踪空目录
│
├── 📝 LICENSE                           # MIT 开源许可证
└── 📝 README.md                         # 项目门面文档
```

---

## 四、后续规划和管理上传靠的是什么

### 🔑 核心基础设施：SSH 协议

你的虚拟机已配置好 SSH 密钥认证，后续所有 Git 操作都通过 SSH（22端口）完成，**无需输入密码或 Token**，且不受 GFW 对 HTTPS 443端口的阻断影响。

**关键文件位置：**
- 私钥：`/home/debian/.ssh/id_ed25519`（绝不能泄露）
- 公钥：`/home/debian/.ssh/id_ed25519.pub`（已添加到 GitHub）
- Host Key：`/home/debian/.ssh/known_hosts`（已预存 github.com）

**Remote URL：** `git@github.com:Yao12333/IoT-Rop-Advanced.git`

---

### 📌 日常三连击（最常用）

每次修改了代码或新增文件后：

```bash
cd /home/debian/git

# 1. 扫描所有修改
git add .

# 2. 盖戳保存
git commit -m "描述你改了什么"

# 3. 推送到 GitHub
git push
```

---

### 🔄 异地同步（换电脑/重装系统后）

```bash
# 克隆仓库（SSH 方式，无需密码）
git clone git@github.com:Yao12333/IoT-Rop-Advanced.git
cd IoT-Rop-Advanced
```

---

### ⬇️ 拉取最新代码（开始工作前必做）

```bash
cd /home/debian/git
git pull
```

> 💡 **好习惯**：每次写新代码前先 `git pull`，避免版本冲突。

---

### 🧹 清理误传的大文件

```bash
# 从 GitHub 删除但保留本地文件
git rm --cached <文件路径>
git commit -m "Clean up: remove binary file"
git push
```

---

### 🚀 下周扩容：添加 D-Link DIR-605 漏洞

按照三大管理铁律，三步走：

```bash
cd /home/debian/git

# 第1步：往 src/ 里放漏洞程序
cp pwn_target src/dlink_dir605/
cp tftp_service.c src/dlink_dir605/

# 第2步：往 exploits/ 里写攻击脚本
touch exploits/exp_dir605_rop.py

# 第3步：往 artifacts/ 里存情报
cp dlink_libc.so.0 artifacts/
cp mips_libc_rop.txt artifacts/

# 提交并推送
git add .
git commit -m "Add D-Link DIR-605 exploit and ROP artifacts"
git push
```

---

### 📐 三大管理铁律（肌肉记忆）

| 文件夹 | 角色 | 放什么 | 命名规范 |
|--------|------|--------|----------|
| `src/` | **敌人** | 漏洞程序源码、编译规则、固件提取的二进制 | 按设备建子目录，如 `cisco_vulnerability/` |
| `exploits/` | **我的武器** | Python 攻击脚本、Shellcode 生成器 | `exp_<目标名>.py`，与 src 子目录对应 |
| `artifacts/` | **情报与垫脚石** | ROP 快照、固件 `.so` 库、偏移分析笔记 | 不能直接运行但对推导 ROP 链至关重要的中间产物 |

---

### ⚠️ 安全提醒

1. **PAT 令牌已泄露**：你的 `github_pat_11AX5Q...` 已在此对话中明文出现，请立即到 GitHub Settings → Developer settings → Personal access tokens 中**撤销并重新生成**。
2. **SSH 私钥保护**：`/home/debian/.ssh/id_ed25519` 是你的身份凭证，绝不能复制、传输或公开。
3. **编译产物不上传**：`vuln_nobd` 二进制、`__pycache__` 等应加入 `.gitignore`，只传源码。

---

### 🛠️ 建议后续优化

1. **添加 `.gitignore`**：排除编译产物和 Python 缓存
   ```
   # 编译产物
   src/**/vuln_nobd
   src/**/*.o
   
   # Python 缓存
   __pycache__/
   *.pyc
   
   # IDE
   .vscode/
   .idea/
   ```

2. **GitHub Actions CI**：自动编译 MIPS 二进制，验证 ROP 链可用性

3. **artifacts 补充**：将你用 ROPgadget/ropper 导出的 gadget 快照存入 `artifacts/mips_libc_rop.txt`
