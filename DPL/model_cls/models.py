import onnxruntime as ort
import torch

# 模型类
class Model:
    def __init__(self, model_path: str, device: torch.device):
        """
        初始化模型，加载 ONNX 模型，并设置设备（CPU 或 GPU）。
        """
        self.device = device
        self.session = ort.InferenceSession(model_path)

    def predict(self, img_tensor: torch.Tensor) -> torch.Tensor:
        """
        使用 ONNX 模型进行推理，返回预测结果。
        """
        # 转换为 ONNX 输入格式
        img_numpy = img_tensor.cpu().numpy()

        # 获取输入名称和推理
        inputs = {self.session.get_inputs()[0].name: img_numpy}
        outputs = self.session.run(None, inputs)

        return torch.tensor(outputs[0])

    def load(self, model_path: str):
        """
        重新加载模型。
        """
        self.session = ort.InferenceSession(model_path)