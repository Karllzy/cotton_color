import argparse
import torch
import onnxruntime as ort
from PIL import Image
import os
import matplotlib.pyplot as plt
from PIL.ImageDraw import Draw
from PIL import ImageDraw, ImageFont
from torchvision import transforms
import time  # 导入time模块

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


# 预测函数
def visualize_model_predictions(model: Model, img_path: str, save_dir: str, class_names: list):
    """
    预测图像并可视化结果，保存预测后的图片到指定文件夹。
    """
    start_time = time.time()  # 开始时间

    img = Image.open(img_path)
    img = img.convert('RGB')  # 转换为 RGB 模式

    # 图像预处理
    data_transforms = transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ])
    img_transformed = data_transforms(img)
    img_transformed = img_transformed.unsqueeze(0)
    img_transformed = img_transformed.to(model.device)

    # 使用模型进行预测
    preds = model.predict(img_transformed)

    # 获取预测类别
    _, predicted_class = torch.max(preds, 1)

    # 在图像上添加预测结果文本
    predicted_label = class_names[predicted_class[0]]

    # 在图片上绘制文本
    img_with_text = img.copy()
    draw = ImageDraw.Draw(img_with_text)
    font = ImageFont.load_default()  # 可以根据需要更改字体
    text = f'Predicted: {predicted_label}'
    text_position = (10, 10)  # 文本的位置，可以根据需要调整
    draw.text(text_position, text, font=font, fill=(0, 255, 0))  # 白色文字

    # 显示结果图片
    img_with_text.show()

    # 保存预测后的图像
    if not os.path.exists(save_dir):
        os.makedirs(save_dir)

    # 获取文件名和保存路径
    img_name = os.path.basename(img_path)
    save_path = os.path.join(save_dir, f"pred_{img_name}")

    # 保存图片
    img_with_text.save(save_path)
    print(f"Prediction saved at: {save_path}")

    end_time = time.time()  # 结束时间
    processing_time = (end_time - start_time) * 1000  # 转换为毫秒
    print(f"Time taken to process {img_name}: {processing_time:.2f} ms")  # 打印每张图片的处理时间（毫秒）


def process_image_folder(model: Model, folder_path: str, save_dir: str, class_names: list):
    """
    处理文件夹中的所有图像，并预测每张图像的类别。
    """
    start_time = time.time()  # 记录总开始时间

    # 获取文件夹中所有图片文件
    image_files = [f for f in os.listdir(folder_path) if f.lower().endswith(('.png', '.jpg', '.jpeg'))]

    # 遍历文件夹中的所有图像
    for image_file in image_files:
        img_path = os.path.join(folder_path, image_file)
        print(f"Processing image: {img_path}")
        visualize_model_predictions(model, img_path, save_dir, class_names)

    end_time = time.time()  # 记录总结束时间
    total_time = (end_time - start_time) * 1000  # 转换为毫秒
    print(f"Total time taken to process all images: {total_time:.2f} ms")  # 打印总处理时间（毫秒）


def main():
    # 命令行参数解析
    parser = argparse.ArgumentParser(description="Use an ONNX model for inference.")

    # 设置默认值，并允许用户通过命令行进行修改
    parser.add_argument('--weights', type=str, default='model/model_1.onnx', help='Path to ONNX model file')
    parser.add_argument('--img-path', type=str, default='d_2/val/dgd',
                        help='Path to image or folder for inference')
    parser.add_argument('--save-dir', type=str, default='detect', help='Directory to save output images')
    parser.add_argument('--gpu', action='store_true', help='Use GPU for inference')

    args = parser.parse_args()

    # 设置设备
    device = torch.device('cuda' if args.gpu and torch.cuda.is_available() else 'cpu')
    if args.gpu and not torch.cuda.is_available():
        print("GPU not available, switching to CPU.")

    # 加载模型
    model = Model(model_path=args.weights, device=device)

    # 模拟加载 class_names
    # 假设模型类别数量为2
    class_names = ['class_0', 'class_1']

    # 检查输入路径是否为文件夹
    if os.path.isdir(args.img_path):
        # 如果是文件夹，处理文件夹中的所有图片
        process_image_folder(model, args.img_path, args.save_dir, class_names)
    else:
        # 如果是单个图片文件，进行预测
        visualize_model_predictions(model, args.img_path, args.save_dir, class_names)


if __name__ == "__main__":
    main()
