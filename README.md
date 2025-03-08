# Raspberry-Bare-Metal-Operating-System
一个树莓派裸机操作系统
# 部署
## 编译
编译器： aarch64-none-elf-gcc，自行下载安装加入到环境变量，然后适当修改我的Makefile后即可make编译  
如果我的Makefile并未全部修改，编译后生成的文件会保存在一个build文件夹中  
其中.o结尾的文件属于编译过程中的过程文件，将其中的kernel.img和根目录下的config.txt以及start中的4份文件存放到树莓派的SD卡中，SD卡需要格式化成FAT32格式。  
启动树莓派4B 
## 现象
你需要一个USB转TTL方便树莓派将输出的信息输出到电脑上的串口接收程序中。  
你可以在这个链接https://pinout.xyz/ 参考树莓派的引脚。  
![image](https://github.com/user-attachments/assets/2bfdcdc9-9014-4719-adf3-451907490e5f)  
将USB转TTL连接到树莓派的IO14、IO15。在电脑端打开串口通信程序，随便，选择好波特率115200，即可在树莓派完成开机的最后，观察到 “Welcome to your bare-metal Raspberry Pi 4!”。
恭喜你，部署成功。  
如果你恰好将树莓派连接到一块屏幕上，你会发现屏幕变成了彩色，这是树莓派的start文件在初始化GPU后的正常现象。
