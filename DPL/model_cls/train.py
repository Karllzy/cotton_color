import os
import argparse
import time
from datetime import datetime

import torch
import torch.nn as nn
import torch.optim as optim
from torch.optim import lr_scheduler
from torchvision import datasets, models, transforms
import matplotlib.pyplot as plt

# 设置 cudnn 优化
torch.backends.cudnn.benchmark = True


def get_next_model_name(save_dir, base_name, ext):
    """
    检索保存目录，生成递增编号的模型文件名。

    Args:
        save_dir (str): 模型保存目录
        base_name (str): 模型基础名称
        ext (str): 模型文件扩展名（例如 ".pt" 或 ".onnx"）

    Returns:
        str: 自动递增编号的新模型文件名
    """
    os.makedirs(save_dir, exist_ok=True)
    existing_files = [f for f in os.listdir(save_dir) if f.startswith(base_name) and f.endswith(ext)]
    existing_numbers = []
    for f in existing_files:
        try:
            num = int(f[len(base_name):].split('.')[0].strip('_'))
            existing_numbers.append(num)
        except ValueError:
            continue
    next_number = max(existing_numbers, default=0) + 1
    return os.path.join(save_dir, f"{base_name}_{next_number}{ext}")


def get_data_loaders(data_dir, batch_size):
    """Prepare data loaders for training and validation."""
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

    image_datasets = {x: datasets.ImageFolder(os.path.join(data_dir, x), data_transforms[x])
                      for x in ['train', 'val']}
    dataloaders = {x: torch.utils.data.DataLoader(image_datasets[x], batch_size=batch_size,
                                                  shuffle=True, num_workers=4)
                   for x in ['train', 'val']}
    dataset_sizes = {x: len(image_datasets[x]) for x in ['train', 'val']}
    class_names = image_datasets['train'].classes

    return dataloaders, dataset_sizes, class_names


def train_model(model, criterion, optimizer, scheduler, dataloaders, dataset_sizes, device,
                num_epochs, model_save_dir, model_base_name):
    """Train the model and save the best weights."""
    since = time.time()
    os.makedirs(model_save_dir, exist_ok=True)

    best_acc = 0.0
    best_model_path = None  # 用于保存最佳模型路径

    for epoch in range(num_epochs):
        print(f'Epoch {epoch}/{num_epochs - 1}')
        print('-' * 10)

        for phase in ['train', 'val']:
            if phase == 'train':
                model.train()
            else:
                model.eval()

            running_loss = 0.0
            running_corrects = 0

            for inputs, labels in dataloaders[phase]:
                inputs, labels = inputs.to(device), labels.to(device)

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

                # 保存新的最佳模型
                best_model_path = get_next_model_name(model_save_dir, model_base_name, '.pt')

                torch.save(model.state_dict(), best_model_path)
                print(f"Best model weights saved at {best_model_path}")

    time_elapsed = time.time() - since
    print(f'Training complete in {time_elapsed // 60:.0f}m {time_elapsed % 60:.0f}s')
    print(f'Best val Acc: {best_acc:.4f}')

    if best_model_path:
        # 仅加载最后保存的最佳模型
        model.load_state_dict(torch.load(best_model_path, weights_only=True))
    else:
        print("No best model was saved during training.")
    return model


def main(args):
    """Main function to train the model based on command-line arguments."""
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    dataloaders, dataset_sizes, class_names = get_data_loaders(args.data_dir, args.batch_size)
    if args.model_name == 'resnet18':
        model = models.resnet18(weights='IMAGENET1K_V1')
        num_ftrs = model.fc.in_features
        model.fc = nn.Linear(num_ftrs, args.num_classes)
    elif args.model_name == 'resnet50':
        model = models.resnet50(weights='IMAGENET1K_V1')
        num_ftrs = model.fc.in_features
        model.fc = nn.Linear(num_ftrs, args.num_classes)
    elif args.model_name == 'alexnet':
        model = models.alexnet(pretrained=True)
        num_ftrs = model.classifier[-1].in_features
        model.classifier[-1] = nn.Linear(num_ftrs, args.num_classes)
    elif args.model_name == 'vgg16':
        model = models.vgg16(pretrained=True)
        num_ftrs = model.classifier[-1].in_features
        model.classifier[-1] = nn.Linear(num_ftrs, args.num_classes)
    elif args.model_name == 'densenet':
        model = models.densenet121(pretrained=True)
        num_ftrs = model.classifier.in_features
        model.classifier = nn.Linear(num_ftrs, args.num_classes)
    elif args.model_name == 'inceptionv3':
        model = models.inception_v3(pretrained=True)
        num_ftrs = model.fc.in_features
        model.fc = nn.Linear(num_ftrs, args.num_classes)
    model = model.to(device)

    # training start time and model info
    model_base_name = f"{args.model_name}_bs{args.batch_size}_ep{args.epochs}_{datetime.now().strftime('%y_%m_%d')}.pt"
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=args.lr, momentum=args.momentum)
    scheduler = lr_scheduler.StepLR(optimizer, step_size=args.step_size, gamma=args.gamma)

    train_model(model, criterion, optimizer, scheduler, dataloaders, dataset_sizes, device,
                args.epochs, args.model_save_dir, model_base_name)

if __name__ == '__main__':
    # 参数解析部分
    parser = argparse.ArgumentParser(description="Train a ResNet18 model on custom dataset.")
    parser.add_argument('--model_name', type=str, default='resnet18', help='Which model to train as base model')
    parser.add_argument('--data-dir', type=str, default='dataset', help='Path to the dataset directory.')
    parser.add_argument('--model-save-dir', type=str, default='model', help='Directory to save the trained model.')
    parser.add_argument('--batch-size', type=int, default=8, help='Batch size for training.')
    parser.add_argument('--epochs', type=int, default=5, help='Number of training epochs.')
    parser.add_argument('--lr', type=float, default=0.0005, help='Learning rate.')
    parser.add_argument('--momentum', type=float, default=0.95, help='Momentum for SGD.')
    parser.add_argument('--step-size', type=int, default=5, help='Step size for LR scheduler.')
    parser.add_argument('--gamma', type=float, default=0.2, help='Gamma for LR scheduler.')
    parser.add_argument('--num-classes', type=int, default=2, help='Number of output classes.')
    args = parser.parse_args()
    main(args)
