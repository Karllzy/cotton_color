import socket
import time

import cv2
import numpy as np

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