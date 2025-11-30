# A2A C++ SDK

基于 A2A (Agent-to-Agent) 协议的 C++ 实现，提供构建和连接智能 Agent 的完整框架。

## 📖 项目介绍

A2A C++ SDK 是一个生产级的 Agent 通信框架，实现了完整的 A2A 协议规范。本项目提供了从协议层到应用层的完整实现，支持构建分布式、可扩展的多 Agent 系统。

**核心价值：**
- 🎯 **标准化通信**：完整实现 A2A 协议，确保 Agent 间互操作性
- 🚀 **生产就绪**：基于 Redis 的分布式 TaskStore，支持持久化和横向扩展
- 🔧 **易于开发**：清晰的分层架构，简洁的 API 设计
- 💪 **高性能**：C++ 实现，支持高并发和低延迟场景
- 🔍 **服务发现**：内置注册中心，支持动态服务注册与发现

## 🏗️ 系统架构

### 1. SDK 分层架构

```
┌─────────────────────────────────────────────────────────┐
│                    应用层 (Application)                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Orchestrator │  │  Math Agent  │  │ Other Agents │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│                   服务层 (Server)                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ AgentServer  │  │ TaskManager  │  │  TaskStore   │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│                   客户端层 (Client)                      │
│  ┌──────────────┐  ┌──────────────┐                   │
│  │ AgentClient  │  │  HttpClient  │                   │
│  └──────────────┘  └──────────────┘                   │
├─────────────────────────────────────────────────────────┤
│                    核心层 (Core)                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │  AgentCard   │  │ AgentMessage │  │   JSON-RPC   │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 2. 分布式部署架构（固定地址）

```
┌─────────────────┐         ┌─────────────────┐
│  Orchestrator   │         │   Math Agent    │
│   (进程 1)       │         │   (进程 2)       │
│   端口: 5000     │         │   端口: 5001     │
└────────┬────────┘         └────────┬────────┘
         │                           │
         │    ┌─────────────────┐   │
         └────►   Redis Store   ◄───┘
              │  (TaskStore)    │
              │   端口: 6379     │
              └─────────────────┘
                      ▲
                      │
              持久化 + 跨进程共享
```

### 3. 动态服务发现架构（推荐）

```
┌──────────────┐         ┌──────────────┐
│ Orchestrator │         │ Math Agent 1 │
│  端口: 5000   │         │  端口: 5001   │
└──────┬───────┘         └──────┬───────┘
       │                        │
       │  ┌──────────────┐      │
       │  │ Math Agent 2 │      │
       │  │  端口: 5002   │      │
       │  └──────┬───────┘      │
       │         │              │
       └─────────┼──────────────┘
                 │
        ┌────────▼────────┐
        │ Registry Server │  ← 服务注册与发现
        │   端口: 8500     │
        └────────┬────────┘
                 │
        ┌────────▼────────┐
        │  Redis Store    │  ← 历史消息持久化
        │   端口: 6379     │
        └─────────────────┘
```

**架构特点：**
- **独立进程**：每个 Agent 独立部署，互不影响
- **服务发现**：通过注册中心动态查找 Agent
- **负载均衡**：轮询方式选择可用的 Agent 实例
- **Redis 持久化**：历史消息存储在 Redis
- **跨进程共享**：多个 Agent 共享对话历史
- **故障隔离**：单个 Agent 故障不影响整体系统

## 📁 代码结构

```
a2a-cpp/
├── include/a2a/                    # SDK 公共头文件
│   ├── core/                       # 核心层：协议模型
│   │   ├── jsonrpc_request.hpp     # JSON-RPC 请求
│   │   ├── jsonrpc_response.hpp    # JSON-RPC 响应
│   │   ├── error_code.hpp          # 错误码定义
│   │   └── types.hpp               # 基础类型定义
│   ├── models/                     # 数据模型
│   │   ├── agent_card.hpp          # Agent 元数据
│   │   ├── agent_message.hpp       # 消息模型
│   │   ├── agent_task.hpp          # 任务模型
│   │   └── message_part.hpp        # 消息部分
│   ├── client/                     # 客户端层
│   │   └── a2a_client.hpp          # A2A 客户端
│   └── server/                     # 服务端层
│       ├── task_manager.hpp        # 任务管理器
│       ├── task_store.hpp          # TaskStore 接口
│       └── memory_task_store.hpp   # 内存实现
│
├── src/                            # SDK 实现文件
│   ├── core/                       # 核心层实现
│   ├── models/                     # 模型层实现
│   ├── client/                     # 客户端层实现
│   └── server/                     # 服务端层实现
│
├── examples/multi_agent_demo/      # 示例：分布式多 Agent 系统
│   ├── redis_orchestrator.cpp      # 固定地址 Orchestrator
│   ├── redis_math_agent.cpp        # 固定地址 Math Agent
│   ├── dynamic_orchestrator.cpp    # 动态服务发现 Orchestrator
│   ├── dynamic_math_agent.cpp      # 动态服务发现 Math Agent
│   ├── registry_server.cpp         # 注册中心服务器
│   ├── redis_task_store.hpp/cpp    # Redis TaskStore 实现
│   ├── agent_registry.hpp          # 注册中心核心逻辑
│   ├── registry_client.hpp         # 注册中心客户端
│   ├── interactive_client.cpp      # 交互式测试客户端
│   ├── start_redis_system.sh       # 启动固定地址系统
│   ├── start_dynamic_system.sh     # 启动动态服务发现系统
│   ├── test_redis_system.sh        # 自动化测试脚本
│   └── chat.sh                     # 交互式聊天脚本
│
├── build/                          # 编译输出目录
├── CMakeLists.txt                  # 主构建配置
├── README.md                       # 本文档
├── 架构说明.md                      # 详细架构文档
└── 使用指南.md                      # 开发指南
```

## ✨ 核心特性

### SDK 功能

- ✅ **完整的 A2A 协议实现**
  - JSON-RPC 2.0 消息格式
  - Agent Card 元数据
  - 上下文管理（contextId）
  - 历史长度控制（history_length）

- ✅ **灵活的 TaskStore**
  - 内存实现（MemoryTaskStore）：适合单机开发
  - Redis 实现（RedisTaskStore）：适合生产环境
  - 可扩展接口（ITaskStore）：支持自定义实现

- ✅ **服务注册与发现**
  - 自动服务注册
  - 按标签查找服务
  - 健康检查机制
  - 负载均衡（轮询）

- ✅ **生产级特性**
  - 线程安全设计
  - HTTP/HTTPS 传输
  - 自动重连机制
  - 错误处理和日志

### 示例系统功能

- 🤖 **智能调度**：Orchestrator 自动识别用户意图
- 🧮 **数学计算**：Math Agent 处理各类数学问题
- 💾 **历史记忆**：跨进程共享对话历史
- 🔄 **上下文理解**：支持多轮对话
- 🔍 **动态发现**：自动发现和选择可用的 Agent

## 🚀 快速开始

### 1. 安装依赖

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libhiredis-dev \
    redis-server
```

### 2. 编译项目
```bash
项目代码中"own—key"都换成自己的api_key
```

```bash
# 进入项目目录
cd /home/ubuntu/a2a-cpp

# 创建并进入 build 目录
mkdir -p build && cd build

# 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译（使用单线程避免内存不足）
make -j1
```

**编译时间：** 约 5-8 分钟

**验证编译成功：**
```bash
$ ls -lh examples/multi_agent_demo/

-rwxrwxr-x redis_orchestrator       # 固定地址 Orchestrator
-rwxrwxr-x redis_math_agent          # 固定地址 Math Agent
-rwxrwxr-x dynamic_orchestrator      # 动态服务发现 Orchestrator
-rwxrwxr-x dynamic_math_agent        # 动态服务发现 Math Agent
-rwxrwxr-x registry_server           # 注册中心服务器
-rwxrwxr-x interactive_client        # 交互式客户端
```

### 3. 运行系统

#### 方式 A：固定地址系统（简单）

适合固定的、已知的 Agent 部署场景。

```bash
cd /home/ubuntu/a2a-cpp/examples/multi_agent_demo

# 启动系统
./start_redis_system.sh
```

**系统组件：**
- Orchestrator (端口 5000)
- Math Agent (端口 5001)
- Redis (端口 6379)

#### 方式 B：动态服务发现系统（推荐）

适合 Agent 数量和地址不固定的场景。

```bash
cd /home/ubuntu/a2a-cpp/examples/multi_agent_demo

# 启动系统
./start_dynamic_system.sh
```

**系统组件：**
- Registry Server (端口 8500) - 注册中心
- Orchestrator (端口 5000)
- Math Agent 1 (端口 5001)
- Math Agent 2 (端口 5002) - 演示负载均衡
- Redis (端口 6379)

### 4. 测试系统

#### 自动化测试

```bash
./test_redis_system.sh
```

**测试内容：**
- ✅ 基本计算功能
- ✅ 上下文理解能力
- ✅ 跨进程历史共享
- ✅ Redis 数据持久化

**预期输出：**
```
✅ 测试通过！
   - Orchestrator 成功保存历史到 Redis
   - Math Agent 成功从 Redis 获取历史
   - 跨进程历史共享正常工作
   - 历史消息数量: 6 (预期 >= 6)
```

#### 交互式测试（推荐）

```bash
./chat.sh
```

**使用示例：**
```
========================================
   A2A 交互式测试客户端
========================================

会话 ID: session-1701234567

提示：
  - 输入您的问题，按回车发送
  - 输入 'quit' 或 'exit' 退出
  - 输入 'new' 开始新的会话
  - 输入 'clear' 清屏

----------------------------------------

您> 计算 15 + 27

Agent> 42

您> 上面的答案乘以 2

Agent> 84

您> 求解方程 3x + 7 = 22

Agent> x = 5

您> quit

感谢使用！再见！
```

### 5. 查看系统状态

```bash
# 查看进程
ps aux | grep -E "(redis_orchestrator|redis_math_agent|registry_server)"

# 查看日志
tail -f logs/redis_orchestrator.log
tail -f logs/redis_math_agent.log
tail -f logs/registry_server.log

# 查看 Redis 中的历史数据
redis-cli KEYS "a2a:*"
redis-cli LRANGE "a2a:history:my-session" 0 -1

# 查看注册的 Agent（动态系统）
curl http://localhost:8500/v1/agents | jq
```

### 6. 停止系统

```bash
# 停止固定地址系统
pkill -f redis_orchestrator
pkill -f redis_math_agent

# 停止动态服务发现系统
pkill -f registry_server
pkill -f dynamic_orchestrator
pkill -f dynamic_math_agent
```

## 🧪 测试场景

### 场景 1：基本计算

```
您> 1+1
Agent> 2

您> 计算 123 * 456
Agent> 56088
```

### 场景 2：上下文理解

```
您> 5 + 3
Agent> 8

您> 上面的答案乘以 10
Agent> 80  # ✅ 记住了上一轮的答案

您> 再除以 4
Agent> 20  # ✅ 继续理解上下文
```

### 场景 3：方程求解

```
您> 求解方程 2x + 5 = 15
Agent> x = 5

您> 验证一下
Agent> 将 x=5 代入：2*5 + 5 = 15 ✓
```

### 场景 4：服务发现（动态系统）

```bash
# 查看注册的 Agent
$ curl http://localhost:8500/v1/agents | jq

{
  "success": true,
  "agents": [
    {
      "id": "orch-1",
      "name": "Orchestrator",
      "address": "http://localhost:5000",
      "tags": ["orchestrator", "coordinator"]
    },
    {
      "id": "math-1",
      "name": "Math Agent",
      "address": "http://localhost:5001",
      "tags": ["math", "calculator"]
    },
    {
      "id": "math-2",
      "name": "Math Agent",
      "address": "http://localhost:5002",
      "tags": ["math", "calculator"]
    }
  ],
  "count": 3
}
```

## 🔧 常见问题

### Q: 编译时内存不足怎么办？

**A:** 使用单线程编译（`-j1`）：
```bash
make -j1
```

### Q: 如何验证系统是否正常运行？

**A:** 检查进程和端口：
```bash
# 检查进程
ps aux | grep -E "(redis_|registry_|dynamic_)"

# 检查端口
sudo netstat -tlnp | grep -E "(5000|5001|5002|6379|8500)"

# 测试 Orchestrator
curl http://localhost:5000/.well-known/agent-card.json
```

### Q: Redis 连接失败怎么办？

**A:** 确保 Redis 服务运行：
```bash
# 启动 Redis
sudo systemctl start redis-server

# 验证
redis-cli ping  # 应返回 PONG
```

### Q: 如何清空历史数据？

**A:** 清空 Redis 数据库：
```bash
redis-cli FLUSHDB
```

### Q: 如何添加新的 Agent？

**A:** 
1. 参考 `dynamic_math_agent.cpp` 编写新的 Agent
2. 在 `CMakeLists.txt` 中添加编译目标
3. 启动时注册到注册中心
4. Orchestrator 会自动发现新的 Agent

### Q: 如何抓取网络包进行调试？

**A:** 使用 tcpdump：
```bash
# 抓取所有相关端口的包
sudo tcpdump -i any -s 0 -w a2a_traffic.pcap \
  'port 5000 or port 5001 or port 5002 or port 6379 or port 8500' -v

# 查看抓包文件
tcpdump -r a2a_traffic.pcap -A
```

## 🛠️ 技术栈

### 核心依赖

- **C++17** - 现代 C++ 特性
- **CMake 3.15+** - 构建系统
- **libcurl** - HTTP 客户端
- **nlohmann/json** - JSON 解析
- **hiredis** - Redis 客户端
- **redis-server** - Redis 数据库

### AI 服务

- **阿里百炼平台** - 通义千问 AI 模型

## 📊 系统对比

| 特性 | 固定地址系统 | 动态服务发现系统 |
|------|-------------|-----------------|
| **部署复杂度** | 简单 | 中等 |
| **服务发现** | 硬编码地址 | 自动发现 |
| **负载均衡** | 不支持 | 支持（轮询） |
| **Agent 扩展** | 需修改代码 | 动态添加 |
| **健康检查** | 无 | 自动检查 |
| **适用场景** | 固定部署 | 动态扩展 |

## 项目作用
本项目是下周即将上线的AI智能体项目的一个子项目，AI智能体项目将会更加给人带来惊叹

## 📝 License

Apache 2.0

## 🙏 致谢

基于 A2A 协议标准实现。

---

**开始使用 A2A C++ SDK 构建您的智能 Agent 系统！** 🚀

## owner 介绍
cpp辅导的阿甘，“奔跑中的cpp / c++”知识星球的创始人，垂直cpp相关领域的辅导
vx： LLqueww
里面服务也不会变，四个坚守目前:

1.每天都会看大家打卡内容，给出合理性建议。

2.大家如果需要简历指导，心里迷茫需要疏导都可以进行预约周六一对一辅导。

3.每周五晚上九点答疑聊天不会变。

4.进去星球了，后续如果有什么其他活动，服务，不收费不收费(可以合理赚钱就收取下星球费用，但是不割韭菜，保持初心)

（还有经历时间考验的独家私密资料）

加入星球的同学都可以提问预约，一对一帮做简历，一对一  职业规划辅导    ，解惑。同时有高质量的项目以及学习资料

学cpp基础，可以把最近开发的这个编程练习平台利用起来
cppagancoding.top

