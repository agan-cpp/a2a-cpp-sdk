#!/bin/bash

# 交互式聊天脚本

echo "正在检查系统状态..."

# 检查 Orchestrator 是否运行（支持两种系统）
ORCH_RUNNING=false
if pgrep -f redis_orchestrator > /dev/null; then
    ORCH_RUNNING=true
    SYSTEM_TYPE="固定地址系统"
elif pgrep -f dynamic_orchestrator > /dev/null; then
    ORCH_RUNNING=true
    SYSTEM_TYPE="动态服务发现系统"
fi

if [ "$ORCH_RUNNING" = false ]; then
    echo "❌ Orchestrator 未运行"
    echo ""
    echo "请先启动系统："
    echo "  固定地址系统:         ./start_redis_system.sh"
    echo "  动态服务发现系统:     ./start_dynamic_system.sh"
    exit 1
fi

# 检查 Math Agent 是否运行（支持两种系统）
MATH_RUNNING=false
if pgrep -f redis_math_agent > /dev/null; then
    MATH_RUNNING=true
elif pgrep -f dynamic_math_agent > /dev/null; then
    MATH_RUNNING=true
fi

if [ "$MATH_RUNNING" = false ]; then
    echo "❌ Math Agent 未运行"
    echo ""
    echo "请先启动系统："
    echo "  固定地址系统:         ./start_redis_system.sh"
    echo "  动态服务发现系统:     ./start_dynamic_system.sh"
    exit 1
fi

echo "✅ 系统运行正常 ($SYSTEM_TYPE)"
echo ""

# 启动交互式客户端
../../build/examples/multi_agent_demo/interactive_client
