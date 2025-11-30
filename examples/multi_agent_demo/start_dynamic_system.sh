#!/bin/bash

# 启动动态服务发现系统

REGISTRY_URL="http://localhost:8500"
REDIS_HOST="127.0.0.1"
REDIS_PORT="6379"

# 创建日志目录
mkdir -p logs
mkdir -p pids

echo "========================================"
echo "   启动动态服务发现系统"
echo "========================================"
echo ""

# 1. 启动注册中心
echo "[1/4] 启动注册中心 (端口 8500)..."
nohup ../../build/examples/multi_agent_demo/registry_server > logs/registry_server.log 2>&1 &
REGISTRY_PID=$!
echo $REGISTRY_PID > pids/registry_server.pid
sleep 2

# 2. 启动 Orchestrator
echo "[2/4] 启动 Orchestrator (端口 5000)..."
nohup ../../build/examples/multi_agent_demo/dynamic_orchestrator \
    orch-1 5000 $REGISTRY_URL $REDIS_HOST $REDIS_PORT \
    > logs/dynamic_orchestrator.log 2>&1 &
ORCH_PID=$!
echo $ORCH_PID > pids/dynamic_orchestrator.pid
sleep 2

# 3. 启动 Math Agent 实例 1
echo "[3/4] 启动 Math Agent 1 (端口 5001)..."
nohup ../../build/examples/multi_agent_demo/dynamic_math_agent \
    math-1 5001 $REGISTRY_URL $REDIS_HOST $REDIS_PORT \
    > logs/dynamic_math_agent_1.log 2>&1 &
MATH1_PID=$!
echo $MATH1_PID > pids/dynamic_math_agent_1.pid
sleep 2

# 4. 启动 Math Agent 实例 2（演示负载均衡）
echo "[4/4] 启动 Math Agent 2 (端口 5002)..."
nohup ../../build/examples/multi_agent_demo/dynamic_math_agent \
    math-2 5002 $REGISTRY_URL $REDIS_HOST $REDIS_PORT \
    > logs/dynamic_math_agent_2.log 2>&1 &
MATH2_PID=$!
echo $MATH2_PID > pids/dynamic_math_agent_2.pid
sleep 2

echo ""
echo "========================================"
echo "   系统启动完成"
echo "========================================"
echo ""
echo "进程 ID:"
echo "  Registry Server:  $REGISTRY_PID"
echo "  Orchestrator:     $ORCH_PID"
echo "  Math Agent 1:     $MATH1_PID"
echo "  Math Agent 2:     $MATH2_PID"
echo ""
echo "服务地址:"
echo "  Registry:         http://localhost:8500"
echo "  Orchestrator:     http://localhost:5000"
echo "  Math Agent 1:     http://localhost:5001"
echo "  Math Agent 2:     http://localhost:5002"
echo "  Redis:            $REDIS_HOST:$REDIS_PORT"
echo ""
echo "查看日志:"
echo "  tail -f logs/registry_server.log"
echo "  tail -f logs/dynamic_orchestrator.log"
echo "  tail -f logs/dynamic_math_agent_1.log"
echo "  tail -f logs/dynamic_math_agent_2.log"
echo ""
echo "查看注册的 Agent:"
echo "  curl http://localhost:8500/v1/agents | jq"
echo ""
echo "开始聊天:"
echo "  ./chat.sh"
echo ""
