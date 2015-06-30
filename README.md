# chunzhenshmphp
Automatically exported from code.google.com/p/chunzhenshmphp

    之前在项目中用到了纯真ip库的扩展, 那时这个项目托管在google code, 现在google code要关闭了, 留在这里备用

使用纯真IP数据库解析PHP的扩展，使用C++编写，支持Windows + Apache2.2 + PHP5(ZTS)、支持Linux + Nginx + PHP5(FastCGI)。

    本扩展在扩展初始化阶段 ，会将 纯真IP 数据库的字典通过MMAP映射到内存中。因此，当你更新了数据库字典之后，必须重启fpm(Apache + PHP 则需要重启Apache)。 

    项目文件为 Visual Studio 2008 的，Windows下可以直接编译。 对于Linux，则需要指定编译器为 g++。 你可以使用如下方式指定编译器: $PHP_PREFIX 为你的PHP安装目录 

    unzip chunzhen_shm.zip
    cd chunzhen_shm
    $PHP_PREFIX/bin/phpize
    CC=g++ CXX=g++ CFLAGS=-O3 CXXFLAGS=-O3 ./configure --with-php-config=$PHP_PREFIX/bin/php-config
    make
    make install

    就安装好了。 

    PHP的配置，在 php.ini 尾部添加:

    [Chunzhen]
    chunzhen_shm.chunzhen_db="D:\Program Files\Apache Software Foundation\QQWry.Dat"

    扩展的调用:

    print_r(chunzhen_getIPLocation('192.168.8.26'));
