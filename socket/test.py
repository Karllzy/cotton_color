import socket
import time

import cv2
import numpy as np
#
class VirtualValve:
    def __init__(self, host, port, column_num=64):
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 声明socket类型，同时生成链接对象
        self.client.connect((host, port))  # 建立一个链接，连接到本地的13452端口
        self.column_num = column_num

    @staticmethod
    def decode_data_to_matrix(data, column_num):
        """
        将接收到的数据解码为矩阵形式
        :param data: 原始字节数据
        :param column_num: 列数，用于解码为矩阵
        :return: 解码后的矩阵
        """
        try:
            # 假设数据中包含字节打包的矩阵
            unpacked_data = np.frombuffer(data, dtype=np.uint8)  # 转换为无符号8位整型数组
            unpacked_bits = np.unpackbits(unpacked_data)  # 将字节解压为位数组

            # 计算行数（确保数据长度是列数的倍数）
            row_num = len(unpacked_bits) // column_num
            if len(unpacked_bits) % column_num != 0:
                print("数据长度不是列数的整数倍，可能存在错误。")
                return None

            # 转换为矩阵
            matrix = unpacked_bits.reshape((row_num, column_num))
            return matrix
        except Exception as e:
            print(f"解码数据为矩阵时发生错误: {e}")
            return None

    @staticmethod
    def display_matrix_as_image(matrix, cell_size=20):
        """
        显示矩阵对应的黑白图像，每个矩阵元素对应固定大小的正方形像素，并添加网格线。
        :param matrix: 解码后的矩阵
        :param cell_size: 每个单元格的大小（像素）
        """
        try:
            import cv2
            import numpy as np

            # 将矩阵转换为0-255的灰度图像（0为黑，255为白）
            image = (matrix * 255).astype(np.uint8)

            # 放大矩阵
            rows, cols = matrix.shape
            enlarged_image = np.kron(image, np.ones((cell_size, cell_size), dtype=np.uint8))

            # 添加网格线
            grid_image = cv2.cvtColor(enlarged_image, cv2.COLOR_GRAY2BGR)
            for r in range(0, rows * cell_size, cell_size):
                cv2.line(grid_image, (0, r), (cols * cell_size, r), (128, 128, 128), 1)
            for c in range(0, cols * cell_size, cell_size):
                cv2.line(grid_image, (c, 0), (c, rows * cell_size), (128, 128, 128), 1)

            # 显示图像
            cv2.imshow("Matrix Image with Grid", grid_image)
            cv2.waitKey(0)  # 显示窗口的同时继续运行程序
            cv2.destroyAllWindows()
        except Exception as e:
            print(f"显示矩阵图像时发生错误: {e}")

    def run(self):
        while True:
            data = self.client.recv(4096)  # 接收信息，指定接收的大小
            if not data:
                break

            print("原始报文:")
            print(data.hex())  # 输出接收到的原始字节数据

            # 尝试解码为矩阵形式
            matrix = self.decode_data_to_matrix(data, self.column_num)
            if matrix is not None:
                print("转换后的矩阵:")
                print(matrix)

                # 显示对应的黑白图像
                self.display_matrix_as_image(matrix)
            else:
                print("无法将数据转换为矩阵。")


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='阀门测程序')
    parser.add_argument('-c', default=True, action='store_true', help='是否是开个客户端', required=False)
    parser.add_argument('-m', default='127.0.0.1', help='指定master主机名')
    parser.add_argument('-p', default=13452, help='指定端口')
    parser.add_argument('-cnt', default=64, help='指定端口')
    args = parser.parse_args()
    mask_size = (1024, args.cnt)  # size of cv (Width, Height)
    if args.c:
        print("运行客户机")
        virtual_valve = VirtualValve(host=args.m, port=args.p)
        virtual_valve.run()
