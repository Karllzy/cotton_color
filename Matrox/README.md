# MIL库环境配置

include_directories(E:/QTexamble/matrox/Include)  
将路径修改为你的安装目录   .../Matrox Imaging/MIL/Include
# 添加 MIL 库的库文件路径
link_directories(E:/QTexamble/matrox/LIB)
file(GLOB MIL_LIBS E:/QTexamble/matrox/LIB/*.lib)
同理 将E:/QTexamble/matrox/LIB部分替换为安装目录下的.../Matrox Imaging/MIL/LIB   即可