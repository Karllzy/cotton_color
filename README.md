﻿# Cotton Color

## 颜色检测

### 艳丽色彩检测

纯色彩检测就可以。饱和度检测。

思路：rgb -> HSV -> s -> threshold -> 杂质 

### 黑色检测/滴灌带检测

L a* b* 色彩空间检测，纯黑色就是杂质，但是容易有噪声。

思路：rgb -> La*b* -> threshold -> 黑色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

### 暗黄色检测/油棉

L a* b* 色彩空间检测，检测暗黄色。

思路：rgb -> La*b* -> threshold -> 暗黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

### 带土地膜检测

L a* b* 色彩空间检测，检测明黄色、白色。

思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质


## 深度学习检测

需求：模板匹配缺少对于纹理的判断，所以要加上深度学习对于各个杂质进行确认。

### 方案1：端到端式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

深度学习思路：rgb -> 可疑色彩图像增强 -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
						              |
						               -> YOLO -> 杂质


### 方案2：验证形式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

融合深度学习思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
															|									  |
									激进方案    			 -> 深度学习 - > 区块判别 -> 杂质     |
																					      |
									保守方案											   -> 深度学习确认 -> 杂质

讨论记录：

暗红色（棉叶）

棉花亮度低

黑线、黑色孔洞

土黄

