
# 编译相关
# 编译t113应用
./build.sh -t113
# 编译linux应用
./build.sh -linux
# 删除编译信息
./build.sh -clean

# 如果修改了res里面的文件 要把这些推到板子上
adb push build/app/res/* /usr/res/
# 在主机对应的目录 build/app
adb push build/app/demo /usr/bin 




