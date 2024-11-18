import os
import cv2
import numpy as np
import time

def read_yolov5_labels(label_folder):
    """
    读取YOLOv5标签文件夹中的标签文件，提取每个框的位置。
    :param label_folder: YOLOv5标签文件夹路径
    :return: 返回一个字典，格式为 {image_name: [(x_center, y_center, width, height), ...]}
    """
    labels = {}

    # 遍历YOLOv5结果文件夹中的所有txt文件
    for filename in os.listdir(label_folder):
        if filename.endswith('.txt'):
            image_name = filename.replace('.txt', '')
            file_path = os.path.join(label_folder, filename)

            # 读取文件并解析检测框
            with open(file_path, 'r') as f:
                boxes = []
                for line in f:
                    parts = line.strip().split()
                    if len(parts) < 5:  # 检查标签格式是否完整
                        continue

                    # 从YOLOv5标签格式解析出数据
                    class_id = int(parts[0])
                    x_center = float(parts[1])
                    y_center = float(parts[2])
                    width = float(parts[3])
                    height = float(parts[4])

                    # 计算框的绝对坐标
                    boxes.append([x_center, y_center, width, height])

                labels[image_name] = boxes

    return labels

def generate_mask(image_shape, boxes, width_blocks=24, height_blocks=24):
    """
    根据检测框信息生成掩膜，返回True和False的矩阵
    :param image_shape: 图像的shape（height, width）
    :param boxes: 检测框信息，格式为 [(x_center, y_center, width, height), ...]
    :param width_blocks: 图像宽度分块数
    :param height_blocks: 图像高度分块数
    :return: 掩膜矩阵 (height, width)，True表示框内区域，False表示其他区域
    """
    height, width = image_shape

    # 创建一个与图像大小相同的全False矩阵
    mask = np.zeros((height, width), dtype=bool)

    # 遍历每个框，更新掩膜矩阵
    for box in boxes:
        x_center, y_center, width_box, height_box = box

        # 转换为绝对坐标
        x1 = int((x_center - width_box / 2) * width)
        y1 = int((y_center - height_box / 2) * height)
        x2 = int((x_center + width_box / 2) * width)
        y2 = int((y_center + height_box / 2) * height)

        # 确保框不超出图像边界
        x1 = max(0, x1)
        y1 = max(0, y1)
        x2 = min(width, x2)
        y2 = min(height, y2)

        # 将检测框区域标记为True
        mask[y1:y2, x1:x2] = True

    return mask

def process_images_and_generate_masks(image_folder, label_folder, output_folder, width_blocks=24, height_blocks=24):
    """
    处理图像文件夹，生成掩膜并保存为True和False的矩阵
    :param image_folder: 输入图像文件夹路径
    :param label_folder: YOLOv5标签文件夹路径
    :param output_folder: 输出掩膜矩阵保存文件夹路径
    :param width_blocks: 图像宽度分块数
    :param height_blocks: 图像高度分块数
    :return: None
    """
    # 确保输出文件夹存在
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # 读取YOLOv5标签文件
    labels = read_yolov5_labels(label_folder)

    # 遍历图像文件夹中的图片
    for filename in os.listdir(image_folder):
        if filename.endswith(('.jpg', '.png', '.bmp')):
            image_path = os.path.join(image_folder, filename)
            image_name = filename.replace('.jpg', '').replace('.png', '').replace('.bmp', '')

            # 获取对应的标签框
            boxes = labels.get(image_name, [])
            if not boxes:
                print(f"未找到检测框信息：{image_name}")
                continue

            # 读取图片
            image = cv2.imread(image_path)
            if image is None:
                print(f"无法读取图片: {image_path}")
                continue

            # 获取图像的尺寸 (height, width)
            height, width = image.shape[:2]

            # 记录处理时间
            start_time = time.time()

            # 生成掩膜
            mask = generate_mask((height, width), boxes, width_blocks, height_blocks)

            # 计算处理时间
            processing_time = time.time() - start_time
            print(f"处理图片 {image_name} 耗时: {processing_time:.4f}秒")

            # 保存掩膜矩阵到文件
            mask_filename = f"{image_name}_mask.npy"
            mask_path = os.path.join(output_folder, mask_filename)
            np.save(mask_path, mask)
            print(f"保存掩膜: {mask_filename}")

# 测试代码
if __name__ == "__main__":
    image_folder = "runs/detect/exp6"  # 输入图像文件夹路径
    label_folder = "runs/detect/labels"  # YOLOv5标签文件夹路径
    output_folder = "datasets/mask"  # 输出掩膜矩阵保存文件夹路径

    process_images_and_generate_masks(image_folder, label_folder, output_folder)
