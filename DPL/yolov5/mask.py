from model import Model

# 初始化 Model 对象，实例化时自动生成并保存掩膜
model = Model(
    image_folder="runs/detect/exp6",
    label_folder="runs/detect/labels",
    output_folder="datasets/mask"
)

# 读取某张图片对应的掩膜
mask_array = model.get_mask_array("image1")  # 替换 "image1" 为你的图片名
print(mask_array)
