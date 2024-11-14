import torch
from PIL import Image
from torch import nn
import onnx
import onnxruntime as ort  # 用于ONNX推理

from torchvision import datasets, models, transforms
import os
import matplotlib.pyplot as plt

from test import device, class_names, imshow


# 加载已训练的 ONNX 模型
def load_onnx_model(model_path='model/best_model_11.14.19.30.onnx'):
    # 使用 ONNX Runtime 加载模型
    session = ort.InferenceSession(model_path)
    return session


# 预测函数
def visualize_model_predictions(onnx_session, img_path):
    img = Image.open(img_path)
    img = img.convert('RGB')  # 转换为 RGB 模式

    data_transforms = transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ])
    img = data_transforms(img)
    img = img.unsqueeze(0)
    img = img.to(device)

    # 将输入转换为 ONNX 兼容的格式（numpy 数组）
    img = img.cpu().numpy()

    # 使用 ONNX Runtime 进行推理
    inputs = {onnx_session.get_inputs()[0].name: img}
    outputs = onnx_session.run(None, inputs)
    preds = outputs[0]

    # 获取预测类别
    _, predicted_class = torch.max(torch.tensor(preds), 1)

    # 可视化结果
    ax = plt.subplot(2, 2, 1)
    ax.axis('off')
    ax.set_title(f'Predicted: {class_names[predicted_class[0]]}')
    imshow(img[0])


# 使用已训练的 ONNX 模型进行预测
if __name__ == '__main__':
    # 加载 ONNX 模型
    model_path = 'model/best_model_11.14.19.30.onnx'
    onnx_session = load_onnx_model(model_path)

    # 图像路径
    img_path = 'd_2/train/dgd/transformed_1.jpg'  # 更改为你的图像路径
    visualize_model_predictions(onnx_session, img_path)
