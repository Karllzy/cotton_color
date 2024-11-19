import argparse
import torch
from torchvision import models
from torch import nn
import os
import re


def load_pytorch_model(model_path, num_classes, device):
    """
    加载 PyTorch 模型，并设置最后的分类层。
    """
    print(f"Loading PyTorch model from {model_path}...")
    model = models.resnet18(weights=None)  # 使用 ResNet18 作为示例
    model.fc = nn.Linear(model.fc.in_features, num_classes)  # 修改最后一层
    model.load_state_dict(torch.load(model_path, map_location=device, weights_only=True))
    model.to(device)  # 将模型加载到指定设备
    model.eval()
    print("PyTorch model loaded successfully.")
    return model


def export_to_onnx(model, onnx_path, img_size, batch_size, device):
    """
    导出 PyTorch 模型为 ONNX 格式，自动递增文件名，并支持 GPU。
    """
    # 确保 `onnx_path` 是一个具体的文件路径
    if not onnx_path.endswith('.onnx'):
        os.makedirs(onnx_path, exist_ok=True)  # 创建文件夹
        base_dir = onnx_path
    else:
        base_dir = os.path.dirname(onnx_path) or '.'  # 提取文件夹部分
        os.makedirs(base_dir, exist_ok=True)

    # 自动递增文件名
    base_name = "model"
    extension = ".onnx"
    existing_files = [f for f in os.listdir(base_dir) if f.endswith(extension)]

    # 使用正则匹配现有文件名
    pattern = re.compile(rf"^{base_name}_(\d+){extension}$")
    numbers = [
        int(match.group(1)) for f in existing_files if (match := pattern.match(f))
    ]
    next_number = max(numbers, default=0) + 1  # 计算下一个编号
    final_name = f"{base_name}_{next_number}{extension}"
    final_path = os.path.join(base_dir, final_name)

    print(f"Exporting model to ONNX format at {final_path}...")

    # 创建虚拟输入张量，并将其移动到指定设备
    dummy_input = torch.randn(batch_size, 3, img_size, img_size, device=device)

    # 导出 ONNX
    torch.onnx.export(
        model,
        dummy_input,
        final_path,
        input_names=['input'],
        output_names=['output'],
        opset_version=11,  # ONNX opset 版本
        dynamic_axes={
            'input': {0: 'batch_size'},  # 动态批量维度
            'output': {0: 'batch_size'}
        }
    )
    print(f"Model exported successfully to {final_path}.")


def main():
    parser = argparse.ArgumentParser(description="Export PyTorch model to ONNX format.")
    parser.add_argument('--weights', type=str, required=True, help='Path to PyTorch model weights (.pt file)')
    parser.add_argument('--onnx-path', type=str, default='onnx', help='Output path for ONNX model')
    parser.add_argument('--img-size', type=int, default=224, help='Input image size (default: 224)')
    parser.add_argument('--batch-size', type=int, default=8, help='Input batch size (default: 1)')
    parser.add_argument('--num-classes', type=int, default=2, help='Number of classes in the model (default: 1000)')
    parser.add_argument('--use-gpu', action='store_true', help='Enable GPU support during export')

    args = parser.parse_args()

    # 设置设备
    device = torch.device('cuda' if args.use_gpu and torch.cuda.is_available() else 'cpu')
    if args.use_gpu and not torch.cuda.is_available():
        print("GPU not available, switching to CPU.")

    # 检查权重文件是否存在
    if not os.path.isfile(args.weights):
        raise FileNotFoundError(f"Model weights file not found: {args.weights}")

    # 加载模型
    model = load_pytorch_model(args.weights, args.num_classes, device)

    # 导出为 ONNX
    export_to_onnx(model, args.onnx_path, args.img_size, args.batch_size, device)


if __name__ == "__main__":
    main()
