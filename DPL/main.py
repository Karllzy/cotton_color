import argparse
import os
import sys

import torch

from model_cls.models import Model as ClsModel

from pathlib import Path


FILE = Path(__file__).resolve()
ROOT = FILE.parents[0]  # YOLOv5 root directory
if str(ROOT) not in sys.path:
    sys.path.append(str(ROOT))  # add ROOT to PATH
ROOT = Path(os.path.relpath(ROOT, Path.cwd()))  # relative

def main():
    # 命令行参数解析
    parser = argparse.ArgumentParser(description="Use an ONNX model for inference.")

    # 设置默认值，并允许用户通过命令行进行修改
    parser.add_argument('--input-dir', type=str, default='dataset/', help='Directory to input images')
    parser.add_argument('--save-dir', type=str, default='detect', help='Directory to save output images')
    parser.add_argument('--gpu', action='store_true', help='Use GPU for inference')

    args = parser.parse_args()

    # 设置设备
    device = torch.device('cuda' if args.gpu and torch.cuda.is_available() else 'cpu')
    if args.gpu and not torch.cuda.is_available():
        print("GPU not available, switching to CPU.")

    # 加载模型
    model = ClsModel(model_path=args.weights, device=device)


if __name__ == '__main__':
    main()