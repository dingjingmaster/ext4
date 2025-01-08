<p align="center">
    <h2>Ext</h2>
</p>

使用e2fs，基于fuse实现ext文件系统

## 依赖

- fuse
- e2fs

## 编译

```shell
cmake -B build .
make -C build
```

## TODO

- [x] 可挂载由mkfs.extX格式化的文件系统
- [ ] 整合日志
- [ ] 整合e2fs
- [ ] 自己实现设备读写操作，方便后续做加解密处理
