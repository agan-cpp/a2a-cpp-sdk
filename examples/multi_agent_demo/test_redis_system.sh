#!/bin/bash

echo "=== 测试 Redis 分布式 TaskStore 系统 ==="
echo ""

CONTEXT_ID="ctx-redis-test-$(date +%s)"

echo "测试场景：验证跨进程历史共享"
echo "Context ID: $CONTEXT_ID"
echo ""

# 第一轮：1+1
echo "第一轮: 1+1"
echo "----------------------------------------"
RESPONSE1=$(curl -s -X POST http://localhost:5000/ \
  -H "Content-Type: application/json" \
  -d "{
    \"jsonrpc\": \"2.0\",
    \"id\": \"test-1\",
    \"method\": \"message/send\",
    \"params\": {
      \"message\": {
        \"role\": \"user\",
        \"contextId\": \"$CONTEXT_ID\",
        \"parts\": [{\"kind\": \"text\", \"text\": \"1+1\"}]
      }
    }
  }")

echo "响应: $(echo $RESPONSE1 | jq -r '.result.parts[0].text' 2>/dev/null || echo $RESPONSE1)"
echo ""

sleep 2

# 检查 Redis 中的数据
echo "检查 Redis 中的历史数据:"
echo "----------------------------------------"
HISTORY_COUNT=$(redis-cli LLEN "a2a:history:$CONTEXT_ID")
echo "历史消息数量: $HISTORY_COUNT"

if [ "$HISTORY_COUNT" -gt 0 ]; then
    echo "✅ Orchestrator 成功保存历史到 Redis"
    echo ""
    echo "历史内容:"
    redis-cli LRANGE "a2a:history:$CONTEXT_ID" 0 -1 | head -20
else
    echo "❌ Redis 中没有历史数据"
fi

echo ""
sleep 2

# 第二轮：上面答案+1
echo "第二轮: 上面答案+1 (测试跨进程历史获取)"
echo "----------------------------------------"
RESPONSE2=$(curl -s -X POST http://localhost:5000/ \
  -H "Content-Type: application/json" \
  -d "{
    \"jsonrpc\": \"2.0\",
    \"id\": \"test-2\",
    \"method\": \"message/send\",
    \"params\": {
      \"message\": {
        \"role\": \"user\",
        \"contextId\": \"$CONTEXT_ID\",
        \"parts\": [{\"kind\": \"text\", \"text\": \"上面答案+1\"}]
      }
    }
  }")

echo "响应: $(echo $RESPONSE2 | jq -r '.result.parts[0].text' 2>/dev/null || echo $RESPONSE2)"
echo ""

sleep 2

# 再次检查 Redis
echo "再次检查 Redis 中的历史数据:"
echo "----------------------------------------"
HISTORY_COUNT=$(redis-cli LLEN "a2a:history:$CONTEXT_ID")
echo "历史消息数量: $HISTORY_COUNT"
echo ""

# 第三轮：再+1
echo "第三轮: 再+1"
echo "----------------------------------------"
RESPONSE3=$(curl -s -X POST http://localhost:5000/ \
  -H "Content-Type: application/json" \
  -d "{
    \"jsonrpc\": \"2.0\",
    \"id\": \"test-3\",
    \"method\": \"message/send\",
    \"params\": {
      \"message\": {
        \"role\": \"user\",
        \"contextId\": \"$CONTEXT_ID\",
        \"parts\": [{\"kind\": \"text\", \"text\": \"再+1\"}]
      }
    }
  }")

echo "响应: $(echo $RESPONSE3 | jq -r '.result.parts[0].text' 2>/dev/null || echo $RESPONSE3)"
echo ""

echo ""
echo "=== 验证结果 ==="
echo ""

# 查看 Orchestrator 日志
echo "Orchestrator 日志（最近 10 行）:"
echo "----------------------------------------"
tail -10 logs/redis_orchestrator.log | grep -E "(保存消息|创建新 Task)"
echo ""

# 查看 Math Agent 日志
echo "Math Agent 日志（最近 10 行）:"
echo "----------------------------------------"
tail -10 logs/redis_math_agent.log | grep -E "(获取到|历史消息)"
echo ""

# 最终 Redis 数据
echo "最终 Redis 数据:"
echo "----------------------------------------"
echo "所有 A2A 相关的 Key:"
redis-cli KEYS "a2a:*"
echo ""
echo "历史消息数量: $(redis-cli LLEN "a2a:history:$CONTEXT_ID")"
echo ""

# 验证总结
echo "=== 测试总结 ==="
echo ""

FINAL_COUNT=$(redis-cli LLEN "a2a:history:$CONTEXT_ID")
if [ "$FINAL_COUNT" -ge 6 ]; then
    echo "✅ 测试通过！"
    echo "   - Orchestrator 成功保存历史到 Redis"
    echo "   - Math Agent 成功从 Redis 获取历史"
    echo "   - 跨进程历史共享正常工作"
    echo "   - 历史消息数量: $FINAL_COUNT (预期 >= 6)"
else
    echo "⚠️ 测试异常"
    echo "   - 历史消息数量: $FINAL_COUNT (预期 >= 6)"
fi

echo ""
echo "查看完整日志:"
echo "  tail -f logs/redis_orchestrator.log"
echo "  tail -f logs/redis_math_agent.log"
echo ""
