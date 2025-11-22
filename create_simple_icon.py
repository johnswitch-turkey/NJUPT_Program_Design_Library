#!/usr/bin/env python3
"""
简单的图标创建脚本 - 创建基础的ICO文件
需要安装: pip install pillow
"""

from PIL import Image, ImageDraw
import os

def create_simple_ico(ico_file):
    """创建一个简单的书籍图标"""
    try:
        # 创建不同尺寸的图像
        sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
        images = []

        for size in sizes:
            # 创建图像
            img = Image.new('RGBA', size, (0, 0, 0, 0))
            draw = ImageDraw.Draw(img)

            w, h = size

            # 绘制简化的书籍图标
            # 背景
            margin = max(2, w // 32)
            draw.rectangle([margin, margin, w-margin, h-margin],
                         fill=(74, 144, 226, 255), outline=(0, 0, 0, 255), width=1)

            # 书籍线条
            book_margin = max(1, w // 64)
            line_spacing = max(1, w // 16)
            for i in range(3):
                y = margin + book_margin + i * line_spacing
                if y < h - margin - book_margin:
                    draw.rectangle([margin*2, y, w-margin*2, y+max(1, w//32)],
                                 fill=(255, 255, 255, 180))

            images.append(img)

        # 保存ICO文件
        img.save(ico_file, format='ICO', sizes=[(16,16), (32,32), (48,48), (64,64), (128,128), (256,256)])
        print(f"✅ 成功创建ICO文件: {ico_file}")
        return True

    except Exception as e:
        print(f"❌ 创建ICO文件失败: {e}")
        return False

if __name__ == "__main__":
    ico_file = "src/resource/library.ico"

    print(f"🔄 正在创建ICO文件: {ico_file}")
    create_simple_ico(ico_file)