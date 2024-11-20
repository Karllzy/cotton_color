import os
import cv2
import numpy as np
import time

class Model:
    def __init__(self, image_folder, label_folder, output_folder, width_blocks=24, height_blocks=24):
        """
        初始化 Model 类，并自动生成并保存掩膜
        :param image_folder: 输入图像文件夹路径
        :param label_folder: YOLOv5标签文件夹路径
        :param output_folder: 输出掩膜矩阵保存文件夹路径
        :param width_blocks: 图像宽度分块数
        :param height_blocks: 图像高度分块数
        """
        self.image_folder = image_folder
        self.label_folder = label_folder
        self.output_folder = output_folder
        self.width_blocks = width_blocks
        self.height_blocks = height_blocks

        # 确保输出文件夹存在
        if not os.path.exists(self.output_folder):
            os.makedirs(self.output_folder)

        # 自动处理并保存掩膜
        self._process_and_save_masks()

    def _read_yolov5_labels(self):
        """
        读取YOLOv5标签文件夹中的标签文件，提取每个框的位置。
        :return: 返回一个字典，格式为 {image_name: [(x_center, y_center, width, height), ...]}
        """
        labels = {}
        for filename in os.listdir(self.label_folder):
            if filename.endswith('.txt'):
                image_name = filename.replace('.txt', '')
                file_path = os.path.join(self.label_folder, filename)

                with open(file_path, 'r') as f:
                    boxes = []
                    for line in f:
                        parts = line.strip().split()
                        if len(parts) < 5:
                            continue

                        x_center = float(parts[1])
                        y_center = float(parts[2])
                        width = float(parts[3])
                        height = float(parts[4])
                        boxes.append([x_center, y_center, width, height])
                    labels[image_name] = boxes
        return labels

    def _generate_mask(self, image_shape, boxes):
        """
        根据检测框信息生成掩膜，返回True和False的矩阵
        :param image_shape: 图像的shape（height, width）
        :param boxes: 检测框信息，格式为 [(x_center, y_center, width, height), ...]
        :return: 掩膜矩阵
        """
        height, width = image_shape
        mask = np.zeros((height, width), dtype=bool)

        for box in boxes:
            x_center, y_center, width_box, height_box = box
            x1 = int((x_center - width_box / 2) * width)
            y1 = int((y_center - height_box / 2) * height)
            x2 = int((x_center + width_box / 2) * width)
            y2 = int((y_center + height_box / 2) * height)

            x1 = max(0, x1)
            y1 = max(0, y1)
            x2 = min(width, x2)
            y2 = min(height, y2)

            mask[y1:y2, x1:x2] = True

        return mask

    def _process_and_save_masks(self):
        """
        处理图像文件夹，生成掩膜并保存为True和False的矩阵
        """
        labels = self._read_yolov5_labels()

        for filename in os.listdir(self.image_folder):
            if filename.endswith(('.jpg', '.png', '.bmp')):
                image_path = os.path.join(self.image_folder, filename)
                image_name = filename.split('.')[0]

                boxes = labels.get(image_name, [])
                if not boxes:
                    print(f"未找到检测框信息：{image_name}")
                    continue

                image = cv2.imread(image_path)
                if image is None:
                    print(f"无法读取图片: {image_path}")
                    continue

                height, width = image.shape[:2]
                start_time = time.time()
                mask = self._generate_mask((height, width), boxes)
                processing_time = time.time() - start_time
                print(f"处理图片 {image_name} 耗时: {processing_time:.4f}秒")

                mask_filename = f"{image_name}_mask.npy"
                mask_path = os.path.join(self.output_folder, mask_filename)
                np.save(mask_path, mask)
                print(f"保存掩膜: {mask_filename}")

    def get_mask_array(self, image_name):
        """
        返回指定图片的掩膜数组
        :param image_name: 图片名称（不带扩展名）
        :return: 掩膜数组
        """
        mask_path = os.path.join(self.output_folder, f"{image_name}_mask.npy")
        if not os.path.exists(mask_path):
            raise FileNotFoundError(f"掩膜文件未找到: {mask_path}")
        return np.load(mask_path)
