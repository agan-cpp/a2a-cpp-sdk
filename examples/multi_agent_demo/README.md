# Multi-Agent Demo

多 Agent 协同系统示例，展示基于 Redis 的分布式 TaskStore 实现。

## 系统架构

```
┌─────────────┐      ┌─────────────┐
│ Orchestrator│      │ Math Agent  │
│  (进程 1)    │      │  (进程 2)    │
│  端口 5000   │      │  端口 5001   │
└──────┬──────┘      └──────┬──────┘
       │                    │
       └────────┬───────────┘
                │
         ┌──────▼──────┐
         │    Redis    │
         │ (TaskStore) │
         │  端口 6379   │
         └─────────────┘
```

## 快速开始

### 1. 启动系统

```bash
./start_redis_system.sh
```

### 2. 运行测试

```bash
./test_redis_system.sh
```

### 3. 手动测试

```bash
# 第一轮对话
curl -X POST http://localhost:5000/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": "1",
    "method": "message/send",
    "params": {
      "message": {
        "role": "user",
        "contextId": "ctx-test",
        "parts": [{"kind": "text", "text": "1+1"}]
      }
    }
  }' | jq -r '.result.parts[0].text'

# 第二轮对话（测试上下文）
curl -X POST http://localhost:5000/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "id": "2",
    "method": "message/send",
    "params": {
      "message": {
        "role": "user",
        "contextId": "ctx-test",
        "parts": [{"kind": "text", "text": "上面答案+1"}]
      }
    }
  }' | jq -r '.result.parts[0].text'
```

## 查看日志

```bash
# 实时查看日志
tail -f logs/redis_orchestrator.log
tail -f logs/redis_math_agent.log
```

## 查看 Redis 数据

```bash
# 查看所有 Key
redis-cli KEYS "a2a:*"

# 查看历史消息
redis-cli LRANGE "a2a:history:ctx-test" 0 -1
```

## 停止服务

```bash
pkill -f redis_orchestrator
pkill -f redis_math_agent
```

## 核心特性

- ✅ 独立进程部署
- ✅ Redis 持久化存储
- ✅ 跨进程历史共享
- ✅ 上下文理解
- ✅ 自动化测试

## 详细文档

查看项目根目录的文档：
- [架构说明.md](../../架构说明.md) - 项目架构和设计详解
- [使用指南.md](../../使用指南.md) - 编译、运行和开发指南
