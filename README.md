# scan_line-z_buffer
使用方法：
可以在main函数中img.write_tga_file()中更改读取文件的路径，obj文件中有3个obj类型文件可供使用，面片从两千多到五千多不等。需要注意的是本代码仅针对三角形面片进行计算，因此只能使用只含三角形面片的obj文件。
运行后输出图片为output.tga。
代码没有使用openGL等库，只使用STL来完成，读取obj文件等基本操作参考了https://github.com/ssloy/tinyrenderer/wiki
Zbuffer算法的数据结构参考了https://blog.csdn.net/weixin_43813453/article/details/86215955
zbuffer边表多边形表以及扫描顺序是与PPT中的一致，从上向下扫描。
此外，在相对复杂的zbuffer算法中尝试使用了部分智能指针，避免操作不当引起内存泄漏
结果图如下图所示

![output](https://user-images.githubusercontent.com/50654768/172186773-a4ef13a0-4bfb-4bcc-ae1c-342d7c5071bd.jpg)
![output](https://user-images.githubusercontent.com/50654768/172187214-4cdd98ca-e304-466a-bf36-7979b509c6f9.jpg)
![output](https://user-images.githubusercontent.com/50654768/172187604-ca0b3107-134a-4045-a38f-9d6236c2fa73.jpg)

经验心得：
PPT中算法描述要求首先建立分类的多边形表和边表。在编写过程中我认为先建立多边形表即可，边表可以随着扫描线的进行，遍历到一个多边形就将其三个边的信息加入到边表中。算法中使用到边都是与对应的多边形挂钩的，所以最好设计一个数据结构把边和对应的多边形编号相对应。
改进思路：
对STL掌握仍然不熟练，该算法使用map，set等关联容器操作会更加方便，之后改进应该主动地去使用合适的数据结构，增进自己的熟练度和理解。



