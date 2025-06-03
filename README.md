# Linux FTP 服务器
**XUPT Linux Group / plan-net : C++ Simple FTP**<br/>
这是一个简单的FTP服务器(基于RFC959 FTP协议)。支持基本的FTP操作，包括文件上传、下载、列出目录等功能。<br/>
该服务器能够在被动模式（PASV）下正常工作。<br/>
暂时不支持主动模式（PORT）以及用户验证。<br/>

~~~sh
git clone https://github.com/DecarbonizedGlucose/FTP-server.git
cd FTP-server
zsh ./build.sh
# 可执行文件目录：bin/
~~~
创建一个配置文件，如config.txt，写入
~~~
ServerRootDirectory={存储路径根目录,绝对路径}
~~~
运行时，
~~~sh
# 客户端
bin/FTP-client
# 服务端
bin/FTP-server {配置文件,绝对路径}
~~~