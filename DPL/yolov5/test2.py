import onnx

# 加载ONNX模型
model_path = "runs/train/exp10/weights/best.onnx"  # 替换为实际模型路径
model = onnx.load(model_path)

# 输出运算符集版本信息
for opset in model.opset_import:
    print(f"Domain: {opset.domain if opset.domain else 'ai.onnx'}, Version: {opset.version}")



