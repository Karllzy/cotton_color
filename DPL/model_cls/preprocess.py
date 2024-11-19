import argparse
import os
import random
import shutil

import cv2
import numpy as np
import json
from shapely.geometry import Polygon, box
from shapely.affinity import translate


def create_dataset_from_folder(image_folder, label_folder, block_size, output_dir):
    """
    批量处理文件夹中的图片和对应标签，生成分类模型的数据集。

    Args:
        image_folder (str): 图片文件夹路径。
        label_folder (str): 标签文件夹路径，对应Labelme生成的JSON文件。
        block_size (tuple): 分块的尺寸 (width, height)。
        output_dir (str): 输出数据集的根目录。
    """
    # 创建输出文件夹
    has_label_dir = os.path.join(output_dir, "has_label")
    no_label_dir = os.path.join(output_dir, "no_label")
    os.makedirs(has_label_dir, exist_ok=True)
    os.makedirs(no_label_dir, exist_ok=True)

    # 遍历图片文件夹
    for filename in os.listdir(image_folder):
        if filename.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp', '.tif')):
            image_path = os.path.join(image_folder, filename)
            label_path = os.path.join(label_folder, os.path.splitext(filename)[0] + ".json")

            # 检查标签文件是否存在
            if not os.path.exists(label_path):
                print(f"Label file not found for image: {filename}")
                continue

            print(f"Processing {filename}...")
            process_single_image(image_path, label_path, block_size, has_label_dir, no_label_dir)


def process_single_image(image_path, label_path, block_size, has_label_dir, no_label_dir):
    """
    处理单张图片并分块保存到对应的文件夹。

    Args:
        image_path (str): 图片路径。
        label_path (str): 标签路径。
        block_size (tuple): 分块的尺寸 (width, height)。
        has_label_dir (str): 包含标注的分块保存目录。
        no_label_dir (str): 无标注的分块保存目录。
    """
    # 加载图片
    image = cv2.imread(image_path)
    img_height, img_width, _ = image.shape

    # 加载Labelme JSON文件
    with open(label_path, 'r', encoding='utf-8') as f:
        label_data = json.load(f)

    # 提取多边形标注
    polygons = []
    for shape in label_data['shapes']:
        if shape['shape_type'] == 'polygon':
            points = shape['points']
            polygons.append(Polygon(points))

    if roi:
        x_min, y_min, x_max, y_max = roi
        x_min, y_min = max(0, x_min), max(0, y_min)
        x_max, y_max = min(img_width, x_max), min(img_height, y_max)
        image = image[y_min:y_max, x_min:x_max]
        img_height, img_width = y_max - y_min, x_max - x_min
        # 偏移标注的多边形
        polygons = [translate(poly.intersection(box(x_min, y_min, x_max, y_max)), -x_min, -y_min) for poly in polygons]

    # 分割图片并保存到对应的文件夹
    block_width, block_height = block_size
    block_id = 0
    base_name = os.path.splitext(os.path.basename(image_path))[0]
    for y in range(0, img_height, block_height):
        for x in range(0, img_width, block_width):
            # 当前分块的边界框
            block_polygon = box(x, y, x + block_width, y + block_height)

            # 判断是否与任何标注的多边形相交
            contains_label = any(poly.intersects(block_polygon) for poly in polygons)

            # 裁剪当前块
            block = image[y:y + block_height, x:x + block_width]

            # 保存到对应文件夹
            folder = has_label_dir if contains_label else no_label_dir
            block_filename = os.path.join(folder, f"{base_name}_block_{block_id}.jpg")
            cv2.imwrite(block_filename, block)
            block_id += 1

            print(f"Saved {block_filename} to {'has_label' if contains_label else 'no_label'} folder.")


def split_dataset(input_dir, output_dir, train_ratio=0.7, val_ratio=0.2, test_ratio=0.1):
    """
    将分类后的数据集划分为 train、val 和 test 数据集，保持来源目录结构。

    Args:
        input_dir (str): 分类结果根目录，包含多个子文件夹（标签文件夹）。
        output_dir (str): 输出数据集目录，将生成 train、val 和 test 文件夹。
        train_ratio (float): train 集比例或固定数量。
        val_ratio (float): val 集比例或固定数量。
        test_ratio (float): test 集比例或固定数量。
    """
    # 定义输出子目录
    train_dir = os.path.join(output_dir, "train")
    val_dir = os.path.join(output_dir, "val")
    test_dir = os.path.join(output_dir, "test")

    # 遍历所有子文件夹（标签文件夹）
    for category in os.listdir(input_dir):
        category_dir = os.path.join(input_dir, category)
        if not os.path.isdir(category_dir):  # 忽略非文件夹
            continue

        # 为当前类别在 train/val/test 创建相同的子文件夹结构
        os.makedirs(os.path.join(train_dir, category), exist_ok=True)
        os.makedirs(os.path.join(val_dir, category), exist_ok=True)
        os.makedirs(os.path.join(test_dir, category), exist_ok=True)

        # 获取当前类别下的所有文件
        files = os.listdir(category_dir)
        random.shuffle(files)

        # 计算分割点
        total_files = len(files)
        if train_ratio < 1:
            train_count = int(total_files * train_ratio)
            val_count = int(total_files * val_ratio)
            test_count = total_files - train_count - val_count
        else:
            train_count = int(train_ratio)
            val_count = int(val_ratio)
            test_count = int(test_ratio)

        # 确保不超过文件总数
        train_count = min(train_count, total_files)
        val_count = min(val_count, total_files - train_count)
        test_count = min(test_count, total_files - train_count - val_count)

        print(f"Category {category}: {train_count} train, {val_count} val, {test_count} test files")

        # 划分数据集
        train_files = files[:train_count]
        val_files = files[train_count:train_count + val_count]
        test_files = files[train_count + val_count:]

        # 复制文件到对应的子目录
        for file in train_files:
            shutil.copy(os.path.join(category_dir, file), os.path.join(train_dir, category, file))
        for file in val_files:
            shutil.copy(os.path.join(category_dir, file), os.path.join(val_dir, category, file))
        for file in test_files:
            shutil.copy(os.path.join(category_dir, file), os.path.join(test_dir, category, file))

        print(f"Category {category} processed. Train: {len(train_files)}, Val: {len(val_files)}, Test: {len(test_files)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Preprocess images to required shapes")

    # 设置默认值，并允许用户通过命令行进行修改
    parser.add_argument('--img-dir', type=str, default=r'..\dataset\dgd\test_img', help='Directory to input images')
    parser.add_argument('--label-dir', type=str, default=r'..\dataset\dgd\test_img', help='Directory to input labels')

    parser.add_argument('--output-dir', type=str, default=r'..\dataset\dgd\runs', help='Directory to save output images')
    parser.add_argument('--roi', type=int, nargs=4, default=None, help='ROI region (x_min y_min x_max y_max)')
    parser.add_argument('--train-ratio', type=float, default=0.7, help='Train set ratio or count')
    parser.add_argument('--val-ratio', type=float, default=0.2, help='Validation set ratio or count')
    parser.add_argument('--test-ratio', type=float, default=0.1, help='Test set ratio or count')

    args = parser.parse_args()

    # 输入文件夹路径
    image_folder = args.img_dir
    label_folder = args.label_dir
    # 输出路径
    output_dir = args.output_dir

    # 分块大小
    block_size = (170, 170)  # 替换为希望的分块尺寸 (宽, 高)
    roi = tuple(args.roi) if args.roi else None
    all_output = os.path.join(output_dir, 'all')

    # 批量生成数据集
    create_dataset_from_folder(image_folder, label_folder, block_size, all_output)
    split_dataset(all_output, output_dir, args.train_ratio, args.val_ratio, args.test_ratio)