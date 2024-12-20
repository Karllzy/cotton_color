# Cotton Color

[TOC]

## 算法整体框架

### 颜色检测

#### 艳丽色彩检测

纯色彩检测就可以。饱和度检测。

思路：rgb -> HSV -> s -> threshold -> 杂质 

#### 黑色检测/滴灌带检测

L a* b* 色彩空间检测，纯黑色就是杂质，但是容易有噪声。

思路：rgb -> La*b* -> threshold -> 黑色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

#### 暗黄色检测/油棉

L a* b* 色彩空间检测，检测暗黄色。

思路：rgb -> La*b* -> threshold -> 暗黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

#### 带土地膜检测

L a* b* 色彩空间检测，检测明黄色、白色。

思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质


### 深度学习检测

需求：模板匹配缺少对于纹理的判断，所以要加上深度学习对于各个杂质进行确认。

#### 方案1：端到端式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

深度学习思路：rgb -> 可疑色彩图像增强 -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
						              |
						               -> YOLO -> 杂质


#### 方案2：验证形式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

融合深度学习思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
															|									  |
									激进方案    			 -> 深度学习 - > 区块判别 -> 杂质     |
																					      |
									保守方案											   -> 深度学习确认 -> 杂质

##### 讨论记录：

暗红色（棉叶）

棉花亮度低

黑线、黑色孔洞

土黄

## 安装记录

### OpenCV安装

下载OpenCV并设置OpenCV_DIR到环境变量，例如：

**在 Windows 上**

1. 打开“控制面板” > “系统和安全” > “系统”。
2. 点击左侧的“高级系统设置”。
3. 在“系统属性”窗口中，点击“环境变量”。
4. 在“系统变量”或“用户变量”中，点击“新建”。
5. 输入变量名 OpenCV_DIR，在变量值中输入 OpenCV 安装的路径，例如 C:\opencv\build。
6. 点击“确定”保存更改。

**在 macOS 或 Linux 上**

1. 打开终端。
2. 编辑您的 shell 配置文件（例如 `~/.bashrc`, `~/.zshrc` 或 `~/.bash_profile`）：

```bash
export OpenCV_DIR=/path/to/opencv/build
```

将 `/path/to/opencv/build` 替换为 OpenCV 安装路径。

保存文件后，运行以下命令使更改生效：

```bash
source ~/.bashrc  # 或者使用 `source ~/.zshrc` 根据您的 shell 类型
```

完成这些步骤后，CMake 应该可以在运行时读取 OpenCV_DIR 变量，确保您的路径正确指向 OpenCV 安装目录的 build 文件夹。

### Qt安装

**让机器能够访问dll：**

1. 将 DLL 添加到环境变量 PATH
2. 为了让应用程序运行时找到 Qt6Widgets.dll，需要将 Qt 的 bin 目录添加到 PATH 环境变量中：

**让编译器能够找到编译路径：**

1. 打开“开始”菜单，搜索“环境变量”并打开“编辑系统环境变量”。
2. 在“系统变量”中找到 PATH，选择并点击“编辑”。
3. 点击“新建”，将 Qt 的 bin 目录路径（例如 `E:\QT\6.8.0\msvc2019_64\bin`）添加进去。
4. 点击“确定”并保存更改。