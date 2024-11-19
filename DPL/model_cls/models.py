from typing import Optional
from torchvision import transforms
import numpy as np
import onnxruntime as ort
import torch

# 模型类
class Model:
    def __init__(self, model_path: str, device: torch.device, block_size: Optional[tuple] = None):
        """
        初始化模型，加载 ONNX 模型，并设置设备（CPU 或 GPU）。
        """
        self.device = device
        self.session = ort.InferenceSession(model_path)
        self.block_size = block_size
        self.data_transforms = transforms.Compose([
            transforms.Resize(256),
            transforms.CenterCrop(224),
            transforms.ToTensor(),
            transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
        ])

    def predict(self, img: np.ndarray) -> torch.Tensor:
        """
        使用 ONNX 模型进行推理，返回预测结果。
        """
        img = self.data_transforms(img)
        img = img.unsqueeze(0)

        # # 转换为 ONNX 输入格式
        img_numpy = img.cpu().numpy()

        inputs = {self.session.get_inputs()[0].name: img_numpy}
        outputs = self.session.run(None, inputs)
        pred = torch.tensor(outputs[0])
        _, predicted_class = torch.max(pred, 1)

        return predicted_class

    def load(self, model_path: str):
        """
        重新加载模型。
        """
        self.session = ort.InferenceSession(model_path, providers=['TensorrtExecutionProvider'])

    def preprocess_predict_img(self, img: np.ndarray, block_size: Optional[tuple] = None) -> np.ndarray:
        if block_size is None:
            block_size = self.block_size
        if block_size:
            img = img.reshape((self.block_size[0], -1, self.block_size[1], 3))
            img = img.transpose((1, 0, 2, 3))
        else:
            img = img[np.newaxis, ...]

        pred = self.predict(img)
        pred = pred.squeeze().numpy()
        return pred
