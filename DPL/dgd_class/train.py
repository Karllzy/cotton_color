import torch
import torch.nn as nn
import torch.optim as optim
from torch.optim import lr_scheduler
import torch.backends.cudnn as cudnn
import numpy as np
import torchvision
from torchvision import datasets, models, transforms
import matplotlib.pyplot as plt
import time
import os
from PIL import Image

# 初始化 cudnn 优化
cudnn.benchmark = True
plt.ion()  # interactive mode

data_transforms = {
    'train': transforms.Compose([
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ]),
    'val': transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ]),
}

data_dir = 'd_2'
image_datasets = {x: datasets.ImageFolder(os.path.join(data_dir, x),
                                          data_transforms[x])
                  for x in ['train', 'val']}
dataloaders = {x: torch.utils.data.DataLoader(image_datasets[x], batch_size=4,
                                              shuffle=True, num_workers=4)
               for x in ['train', 'val']}
dataset_sizes = {x: len(image_datasets[x]) for x in ['train', 'val']}
class_names = image_datasets['train'].classes

device = torch.device("cuda:0")

#
def imshow(inp, title=None):
    """Display image for Tensor."""
    if isinstance(inp, np.ndarray):
        inp = inp.transpose((1, 2, 0))
    # inp = inp.numpy().transpose((1, 2, 0))
    mean = np.array([0.485, 0.456, 0.406])
    std = np.array([0.229, 0.224, 0.225])
    inp = std * inp + mean
    inp = np.clip(inp, 0, 1)
    plt.imshow(inp)
    if title is not None:
        plt.title(title)
    plt.pause(0.001)  # pause a bit so that plots are updated


def train_model(model, criterion, optimizer, scheduler, num_epochs=25, model_save_dir='model', save_onnx=False):
    since = time.time()

    # 创建保存模型的目录（如果不存在）
    os.makedirs(model_save_dir, exist_ok=True)

    # 设置模型保存路径
    best_model_params_path = os.path.join(model_save_dir, 'best_model_params_11.14.19.30.pt')
    best_model_onnx_path = os.path.join(model_save_dir, 'best_model_11.14.19.30.onnx')

    # 初始保存模型
    torch.save(model.state_dict(), best_model_params_path)
    best_acc = 0.0

    for epoch in range(num_epochs):
        print(f'Epoch {epoch}/{num_epochs - 1}')
        print('-' * 10)

        for phase in ['train', 'val']:
            if phase == 'train':
                model.train()  # Set model to training mode
            else:
                model.eval()  # Set model to evaluate mode

            running_loss = 0.0
            running_corrects = 0

            for inputs, labels in dataloaders[phase]:
                inputs = inputs.to(device)
                labels = labels.to(device)

                optimizer.zero_grad()

                with torch.set_grad_enabled(phase == 'train'):
                    outputs = model(inputs)
                    _, preds = torch.max(outputs, 1)
                    loss = criterion(outputs, labels)

                    if phase == 'train':
                        loss.backward()
                        optimizer.step()

                running_loss += loss.item() * inputs.size(0)
                running_corrects += torch.sum(preds == labels.data)

            if phase == 'train':
                scheduler.step()

            epoch_loss = running_loss / dataset_sizes[phase]
            epoch_acc = running_corrects.double() / dataset_sizes[phase]

            print(f'{phase} Loss: {epoch_loss:.4f} Acc: {epoch_acc:.4f}')

            if phase == 'val' and epoch_acc > best_acc:
                best_acc = epoch_acc
                # 保存最佳模型
                torch.save(model.state_dict(), best_model_params_path)

                # 如果需要保存ONNX格式的模型，可以在这里执行
                if save_onnx:
                    # 导出模型为 ONNX 格式
                    model.eval()
                    dummy_input = torch.randn(1, 3, 224, 224).to(device)  # 用于推理的假输入（与训练时输入的大小一致）
                    torch.onnx.export(model, dummy_input, best_model_onnx_path,
                                      export_params=True, opset_version=11,
                                      do_constant_folding=True, input_names=['input'], output_names=['output'])
                    print(f"ONNX model saved at {best_model_onnx_path}")

        print()

    time_elapsed = time.time() - since
    print(f'Training complete in {time_elapsed // 60:.0f}m {time_elapsed % 60:.0f}s')
    print(f'Best val Acc: {best_acc:4f}')

    model.load_state_dict(torch.load(best_model_params_path, weights_only=True))
    return model


# def visualize_model(model, num_images=6):
#     was_training = model.training
#     model.eval()
#     images_so_far = 0
#     fig = plt.figure()
#
#     with torch.no_grad():
#         for i, (inputs, labels) in enumerate(dataloaders['train']):
#             inputs = inputs.to(device)
#             labels = labels.to(device)
#
#             outputs = model(inputs)
#             _, preds = torch.max(outputs, 1)
#
#             for j in range(inputs.size()[0]):
#                 images_so_far += 1
#                 ax = plt.subplot(num_images // 2, 2, images_so_far)
#                 ax.axis('off')
#                 ax.set_title(f'predicted: {class_names[preds[j]]}')
#                 plt.imshow(inputs.cpu().data[j])
#
#                 if images_so_far == num_images:
#                     model.train(mode=was_training)
#                     return
#         model.train(mode=was_training)


if __name__ == '__main__':
    # TODO:
    #  1. 能像yolo一样使用不同参数运行
    #  2. 能够比较不同模型的预测时间，根据配置可以切换模型。

    model_ft = models.resnet18(weights='IMAGENET1K_V1')
    num_ftrs = model_ft.fc.in_features
    model_ft.fc = nn.Linear(num_ftrs, 2)

    model_ft = model_ft.to(device)

    criterion = nn.CrossEntropyLoss()

    optimizer_ft = optim.SGD(model_ft.parameters(), lr=0.001, momentum=0.9)

    exp_lr_scheduler = lr_scheduler.StepLR(optimizer_ft, step_size=7, gamma=0.1)

    # 指定保存模型的目录
    model_save_dir = 'model'  # 你可以修改为你希望保存的路径

    # 训练模型并保存 ONNX 格式的模型
    model_ft = train_model(model_ft, criterion, optimizer_ft, exp_lr_scheduler, num_epochs=25,
                           model_save_dir=model_save_dir, save_onnx=True)
    # visualize_model(model_ft)
    # TODO: 3. 新增dgd_class/export.py, 可以把训练得到的pt文件转换为onnx
