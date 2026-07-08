# 第43章 Linux设备树学习笔记（重要度：★★★★★）

> 学习日期：2026-07-08  
> 资料来源：正点原子《I.MX6U嵌入式Linux驱动开发指南》第43章 + 本章学习提问整理  
> 当前阶段：从“字符设备框架”进入“设备树/BSP/板级驱动”的关键转折章

---

## 43.1 什么是设备树？（重要度：★★★★★）

### 核心理解

设备树（Device Tree）本质上是**硬件说明书**，用树形结构描述开发板上的硬件资源。

它描述的是：

| 内容 | 例子 |
|---|---|
| CPU 信息 | Cortex-A7、CPU 编号 |
| 内存信息 | 起始地址、大小 |
| SoC 内部外设 | GPIO、UART、I2C、SPI、USB |
| 板级外设 | I2C 上接了什么芯片、SPI 上接了什么芯片 |
| 引脚/中断/时钟资源 | pinctrl、interrupts、clocks |

一句话：

```text
设备树负责说明“板子上有什么硬件、接在哪里、需要哪些资源”。
驱动负责说明“拿到这些资源以后怎么操作硬件”。
```

### 为什么需要设备树？

早期 ARM Linux 没有设备树时，板级信息大量写在 `arch/arm/mach-xxx`、`arch/arm/plat-xxx` 下面的 C 文件里。

问题是：

```text
同一个 SoC 可以做很多块板子
每块板子的外设又不一样
如果都写进内核 C 文件，内核会堆积大量板级硬编码
```

设备树的作用就是把这些板级硬件描述从内核 C 代码中分离出去。

### 和前面章节的关系

第41/42章 LED 驱动里，我们在 C 文件中直接写寄存器物理地址：

```c
#define CCM_CCGR1_BASE        0X020C406C
#define SW_MUX_GPIO1_IO03     0X020E0068
#define SW_PAD_GPIO1_IO03     0X020E02F4
#define GPIO1_DR_BASE         0X0209C000
#define GPIO1_GDIR_BASE       0X0209C004
```

到了设备树驱动，这些硬件信息应该逐步放到 `.dts` 中，驱动再通过 OF 函数读取。

也就是：

```text
以前：驱动 C 文件里写死硬件资源
现在：DTS 描述硬件资源，驱动从设备树读取资源
```

---

## 43.2 DTS、DTB 和 DTC（重要度：★★★★★）

### 三者关系

| 名称 | 全称 | 作用 | 类比 |
|---|---|---|---|
| DTS | Device Tree Source | 设备树源码，人能看懂 | `.c` 文件 |
| DTB | Device Tree Blob | 编译后的二进制设备树，内核使用 | `.o`/二进制文件 |
| DTC | Device Tree Compiler | 把 DTS 编译成 DTB 的工具 | `gcc` |

整体流程：

```text
imx6ull-alientek-emmc.dts
        +
imx6ull.dtsi
        ↓ DTC 编译
imx6ull-alientek-emmc.dtb
        ↓ U-Boot bootz 传给 Linux
Linux 内核解析 DTB
        ↓
/proc/device-tree 中体现出来
        ↓
驱动通过 OF 函数读取节点和属性
```

### 编译设备树

在 Linux 内核源码根目录下：

```bash
make dtbs
```

或者编译全部：

```bash
make all
```

只想编译设备树时，优先用：

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- dtbs
```

### 哪些 DTS 会被编译？

由：

```text
arch/arm/boot/dts/Makefile
```

中的 `dtb-$(CONFIG_SOC_IMX6ULL)` 决定。

正点原子 ALPHA EMMC 板对应：

```text
imx6ull-alientek-emmc.dtb
```

### 易错点

`DTS` 不是内核直接使用的最终文件，内核启动时拿到的是 `DTB`。驱动也不是直接打开 `.dts` 文本文件，而是内核启动后已经解析好设备树，驱动通过内核提供的 OF API 查询节点和属性。

---

## 43.3 DTS 语法（重要度：★★★★★）

### 43.3.1 .dtsi 头文件（重要度：★★★★☆）

`.dtsi` 类似 C 语言中的头文件。

通常分工：

| 文件 | 描述内容 |
|---|---|
| `.dtsi` | SoC 级公共信息，如 CPU、UART、I2C、GPIO 控制器 |
| `.dts` | 具体开发板信息，如板子上外接了什么芯片、哪些外设启用 |

例如：

```dts
#include <dt-bindings/input/input.h>
#include "imx6ull.dtsi"
```

可以引用：

```text
.h
.dtsi
.dts
```

但自己写设备树公共头文件时，建议使用 `.dtsi` 后缀。

### 43.3.2 设备节点（重要度：★★★★★）

设备树由节点和属性组成。

最小结构示例：

```dts
/ {
    aliases {
        can0 = &flexcan1;
    };

    cpus {
        #address-cells = <1>;
        #size-cells = <0>;

        cpu0: cpu@0 {
            compatible = "arm,cortex-a7";
            device_type = "cpu";
            reg = <0>;
        };
    };
};
```

#### 根节点 `/`

每个设备树只有一个根节点：

```dts
/ {
};
```

如果 `.dts` 和 `.dtsi` 中都写了 `/`，不会冲突，DTC 编译时会合并成一个根节点。

#### 节点命名

标准格式：

```text
node-name@unit-address
```

例子：

```dts
serial@02020000
```

含义：

```text
serial      节点名称
02020000    通常是寄存器首地址或设备地址
```

#### label

完整格式：

```text
label: node-name@unit-address
```

例子：

```dts
uart1: serial@02020000 {
};
```

其中：

```text
uart1               label
serial@02020000     真正的节点名
```

label 的作用是方便其他地方引用：

```dts
&uart1 {
    status = "okay";
};
```

### 节点和属性

节点像“硬件对象”，属性像“硬件参数”。

```dts
uart1: serial@02020000 {
    compatible = "fsl,imx6ul-uart";
    reg = <0x02020000 0x4000>;
    status = "disabled";
};
```

这里：

| 内容 | 含义 |
|---|---|
| `uart1` | label |
| `serial@02020000` | 节点名 |
| `compatible` | 这个设备能匹配什么驱动 |
| `reg` | 寄存器地址范围 |
| `status` | 是否启用 |

### 个人理解：是不是每个设备就是一个节点？

基本可以这么理解：

```text
设备树中的节点通常表示一个硬件对象或功能模块。
```

但要注意：

```text
不是每个节点都一定代表一颗外部芯片。
```

节点也可以表示：

| 节点类型 | 例子 |
|---|---|
| SoC 内部模块 | GPIO、UART、I2C、SPI、USB |
| 外接芯片 | AHT20、WM8960、MPU6050 |
| 总线/地址区域 | soc、aips-bus |
| 启动参数节点 | chosen |
| 别名节点 | aliases |

所以更准确的说法是：

```text
节点 = 一个硬件对象、功能模块，或设备树需要表达的系统结构。
```

---

## 43.3.3 标准属性（重要度：★★★★★）

### 1. compatible 属性（重要度：★★★★★）

`compatible` 是设备树中最重要的属性之一，用来完成**设备和驱动的匹配**。

格式：

```dts
compatible = "manufacturer,model";
```

例子：

```dts
compatible = "fsl,imx6ul-uart", "fsl,imx6q-uart", "fsl,imx21-uart";
```

含义：

```text
优先匹配 "fsl,imx6ul-uart"
如果没有，再看后面的兼容项
通常越靠前越具体，越靠后越通用
```

驱动中会有匹配表：

```c
static const struct of_device_id xxx_of_match[] = {
    { .compatible = "fsl,imx6ul-uart" },
    { /* sentinel */ }
};
```

只要设备树节点里的 `compatible` 和驱动匹配表里的 `.compatible` 字符串相同，就说明这个设备可以交给该驱动处理。

### 个人疑问：compatible 是不是驱动文件名？

不是。

`compatible` 是设备树节点提供的“身份标签”，驱动也声明自己支持哪些身份标签。它不要求等于驱动源文件名。

例如：

```dts
compatible = "fsl,imx-audio-wm8960";
```

驱动文件可能叫：

```text
imx-wm8960.c
```

它们不是靠文件名匹配，而是靠字符串匹配。

### 2. model 属性（重要度：★★★☆☆）

用于描述设备模块信息。

```dts
model = "wm8960-audio";
```

根节点中也常见：

```dts
model = "Freescale i.MX6 ULL 14x14 EVK Board";
```

### 3. status 属性（重要度：★★★★★）

表示设备状态。

| 值 | 含义 |
|---|---|
| `"okay"` | 设备启用 |
| `"disabled"` | 设备禁用 |
| `"fail"` | 设备不可用 |
| `"fail-xxx"` | 设备失败，xxx 描述原因 |

常见场景：

```dts
&i2c1 {
    status = "okay";
};
```

如果某个控制器在 `.dtsi` 中默认是：

```dts
status = "disabled";
```

具体板级 `.dts` 中要使用它，就改成：

```dts
status = "okay";
```

### 4. #address-cells 和 #size-cells（重要度：★★★★★）

这是本章最容易卡住的点。

一句话：

```text
#address-cells 决定子节点 reg 属性里“地址”占几个 32 位 cell。
#size-cells 决定子节点 reg 属性里“长度”占几个 32 位 cell。
```

注意：

```text
不是说地址值等于 1。
不是说设备大小等于 1。
而是规定 reg 里的地址和长度各用几个 32 位数字表示。
```

#### 例1：地址和长度各占 1 个 cell

```dts
aips3: aips-bus@02200000 {
    #address-cells = <1>;
    #size-cells = <1>;

    dcp: dcp@02280000 {
        reg = <0x02280000 0x4000>;
    };
};
```

解释：

```text
address = 0x02280000
length  = 0x4000
```

也就是：

```text
从 0x02280000 开始，占用 0x4000 字节寄存器空间。
```

#### 例2：I2C 子设备没有长度字段

```dts
i2c1: i2c@021a0000 {
    #address-cells = <1>;
    #size-cells = <0>;

    aht20@38 {
        compatible = "aosong,aht20";
        reg = <0x38>;
    };
};
```

解释：

```text
#address-cells = <1>  表示 I2C 子设备地址占 1 个 cell
#size-cells = <0>     表示没有长度字段
reg = <0x38>          表示 AHT20 的 I2C 从机地址是 0x38
```

这里 `reg` 不是寄存器物理地址，而是 I2C 设备地址。

### 个人疑问：reg 到底是什么意思？

`reg` 的含义取决于父节点是什么。

| 父节点类型 | 子节点 reg 含义 |
|---|---|
| SoC 总线/aips | 寄存器物理地址 + 寄存器区域长度 |
| I2C 控制器 | I2C 从机地址 |
| SPI 控制器 | SPI 片选号/片选地址 |
| CPU 节点 | CPU 编号 |

所以不能机械理解为：

```text
reg 永远等于寄存器地址
```

更准确：

```text
reg 表示该节点在父节点地址空间中的地址信息。
```

### 5. ranges 属性（重要度：★★★☆☆）

`ranges` 表示子地址空间到父地址空间的映射关系。

空 `ranges`：

```dts
ranges;
```

表示：

```text
子地址空间和父地址空间一致，不需要地址转换。
```

在 I.MX6ULL 中比较常见。

### 6. name 和 device_type（重要度：★★☆☆☆）

`name` 已经不推荐使用。

`device_type` 主要用于 `cpu`、`memory` 等少数节点。

例如：

```dts
cpu0: cpu@0 {
    compatible = "arm,cortex-a7";
    device_type = "cpu";
    reg = <0>;
};
```

普通外设驱动学习中，不需要重点背。

---

## 43.3.4 根节点 compatible 属性（重要度：★★★★☆）

普通设备节点的 `compatible` 用来匹配设备驱动。

根节点 `/` 的 `compatible` 用来告诉内核：

```text
这是一块什么开发板，使用什么 SoC。
```

例子：

```dts
/ {
    model = "Freescale i.MX6 ULL 14x14 EVK Board";
    compatible = "fsl,imx6ull-14x14-evk", "fsl,imx6ull";
};
```

含义：

| 字符串 | 含义 |
|---|---|
| `"fsl,imx6ull-14x14-evk"` | 具体开发板 |
| `"fsl,imx6ull"` | 通用 SoC |

内核里会有类似：

```c
static const char *imx6ul_dt_compat[] __initconst = {
    "fsl,imx6ul",
    "fsl,imx6ull",
    NULL,
};

DT_MACHINE_START(IMX6UL, "Freescale i.MX6 Ultralite (Device Tree)")
    .dt_compat = imx6ul_dt_compat,
MACHINE_END
```

只要根节点的 `compatible` 能匹配 `.dt_compat` 表中的某一项，内核就知道支持这类机器。

### 启动匹配流程

```text
start_kernel()
  ↓
setup_arch()
  ↓
setup_machine_fdt()
  ↓
of_flat_dt_match_machine()
  ↓
比较根节点 compatible 和内核 machine_desc.dt_compat
  ↓
找到匹配的 machine_desc
```

### 个人理解

这里不是普通设备驱动的 `probe` 匹配，而是内核启动早期的“整机匹配”。

可以理解为：

```text
根节点 compatible：告诉内核这是一台什么机器。
普通设备节点 compatible：告诉内核这个外设应该找哪个驱动。
```

---

## 43.3.5 向节点追加或修改内容（重要度：★★★★★）

不要直接修改公共 `.dtsi` 中的 SoC 节点来添加板级外设。

原因：

```text
.dtsi 是 SoC 公共描述文件
所有使用这个 SoC 的板子都会包含它
如果把某块板子特有的外设直接写进 .dtsi，会影响其他板子
```

正确方式是在具体板级 `.dts` 中用 `&label` 追加或修改：

```dts
&i2c1 {
    clock-frequency = <100000>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_i2c1>;
    status = "okay";

    fxls8471@1e {
        compatible = "fsl,fxls8471";
        reg = <0x1e>;
        position = <0>;
    };
};
```

核心：

```text
&i2c1 不是重新创建一个节点
而是打开 imx6ull.dtsi 中已有 label 为 i2c1 的节点，继续追加/修改内容
```

### 和以后 I2C 传感器项目的关系

以后写 AHT20/SHT30 这类 I2C 传感器，大概率就是：

```dts
&i2c1 {
    status = "okay";

    aht20@38 {
        compatible = "aosong,aht20";
        reg = <0x38>;
    };
};
```

然后驱动里写：

```c
static const struct of_device_id aht20_of_match[] = {
    { .compatible = "aosong,aht20" },
    { }
};
```

设备树和驱动通过 `compatible` 绑定。

---

## 43.4 创建小型模板设备树（重要度：★★★☆☆）

这一节主要是帮助理解设备树层级结构，不需要死背完整模板。

模板结构：

```text
/
├── cpus
│   └── cpu@0
└── soc
    ├── sram@00900000
    ├── aips-bus@02000000
    │   └── ecspi@02008000
    ├── aips-bus@02100000
    │   └── usb@02184000
    └── aips-bus@02200000
        └── rngb@02284000
```

重点不是自己从零写完整设备树，而是看懂：

```text
父节点通过 #address-cells / #size-cells 规定子节点 reg 的格式。
每个外设节点通过 compatible / reg / status 等属性描述自己。
```

---

## 43.5 设备树在系统中的体现（重要度：★★★★☆）

Linux 启动后会解析 DTB，并在根文件系统中体现为：

```text
/proc/device-tree
```

在板子上可以查看：

```bash
ls /proc/device-tree
cat /proc/device-tree/model
cat /proc/device-tree/compatible
```

进入 `soc` 节点：

```bash
ls /proc/device-tree/soc
```

理解方式：

```text
DTS 中的节点，在 /proc/device-tree 下体现为目录。
DTS 中的属性，在 /proc/device-tree 下体现为文件。
```

### 易错点

`/proc/device-tree` 不是原始 `.dts` 文件，而是内核解析 DTB 后导出的运行时视图。

---

## 43.6 特殊节点（重要度：★★★★☆）

### 43.6.1 aliases 子节点（重要度：★★★☆☆）

`aliases` 用来定义别名。

```dts
aliases {
    can0 = &flexcan1;
    ethernet0 = &fec1;
    gpio0 = &gpio1;
    i2c0 = &i2c1;
    spi0 = &ecspi1;
};
```

作用：

```text
方便内核或驱动用稳定名字访问某些节点。
```

但实际修改设备树时，更常见的是直接用 `&label`。

### 43.6.2 chosen 子节点（重要度：★★★★☆）

`chosen` 不是一个真实硬件设备，它主要用于 U-Boot 向 Linux 内核传递启动参数。

设备树中可能只写：

```dts
chosen {
    stdout-path = &uart1;
};
```

但系统启动后：

```bash
cat /proc/device-tree/chosen/bootargs
```

会看到类似：

```text
console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw
```

原因：

```text
U-Boot 启动内核前，会把 bootargs 环境变量写入 DTB 的 /chosen/bootargs 属性。
```

调用链大致是：

```text
bootz
  ↓
do_bootz()
  ↓
do_bootm_linux()
  ↓
image_setup_linux()
  ↓
image_setup_libfdt()
  ↓
fdt_chosen()
  ↓
向 chosen 节点添加 bootargs
```

---

## 43.7 Linux 内核解析 DTB 文件（重要度：★★★☆☆）

内核启动后会解析 DTB。

主要流程：

```text
start_kernel()
  ↓
setup_arch()
  ↓
unflatten_device_tree()
  ↓
__unflatten_device_tree()
  ↓
unflatten_dt_node()
```

理解即可，不需要现在深入源码。

重点是知道：

```text
内核不是直接一直读 .dts 文本。
内核启动时拿到 DTB，解析后在内存中形成设备树结构。
驱动通过 OF 函数读取这些结构。
```

---

## 43.8 绑定信息文档（重要度：★★★★★）

绑定文档路径：

```text
Documentation/devicetree/bindings
```

它告诉你某类设备树节点应该怎么写：

| 要写的设备 | 查哪里 |
|---|---|
| I2C 控制器 | `Documentation/devicetree/bindings/i2c/` |
| SPI 控制器/设备 | `Documentation/devicetree/bindings/spi/` |
| LED | `Documentation/devicetree/bindings/leds/` |
| GPIO key | `Documentation/devicetree/bindings/input/` |
| 网络/CAN | `Documentation/devicetree/bindings/net/` |

绑定文档通常会写：

```text
Required properties
Optional properties
Example
```

### 个人理解

设备树不是随便把属性放进去。不同设备的属性名、格式、含义，应该遵守绑定文档。

例如 I2C 控制器常见必需属性：

```text
compatible
reg
interrupts
clocks
```

可选属性：

```text
clock-frequency
```

---

## 43.9 设备树常用 OF 操作函数（重要度：★★★★★）

OF 函数是驱动读取设备树的接口，常见头文件：

```c
#include <linux/of.h>
#include <linux/of_address.h>
```

### 总体思路

驱动使用设备树的一般流程：

```text
1. 找到设备节点
2. 读取节点属性
3. 获取寄存器/GPIO/中断/时钟等资源
4. 初始化硬件
```

第44章设备树 LED 会用到：

```c
of_find_node_by_path()
of_property_read_string()
of_property_read_u32_array()
of_iomap()
```

### 43.9.1 查找节点的 OF 函数（重要度：★★★★★）

内核用 `struct device_node` 描述一个设备树节点。

常用函数：

| 函数 | 作用 |
|---|---|
| `of_find_node_by_name` | 按节点名查找 |
| `of_find_node_by_type` | 按 `device_type` 查找 |
| `of_find_compatible_node` | 按 `compatible` 查找 |
| `of_find_matching_node_and_match` | 按 `of_device_id` 匹配表查找 |
| `of_find_node_by_path` | 按完整路径查找 |

第44章最直接会用：

```c
dtsled.nd = of_find_node_by_path("/alphaled");
```

### 43.9.2 查找父/子节点的 OF 函数（重要度：★★★☆☆）

| 函数 | 作用 |
|---|---|
| `of_get_parent` | 获取父节点 |
| `of_get_next_child` | 迭代获取子节点 |

暂时理解即可。

### 43.9.3 提取属性值的 OF 函数（重要度：★★★★★）

常用函数：

| 函数 | 作用 |
|---|---|
| `of_find_property` | 查找属性 |
| `of_property_count_elems_of_size` | 统计属性元素数量 |
| `of_property_read_u32_index` | 读取指定下标的 u32 |
| `of_property_read_u32_array` | 读取 u32 数组 |
| `of_property_read_u32` | 读取单个 u32 |
| `of_property_read_string` | 读取字符串 |
| `of_n_addr_cells` | 获取 `#address-cells` |
| `of_n_size_cells` | 获取 `#size-cells` |

第44章例子：

```c
ret = of_property_read_string(dtsled.nd, "status", &str);
ret = of_property_read_u32_array(dtsled.nd, "reg", regdata, 10);
```

### 43.9.4 其他常用 OF 函数（重要度：★★★★★）

| 函数 | 作用 |
|---|---|
| `of_device_is_compatible` | 判断节点是否包含某个 compatible |
| `of_get_address` | 获取 `reg` 等地址属性 |
| `of_translate_address` | 将设备树地址转换为物理地址 |
| `of_address_to_resource` | 将 `reg` 转成 `resource` |
| `of_iomap` | 根据 `reg` 直接完成物理地址到虚拟地址映射 |

### of_iomap 和 ioremap 的关系

以前写法：

```c
virt = ioremap(phys_addr, size);
```

设备树写法：

```c
virt = of_iomap(node, index);
```

`of_iomap` 本质上会读取节点 `reg` 中第 `index` 段地址，然后完成映射。

第44章会出现：

```c
IMX6U_CCM_CCGR1  = of_iomap(dtsled.nd, 0);
SW_MUX_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
SW_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
GPIO1_DR          = of_iomap(dtsled.nd, 3);
GPIO1_GDIR        = of_iomap(dtsled.nd, 4);
```

---

## 本章个人提问与理解记录

### 1. 43.2 看不懂能不能先放着？

不能整节跳过，但可以先半懂。

最低要掌握：

```text
DTS 是源码
DTB 是编译后的二进制
DTC 是编译工具
U-Boot 把 DTB 传给 Linux
Linux 解析 DTB 后，驱动通过 OF 函数读取
```

不用现在深挖 DTC 内部源码。

### 2. 设备树是不是代码逻辑？

不是。

设备树不负责执行硬件操作，它只是描述硬件。

```text
DTS：硬件说明书
驱动：操作硬件的代码
```

### 3. 设备节点、/dev 设备节点、device_create 的节点是不是一回事？

不是，要分清三个“节点”：

| 名称 | 所在位置 | 作用 |
|---|---|---|
| 设备树节点 | `.dts` / `/proc/device-tree` | 描述硬件 |
| `/dev/xxx` 设备节点 | 用户空间 `/dev` | 用户程序访问驱动的入口 |
| `device_create` 创建设备 | 驱动初始化时 | 通知用户空间创建 `/dev/xxx` |

设备树节点解决：

```text
硬件在哪里、是什么、接了哪些资源。
```

`/dev` 节点解决：

```text
用户程序怎么 open/read/write/ioctl 这个驱动。
```

它们有关联，但不是同一个东西。

### 4. compatible 解决什么问题？

解决：

```text
哪个驱动来管理这个设备。
```

不是解决：

```text
用户程序怎么访问这个设备。
```

用户程序访问字符设备仍然需要：

```text
设备号
cdev
class_create
device_create
/dev/xxx
file_operations
```

### 5. reg 为什么有时是寄存器地址，有时是 I2C 地址？

因为 `reg` 的解释权在父节点。

```text
挂在 SoC 总线下：reg 多半是寄存器物理地址 + 长度
挂在 I2C 控制器下：reg 是 I2C 从机地址
挂在 SPI 控制器下：reg 常表示片选号
```

所以看到 `reg` 时先问：

```text
这个节点的父节点是谁？
父节点规定 #address-cells / #size-cells 是多少？
```

### 6. #address-cells = <1> 是不是地址等于 1？

不是。

它表示：

```text
子节点 reg 里的地址部分占 1 个 32 位 cell。
```

同理：

```text
#size-cells = <0> 表示子节点 reg 没有长度字段。
```

### 7. OF 函数要不要全背？

不用全背。

先记第44章马上会用的四个：

```c
of_find_node_by_path()
of_property_read_string()
of_property_read_u32_array()
of_iomap()
```

以后学 GPIO 子系统时再重点看：

```c
of_get_named_gpio()
```

学中断时再看中断相关 OF 函数。

### 8. “驱动到底是啥”在设备树里怎么理解？

驱动就是内核里负责管理某类硬件的软件模块。

它通常做几件事：

```text
1. 和设备树/总线匹配，确认自己要管理哪个硬件
2. 从设备树读取寄存器、GPIO、中断、时钟等资源
3. 初始化硬件
4. 向内核注册接口
5. 给用户空间提供 open/read/write/ioctl 等访问方式
```

设备树只是告诉驱动硬件信息，真正操作硬件的是驱动。

---

## 本章和第44章的衔接

第44章设备树 LED 的核心就是本章综合练习：

```text
1. 在 imx6ull-alientek-emmc.dts 根节点下添加 alphaled 节点
2. 在 alphaled 节点中写 compatible/status/reg
3. 编译生成新的 dtb
4. 驱动用 of_find_node_by_path 找到 /alphaled
5. 用 of_property_read_string 读取 status
6. 用 of_property_read_u32_array 读取 reg
7. 用 of_iomap 映射寄存器
8. 像第42章一样控制 LED
```

也就是：

```text
第42章：寄存器地址写死在 C 文件
第44章：寄存器地址写在 DTS，驱动通过 OF 函数读取
```

---

## 面试速查

### 1. 什么是设备树？

设备树是一种用树形结构描述硬件资源的机制，用于把 ARM 平台板级硬件信息从内核 C 代码中分离出来。DTS 描述硬件，DTC 编译生成 DTB，U-Boot 将 DTB 传给 Linux，内核解析后驱动通过 OF 函数获取硬件资源。

### 2. DTS、DTB、DTC 区别？

```text
DTS：设备树源码
DTB：DTS 编译后的二进制文件，内核实际使用
DTC：设备树编译器，负责 DTS -> DTB
```

### 3. compatible 有什么作用？

`compatible` 用于设备和驱动匹配。设备树节点提供 compatible 字符串，驱动通过 `of_device_id` 匹配表声明支持哪些 compatible，字符串匹配成功后设备即可绑定到对应驱动。

### 4. reg 属性表示什么？

`reg` 描述节点在父节点地址空间中的地址信息。它的具体含义由父节点决定，可能是寄存器物理地址和长度，也可能是 I2C 从机地址或 SPI 片选号。

### 5. #address-cells / #size-cells 表示什么？

它们规定子节点 `reg` 属性中地址和长度字段各占几个 32 位 cell。

### 6. status = "okay" 和 "disabled" 区别？

`okay` 表示设备启用，内核可以为其创建设备并匹配驱动；`disabled` 表示设备禁用，通常不会参与驱动匹配和初始化。

### 7. chosen 节点有什么用？

`chosen` 不是真实硬件节点，主要用于 U-Boot 向 Linux 内核传递启动参数，比如 `bootargs`。

### 8. 驱动如何读取设备树？

先通过 `of_find_node_by_path`、`of_find_compatible_node` 等函数找到节点，再通过 `of_property_read_xxx`、`of_iomap`、`of_get_named_gpio` 等函数读取属性或获取资源。

---



# 第43章 Linux设备树——你这次问题的总结

你这一章真正需要建立的主线是：

```text
设备树负责描述硬件
        ↓
Linux内核解析DTB
        ↓
驱动通过OF函数找到节点、读取属性
        ↓
驱动使用这些硬件资源操作设备
```

设备树本身不是驱动，也不会主动操作硬件。它更像一份交给内核和驱动使用的“硬件配置表”。

---

## 一、设备、驱动和设备树的关系

### 设备

设备就是硬件，例如：

```text
LED
按键
UART控制器
I2C控制器
I2C传感器
SPI设备
```

### 驱动

驱动是操作设备的代码，例如：

```text
初始化硬件
读写寄存器
响应应用程序的read/write
处理中断
```

### 设备树

设备树描述：

```text
板子上有什么设备
设备使用什么地址
设备连接了哪个GPIO
设备使用哪个中断
设备与哪个驱动兼容
```

三者关系：

```text
设备树描述设备
       ↓
compatible帮助匹配驱动
       ↓
驱动读取设备树提供的资源
       ↓
驱动操作真实硬件
```

---

# 二、节点和属性

设备树由节点和属性组成：

```dts
device_node {
    property1 = ...;
    property2 = ...;
};
```

例如：

```dts
led {
    compatible = "alientek,led";
    status = "okay";
    reg = <0x0209c000 0x4>;
};
```

这里：

```text
led             节点
compatible      属性
status          属性
reg             属性
```

设备树的核心就是：

> 按照硬件层级创建节点，然后为节点填写对应属性。

但属性不能随便起名。标准设备通常要按照设备树绑定文档规定的属性名称和格式填写。

---

# 三、`#address-cells` 和 `#size-cells`

这是你前面比较容易混淆的地方。

## 基本含义

```dts
#address-cells = <1>;
#size-cells = <1>;
```

表示该节点的**子节点**在写 `reg` 时：

```text
地址占1个cell
长度占1个cell
```

一个 cell 是32位，也就是4字节。

因此子节点可以写：

```dts
reg = <0x02020000 0x4000>;
```

解释为：

```text
起始地址：0x02020000
地址空间长度：0x4000字节
```

最重要的规则：

> **一个节点自己的 `reg`，由它的父节点的 `#address-cells` 和 `#size-cells` 决定。**

而节点内部写的：

```dts
#address-cells
#size-cells
```

是给它自己的子节点定规则。

---

## 长度是什么意思

例如：

```dts
reg = <0x02020000 0x4000>;
```

长度 `0x4000` 表示：

> 从物理地址 `0x02020000` 开始，这个设备拥有一段长度为 `0x4000` 字节的寄存器地址空间。

地址范围为：

```text
0x02020000 ～ 0x02023FFF
```

计算方式：

```text
结束地址 = 起始地址 + 长度 - 1
```

如果是：

```dts
reg = <0x020C406C 0x04>;
```

就表示从 `0x020C406C` 开始的4字节区域，通常对应一个32位寄存器。

---

## 为什么 I2C 节点里 `#size-cells = <0>`，但自己的 `reg` 仍有长度

代码：

```dts
i2c1: i2c@021a0000 {
    #address-cells = <1>;
    #size-cells = <0>;

    reg = <0x021a0000 0x4000>;
};
```

这里分两层看。

### I2C1控制器自己的 `reg`

```dts
reg = <0x021a0000 0x4000>;
```

由 I2C1 的父节点决定格式。

I2C1是SoC内部的内存映射外设，因此需要：

```text
物理地址 + 寄存器区域长度
```

### I2C1内部的两个 cells 属性

```dts
#address-cells = <1>;
#size-cells = <0>;
```

是给I2C从设备规定格式。

例如：

```dts
sensor@1e {
    reg = <0x1e>;
};
```

这里的 `0x1e` 是I2C从机地址，不是CPU物理地址，也不需要填写地址空间长度。

因此：

```text
I2C控制器：
reg = <物理地址 长度>

I2C从设备：
reg = <I2C从机地址>
```

`#size-cells = <0>` 不是说“长度值等于0”，而是：

> 子节点的 `reg` 里没有长度这一项。

---

# 四、`compatible` 属性

## 普通设备节点的 compatible

例如：

```dts
led {
    compatible = "alientek,led";
};
```

驱动中有：

```c
static const struct of_device_id led_of_match[] = {
    { .compatible = "alientek,led" },
    { }
};
```

匹配成功后，通常执行驱动的：

```c
probe();
```

流程：

```text
普通设备节点compatible
        ↓
匹配外设驱动
        ↓
执行probe
```

---

## 根节点的 compatible

例如：

```dts
/ {
    compatible = "fsl,imx6ull-14x14-evk",
                 "fsl,imx6ull";
};
```

根节点代表整块开发板，所以它不是匹配某个LED或I2C驱动，而是告诉内核：

```text
这是什么开发板
使用什么SoC
应该使用哪套整机/SoC初始化代码
```

区别：

```text
普通节点compatible
→ 匹配外设驱动
→ 执行probe

根节点compatible
→ 匹配整机或SoC支持代码
→ 选择machine_desc等启动初始化代码
```

你当前只需要理解这个区别。

根节点后面讲的：

```text
machine id
machine_desc
DT_MACHINE_START
setup_machine_fdt
of_flat_dt_match_machine
score
```

属于内核启动流程，现在可以略读，不必逐行分析。

---

## compatible为什么可以有多个字符串

例如：

```dts
compatible = "fsl,imx6ull-14x14-evk",
             "fsl,imx6ull";
```

一般按照：

```text
具体型号 → 通用型号
```

排列。

含义是：

```text
优先按照具体开发板匹配
如果没有专用支持，再按照通用I.MX6ULL平台匹配
```

---

# 五、使用 label 向已有节点添加内容

原节点：

```dts
i2c1: i2c@021a0000 {
    status = "disabled";
};
```

其中：

```dts
i2c1:
```

是标签 `label`。

在其他位置可以写：

```dts
&i2c1 {
    status = "okay";
    clock-frequency = <100000>;
};
```

意思是：

> 找到标签为 `i2c1` 的已有节点，继续向里面添加或修改内容。

规则：

```text
属性原来存在 → 覆盖修改
属性原来不存在 → 新增属性
写新的子节点 → 添加子设备
```

例如：

```dts
&i2c1 {
    status = "okay";

    sensor@1e {
        compatible = "vendor,sensor";
        reg = <0x1e>;
    };
};
```

这表示：

```text
启用I2C1控制器
并在I2C1总线上添加地址为0x1e的传感器
```

注意：

```text
label:   给节点定义标签
&label   引用已有节点
```

---

# 六、43.4 创建小型模板设备树

这一节的核心确实比较简单：

```text
创建根节点
创建子节点
填写属性
建立父子层级
使用label和&label
```

你只要理解：

> 设备树是在按照硬件关系“搭一棵树”，并把硬件信息填写进去。

即可。

需要注意设备树只描述硬件，不负责：

```text
读写寄存器
点亮LED
处理中断
传输I2C数据
```

这些工作由驱动完成。

---

# 七、43.6 特殊节点

## `aliases`

示例：

```dts
aliases {
    serial0 = &uart1;
    ethernet0 = &fec1;
};
```

作用：

> 给设备节点设置统一别名，可能影响串口、网卡等设备的编号顺序。

例如：

```text
uart1 → serial0
fec1  → ethernet0
```

现在理解用途即可，不需要研究内核解析源码。

## `chosen`

示例：

```dts
chosen {
    stdout-path = &uart1;
};
```

作用：

> 指定启动阶段的标准输出设备，例如使用UART1输出启动日志。

还可能包含：

```dts
bootargs = "...";
```

`bootargs` 是传给Linux内核的启动参数，例如：

```text
控制台串口
根文件系统位置
波特率
其他内核参数
```

理解作用即可，U-Boot如何修改 `chosen` 可以暂时略读。

---

# 八、43.7 Linux内核解析DTB

这一节知道流程即可：

```text
DTS源文件
   ↓ DTC编译
DTB二进制文件
   ↓ U-Boot传给内核
Linux解析DTB
   ↓
在内存中建立设备树节点
   ↓
驱动通过OF函数访问
```

类似这些函数：

```c
unflatten_device_tree()
__unflatten_device_tree()
unflatten_dt_node()
```

目前不用逐行分析，也不用背。

只需要记住：

> 驱动并不是直接读取DTS文本，而是访问内核解析DTB后形成的设备树节点。

---

# 九、43.8 设备树绑定文档

这一节很实用。

设备树节点里的属性不能完全凭感觉写。要查看绑定文档，确认：

```text
compatible应该写什么
哪些属性必须写
哪些属性可选
属性格式是什么
有没有示例
```

旧内核常见目录：

```text
Documentation/devicetree/bindings/
```

看绑定文档时优先看：

```text
1. compatible
2. Required properties
3. Optional properties
4. Examples
```

实际写设备树通常需要结合三种资料：

```text
绑定文档
→ 告诉你属性名称和格式

芯片参考手册
→ 告诉你寄存器地址、中断号等具体数据

开发板原理图
→ 告诉你外部硬件实际连接方式
```

所以43.8不是要求背文档，而是养成：

> 写设备树前先查对应绑定规范。

---

# 十、43.9 OF操作函数

这是第43章最重要的部分。

前面是在设备树里写数据，43.9开始讲：

> 驱动怎么从设备树里取数据。

整体流程：

```text
找到节点
   ↓
读取属性
   ↓
获得寄存器、GPIO、中断等资源
   ↓
映射并操作硬件
```

---

## `struct device_node`

```c
struct device_node *np;
```

可以理解为：

> 指向内核中某个设备树节点的指针。

后续读取属性时，都要通过它指定“从哪个节点读取”。

---

## 查找节点

重点函数：

```c
of_find_node_by_path()
```

例如：

```c
np = of_find_node_by_path("/alphaled");
```

含义：

> 查找路径为 `/alphaled` 的设备树节点。

成功返回节点指针，失败返回 `NULL`。

其他函数知道用途即可：

```c
of_find_node_by_name()
of_find_node_by_type()
of_find_compatible_node()
```

---

## 读取字符串属性

```c
const char *str;

ret = of_property_read_string(np, "status", &str);
```

含义：

```text
从np指向的节点
读取status属性
把结果放进str
```

成功后：

```text
str → "okay"
```

---

## 读取整数属性

```c
u32 value;

ret = of_property_read_u32(np, "test-value", &value);
```

表示读取一个32位整数。

最常见的是：

```c
of_property_read_u32()
```

---

## 读取整数数组

设备树：

```dts
reg = <0x020C406C 0x04
       0x020E0068 0x04>;
```

驱动：

```c
u32 regdata[4];

ret = of_property_read_u32_array(
    np,
    "reg",
    regdata,
    4
);
```

结果：

```text
regdata[0] = 0x020C406C
regdata[1] = 0x04
regdata[2] = 0x020E0068
regdata[3] = 0x04
```

---

## `of_find_property()`

```c
proper = of_find_property(np, "compatible", NULL);
```

作用：

> 找到节点中的某个属性，并返回对应的 `struct property`。

目前看懂即可。单纯读取数据时，通常直接使用：

```c
of_property_read_string()
of_property_read_u32()
```

更方便。

---

## `of_iomap()`

```c
base = of_iomap(np, 0);
```

可以理解为：

> 从节点的 `reg` 属性中取得第0组地址资源，并完成IO地址映射。

例如：

```dts
reg = <0x020C406C 0x04
       0x020E0068 0x04>;
```

那么：

```text
of_iomap(np, 0)
→ 映射第0组 <0x020C406C 0x04>

of_iomap(np, 1)
→ 映射第1组 <0x020E0068 0x04>
```

它相当于帮助驱动完成：

```text
读取reg
+
获取物理地址和长度
+
ioremap地址映射
```

---

# 十一、这一章的学习优先级

| 内容                       | 当前掌握程度 |
| ------------------------ | ------ |
| 节点、属性、层级                 | 必须理解   |
| `reg`、address/size cells | 必须理解   |
| 普通节点 `compatible`        | 必须理解   |
| 根节点 `compatible`         | 理解区别即可 |
| 根节点匹配源码                  | 暂时略读   |
| `label` 和 `&label`       | 必须会看   |
| `aliases`、`chosen`       | 理解用途即可 |
| 内核解析DTB源码                | 知道流程即可 |
| 绑定信息文档                   | 知道怎么查  |
| OF操作函数                   | 重点掌握   |

---

# 十二、你现在应该形成的完整认识

```text
1. 在DTS中创建节点
2. 使用属性描述硬件资源
3. DTS编译为DTB
4. U-Boot把DTB传给Linux
5. Linux解析DTB
6. 驱动找到对应节点
7. 驱动读取compatible、reg等属性
8. 驱动映射地址、获取资源
9. 驱动真正操作硬件
```

这一章不要求你把所有函数背下来。你现在最重要的是看懂：

```c
of_find_node_by_path()
of_property_read_string()
of_property_read_u32()
of_property_read_u32_array()
of_iomap()
```

以及始终记住两个核心规则：

> **设备树只描述硬件，驱动才操作硬件。**

> **节点自己的 `reg` 格式由父节点的 `#address-cells` 和 `#size-cells` 决定。**


## 一句话总结

第43章的核心不是背 DTS 语法，而是建立“硬件信息写在设备树，驱动通过 OF 函数读取设备树，再初始化硬件”的 Linux 驱动开发思维。

---

## 下一步

继续看第44章设备树 LED 驱动时，重点盯住这条链路：

```text
alphaled DTS 节点
  ↓
reg/status/compatible 属性
  ↓
of_find_node_by_path()
  ↓
of_property_read_string()
  ↓
of_property_read_u32_array()
  ↓
of_iomap()
  ↓
控制 GPIO1_IO03 点灯
```

不要急着背所有 OF 函数，先把第44章 LED 这个最小闭环跑通。

---

## Ubuntu 保存与提交命令

如果要把这份笔记放到 Ubuntu 学习仓库：

```bash
cd ~/linux-learning
code notes/43-驱动书第43章Linux设备树笔记.md
git add notes/43-驱动书第43章Linux设备树笔记.md
git commit -m "notes: add driver chapter 43 device tree"
git push
```

