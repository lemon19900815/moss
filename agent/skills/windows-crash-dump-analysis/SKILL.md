---
name: windows-crash-dump-analysis
description: 分析 Windows 崩溃 dump(.dmp)与偶发崩溃、堆损坏问题的方法与命令集。覆盖异常码策略判定(c0000374/c0000005 等)、cdb/WinDbg 符号配置与现场取证(!analyze -v/.ecxr/~*kb)、堆损坏现场还原(!heap -s、错误地址残留数据、内存字符串搜索)、源码闭环定位无界拷贝与定长缓冲、日志时间线对齐、gflags 页堆复现与修复验证。当用户提供 .dmp 文件要求分析崩溃原因、排查 C++ 内存越界/堆损坏/strcpy 溢出、或需要页堆/gflags/cdb 验证内存问题时使用。
---

# Windows 崩溃 Dump 分析

按"策略判定 → 取证步骤 → 可复用命令"执行。目标：从 dump 到根因到修复验证的完整闭环。

## 核心理念（动手前先记住）

1. **崩溃点 ≠ 犯罪现场**。`0xC0000374`（堆损坏）是分配器**自检失败**：栈顶多为无辜的 malloc/free（数据库、日志等），真凶此前已悄悄越界写坏了堆。`0xC0000005`（AV）多数当场犯罪——看写地址与故障指令即可定罪。
2. **先定异常码，再选路线**。异常码决定后续所有步骤的重点。
3. **三件套交叉闭环**：dump 证据 ↔ 源码 ↔ 日志时间线。三者互证才下结论；只凭调用栈会冤枉人。

## 第 0 步：策略判定（异常码 → 路线）

| 异常码 | 含义 | 路线 |
|---|---|---|
| 0xC0000374 | STATUS_HEAP_CORRUPTION | 延迟自检 → 直奔"堆取证"（第 3 步）；错误地址处的残留数据是破案关键 |
| 0xC0000005 | Access Violation | 看 !analyze 的 WRITE_ADDRESS 与故障指令；写地址远离合法区 = 当场越界 |
| 0xC0000409 | Stack buffer overrun / fast-fail | /GS 栈保护触发；沿栈找 __report_gsfailure 上一层函数 |
| 0xC00000FD | Stack overflow | 递归或深调用；看栈区里重复出现的同一函数帧 |
| 0xC000041D | 回调中未处理异常 | 异常穿过窗口/线程回调边界；查回调注册处 |
| 0xE06D7363 | C++ 异常未捕获 | `.exr -1` 看异常对象，throw 点在异常记录中 |
| 0xC0000135 | DLL not found | 部署/依赖问题，不是代码 bug |

没有 dump 时，事件查看器"应用程序"日志的崩溃条目同样给出异常码，仍可先定策略。

## 第 1 步：环境与符号（决定栈可读性）

- 调试器：`cdb.exe`（WinDbg 命令行版），常见于 `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\`。
- 符号路径 = 本地 PDB + 微软符号服务器（缓存到 C:\symcache，只需配置一次）：
  ```
  set _NT_SYMBOL_PATH=srv*C:\symcache*https://msdl.microsoft.com/download/symbols
  ```
- 本地 PDB 与 exe 放同目录；进入调试器后可用 `.reload /f` 强制加载。
- 打开 dump：`cdb -z <dump路径>`。
- 若 `!heap` 等扩展报 "failed to initialize"：几乎总是 ntdll 符号没加载成功 → 先检查符号路径。
- exe 与 PDB 版本必须匹配（编译时间戳一致），否则行号会错位。

## 第 2 步：第一现场（崩溃线程）

```
!analyze -v      # 先跑：FAULTING_IP / EXCEPTION_CODE / WRITE_ADDRESS / 归因桶
.ecxr            # 切换到异常上下文（不切上下文，看到的栈是错的）
~*kb             # 全部线程栈，识别"干活线程"与"睡觉线程"
~<TID>s; kb      # 切到指定线程
u <地址>          # 反汇编故障点
ln <地址>         # 最近符号（带偏移）→ 定位源码入口
```

判读要点：
- 崩溃线程 TID 与日志中干活的时间点对齐；
- 若只有一个线程活跃、其余全在 Wait/Sleep，重点拓它的完整调用链，从下到上逐层翻译成源码函数；
- 栈上 `模块!函数+偏移` 是定位源码的唯一入口。

## 第 3 步：堆损坏取证（0xC0000374 核心步骤）

```
!heap -s                     # 看 Error type / Error address（如 HEAP_FAILURE_SEGMENT_LFH_BITMAP_CORRUPTION）
db <错误地址>-0x40 L0x200     # 看错误地址周围内存——残留数据往往直接"指名道姓"
s -a <起始地址> L<长度> "<业务关键词>"   # 在内存区搜 SN/SQL/JSON 等业务字符串
!heap -x <地址>               # 查地址的块归属：谁分配的、块多大
!heap -p -a <地址>            # 同上（页堆模式细节更多）
```

关键经验：
- **错误地址附近的内存残留是最强证据**：本案在错误地址处看到 118 字符设备 SN 拼接串，直接锁定"谁的数据越界乱写"。
- **LFH 槽粒度 16 字节**：请求 100 字节 → 实占 **112** 字节槽。写 119 字节 = 越过 112 槽边界 7 字节 → 破坏相邻块/LFH 位图 → 之后任意堆操作触发自检失败。算数要闭合：`写入长度 − 槽大小 = 溢出量`。
- 越界写入点与自检失败点可能相隔毫秒~秒级、甚至不同线程——这就是"栈上出现无辜者"的由来。

## 第 4 步：回源码闭环

1. 用 `ln`/`u` 把栈上模块+偏移翻译成函数，打开源码定位。
2. 双向搜无界写：
   - **写入侧**：`strcpy`、`strcat`、`sprintf`、`memcpy`、`memmove`（目标尤其可能是裸 `char*` 成员）
   - **分配侧**：`new char[N]`、`malloc`、定长分配工厂（如 `AllocMsgRsp(100,...)`）
3. **算数闭合**：dump 残留字符串长度 vs 源码缓冲大小。例：`strlen=118` → strcpy 写 119 字节 > 缓冲 100 → 溢出 19，与 dump 观察一致 → 坐实。
4. 查历史与影响面：
   ```
   git log -S "<可疑代码片段>" --oneline     # 该缺陷的引入提交
   git log --follow -p <文件>               # 变迁与是否已修复
   ```
   确认引入版本、现网版本是否含修复 → 决定是否需紧急出补丁。
5. 接口设计教训：结构体若只有裸 `char*`、无长度字段，被调库**无法知道**调用方缓冲多大，必然埋雷。修复方向二选一：调用方给足缓冲（并要求库侧校验/加长度参数），或库侧按实际长度分配（auto-alloc）。
6. 别只修"踩雷点"：栈上崩溃的模块（数据库连接、日志）通常无辜，改动它只会掩盖问题；真凶在越界写入处。

## 第 5 步：日志时间线对齐

- 崩溃时刻**前 20~50 行**日志：最后一次成功操作是什么？越界发生在哪个请求/响应/回调里？
- 案例模式：崩溃前约 100ms 的 "request → response" 日志里，response 字段长度正好 == dump 残留字符串长度 → 触发路径坐实。
- 进程启动时间与日志重启行对照 → 连续运行时长、恢复中断时长（影响面评估）。

## 第 6 步：页堆复现与修复验证（gflags）

裸跑时越界常常"静默"（写坏邻居，过一会儿才 0xC0000374）。页堆让越界**当场**停在犯罪指令上：

```
# 1) 开启完全页堆（写 HKLM 注册表，需管理员；非提权 shell 用
#    powershell Start-Process -Verb RunAs -Wait 触发 UAC 提权）
gflags /p /enable <程序名.exe> /full

# 验证已生效：
reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\<程序名.exe>"
#   → GlobalFlag=0x02000000, PageHeapFlags=0x3

# 2) 挂调试器跑（避免 WER 弹窗卡死；-g 跳过初始断点，-G 忽略退出断点）
cdb -g -G -c "r; .ecxr; kb; !analyze -v; q" <程序名.exe> > crash.txt 2>&1

# 3) 用完必须关闭（这是持久注册表设置，不是一次性开关！）
gflags /p /disable <程序名.exe>
```

判读：
- 页堆下每次分配**独占页面、末尾紧跟守护页**：越界第 1 字节即 AV（0xC0000005），`FAULTING_IP` 直接落在 strcpy/memcpy 写指令上、调用栈直指犯罪行——与裸跑"延迟 0xC0000374、栈上是无辜 malloc"形成鲜明对比。
- 布局对照互为验证：裸跑相邻块间隔 ≈ LFH 槽（100→112）；页堆下 buffer 到守护页距离同样 ≈112 → 两侧算数一致。
- **修复验证**：打了补丁（长度检查或 auto-alloc）的版本在页堆下应 0 退出。
- `/p /disable` 后注册表键可能残留但值为空，属正常；功能验证（重跑用例行为复原）为准。
- 其他抓崩溃手段：WER LocalDumps（`HKLM\...\Windows Error Reporting\LocalDumps\<exe>`，DumpFolder + DumpType=2）；`procdump -ma -i` 常驻抓取。

## 命令速查（拷贝即用）

```
# 符号与打开
set _NT_SYMBOL_PATH=srv*C:\symcache*https://msdl.microsoft.com/download/symbols
cdb -z crash.dmp

# 第一现场
!analyze -v ; .ecxr ; ~*kb ; ln <addr> ; u <addr>

# 堆取证
!heap -s ; db <err_addr>-0x40 L0x200 ; s -a <start> L<len> "keyword" ; !heap -x <addr>

# 页堆
gflags /p /enable <exe> /full          # 管理员
cdb -g -G -c "r; .ecxr; kb; !analyze -v; q" <exe>
gflags /p /disable <exe>               # 用完必关
```

## 真实案例（浓缩版）

**MiddleWare.exe 偶发崩溃（0xC0000374）**

- `!analyze`：c0000374；崩溃栈落在 mysqlcppconn 的 malloc（数据库层）——**第一眼极易误判**
- `!heap -s`：`HEAP_FAILURE_SEGMENT_LFH_BITMAP_CORRUPTION`，记下 Error address
- `db` 错误地址：正落在 **118 字符设备 SN 拼接串**中部（`1234-6789-1234|xyz-2-d200-1|...|xyz-2-k200-4`）
- 源码闭环：该串是 mlinebus 多设备 client id；`MQTTHandler::request()` 用 `strcpy` 拷进调用方 `new char[100]` 定长缓冲 → 写 119 字节（超 112 槽 7 字节）→ LFH 位图损坏
- 时间线：崩溃前约 100ms 的 request/response 日志中 response 字段 == 该 SN 串 → 触发路径坐实
- 页堆验证（见 `test/strcpy-overflow/helloworld.cpp` 复刻 demo）：裸跑静默越界、delete 时才 0xC0000374；页堆下当场 0xC0000005 于 strcpy 指令
- 结论：**QC 数据库 malloc 只是踩雷点，真凶是约 2 秒前的无界 strcpy**；该 strcpy 自初始提交即存在

**关键教训**：看到 c0000374 而栈在数据库/日志/无关模块时，不要动那个模块——去错误地址的内存里找"别人的数据"。
