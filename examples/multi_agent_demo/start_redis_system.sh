#!/bin/bash

echo "=== 启动基于 Redis 的分布式 TaskStore 系统 ==="
echo ""

# 停止旧进程
echo "1. 停止旧进程..."
pkill -f redis_orchestrator
pkill -f redis_math_agent
sleep 1

# 检查 Redis 是否运行
echo "2. 检查 Redis 服务..."
if ! pgrep -x redis-server > /dev/null; then
    echo "   启动 Redis..."
    sudo systemctl start redis-server
    sleep 2
fi

# 验证 Redis 连接
if redis-cli ping > /dev/null 2>&1; then
    echo "   ✅ Redis 运行正常"
else
    echo "   ❌ Redis 连接失败"
    exit 1
fi

# 清空 Redis 中的旧数据（可选）
echo "3. 清空旧数据..."
redis-cli FLUSHDB > /dev/null
echo "   ✅ Redis 数据已清空"

# 创建日志目录
mkdir -p logs

# 启动 Orchestrator（独立进程）
echo "4. 启动 Orchestrator (端口 5000)..."
cd ../../build/examples/multi_agent_demo
nohup ./redis_orchestrator 5000 127.0.0.1 6379 > ../../../examples/multi_agent_demo/logs/redis_orchestrator.log 2>&1 &
ORCH_PID=$!
echo "   PID: $ORCH_PID"
sleep 2

# 启动 Math Agent（独立进程）
echo "5. 启动 Math Agent (端口 5001)..."
nohup ./redis_math_agent 5001 127.0.0.1 6379 > ../../../examples/multi_agent_demo/logs/redis_math_agent.log 2>&1 &
MATH_PID=$!
echo "   PID: $MATH_PID"
sleep 2

cd ../../../examples/multi_agent_demo

echo ""
echo "=== 分布式 TaskStore 系统已启动 ===" 
echo ""
echo "进程 ID:"
echo "  Orchestrator: $ORCH_PID"
echo "  Math Agent:   $MATH_PID"
echo ""
echo "服务地址:"
echo "  Orchestrator: http://localhost:5000"
echo "  Math Agent:   http://localhost:5001"
echo "  Redis:        127.0.0.1:6379"
echo ""
echo "日志文件:"
echo "  Orchestrator: logs/redis_orchestrator.log"
echo "  Math Agent:   logs/redis_math_agent.log"
echo ""
echo "测试命令:"
echo "  ./test_redis_system.sh"
echo ""
echo "查看日志:"
echo "  tail -f logs/redis_orchestrator.log"
echo "  tail -f logs/redis_math_agent.log"
echo ""
echo "查看 Redis 数据:"
echo "  redis-cli KEYS 'a2a:*'"
echo "  redis-cli LRANGE 'a2a:history:ctx-123' 0 -1"
echo ""
echo "停止服务:"
echo "  pkill -f redis_orchestrator"
echo "  pkill -f redis_math_agent"
echo ""
