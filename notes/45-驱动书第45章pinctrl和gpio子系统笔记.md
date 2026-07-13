# 第45章 pinctrl 和 GPIO 子系统实验

学习日期：2026-07-11  
重要度：★★★★★  
关联章节：第 43 章设备树、第 44 章设备树 LED 驱动、第 46 章蜂鸣器驱动。

## 1. 本章目标

第 44 章中，驱动从设备树读取寄存器地址后，仍自己调用 `ioremap()`、读写 IOMUXC/GPIO 寄存器。

第 45 章改为使用内核已经提供的两套框架：

| 子系统 | 负责什么 | 典型动作 |
| --- | --- | --- |
| pinctrl | 物理 PIN 做什么、PIN 的电气属性 | 把 PAD 复用为 GPIO/UART/I2C；设置上下拉、驱动能力、速度等 |
| GPIO | 使用哪一个 GPIO、输入/输出、读写电平 | 申请 GPIO、设方向、读电平、写电平 |

核心结论：`pinctrl` 管“这个物理脚是什么”，GPIO 子系统管“怎样使用这根 GPIO”。

```text
物理 PAD
  -> pinctrl：复用为 GPIOx_IOy，配置电气特性
  -> GPIO 控制器：提供 GPIOx_IOy 的读写能力
  -> 设备驱动：申请、设方向、读/写此 GPIO
  -> LED/按键/蜂鸣器等外部电路
```

## 2. 45.1 pinctrl 子系统

### 2.1 为什么需要 pinctrl

同一个 SoC 引脚通常支持多种复用功能。例如 `UART1_RTS_B` 可以是 UART、USDHC、CSI 或 `GPIO1_IO19`。直接操作 IOMUXC 寄存器很繁琐，也容易让两个设备抢同一根脚。

pinctrl 驱动读取设备树中的 PIN 描述，完成：

- 复用功能配置。
- 电气属性配置：上下拉、驱动能力、速度、开漏、输入迟滞等。

I.MX6ULL 的 IOMUXC 节点来自 SoC 级 `.dtsi`：

```dts
iomuxc: iomuxc@020e0000 {
    compatible = "fsl,imx6ul-iomuxc";
    reg = <0x020e0000 0x4000>;
};
```

`compatible` 会匹配 `drivers/pinctrl/freescale/pinctrl-imx6ul.c`。驱动匹配后解析 `fsl,pins`，再配置 IOMUXC。

### 2.2 `fsl,pins` 宏的六项信息

示例：

```dts
MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059
```

宏在 `arch/arm/boot/dts/imx6ul-pinfunc.h` 中展开为：

```text
<mux_reg conf_reg input_reg mux_mode input_val> config
<0x0090  0x031C   0x0000    0x5      0x0>      0x17059
```

| 字段 | 含义 | 例子中的作用 |
| --- | --- | --- |
| `mux_reg` | 复用寄存器相对 IOMUXC 基地址的偏移 | `0x020e0000 + 0x0090 = 0x020e0090` |
| `mux_mode` | 写入复用寄存器的模式值 | `0x5`，把 `UART1_RTS_B` 选为 `GPIO1_IO19` |
| `conf_reg` | PAD 电气属性寄存器的偏移 | `0x020e0000 + 0x031c = 0x020e031c` |
| `config` | 写入 `conf_reg` 的电气配置值 | `0x17059`，配置上下拉、驱动能力等 |
| `input_reg` | 输入路径选择寄存器的偏移 | 本例为 `0`，表示此功能不需要设置 |
| `input_val` | 写入输入路径选择寄存器的值 | 本例无效 |

记忆法：

```text
mux_reg + mux_mode       = 去哪里选功能、选哪个功能
conf_reg + config        = 去哪里设电气属性、设成什么样
input_reg + input_val    = 去哪里选输入路径、选哪一路
```

对于同一个物理 PIN，`mux_reg` 和 `conf_reg` 的地址由芯片设计决定，因此固定；复用功能变了，通常变的是 `mux_mode`，电气要求变了，变的是 `config`。

### 2.3 PIN 组与设备状态

同一外设通常需要多个 PIN。例如 UART 有 TX/RX，I2C 有 SCL/SDA，SPI 有 SCLK/MOSI/MISO/CS。把它们放进同一个 pinctrl 组，表示“这是一套同时使用的引脚配置”。

```dts
&iomuxc {
    imx6ul-evk {
        pinctrl_uart1: uart1grp {
            fsl,pins = <
                MX6UL_PAD_UART1_TX_DATA__UART1_DCE_TX 0x1b0b1
                MX6UL_PAD_UART1_RX_DATA__UART1_DCE_RX 0x1b0b1
            >;
        };
    };
};
```

`imx6ul-evk` 在这里主要是板级 PIN 配置的组织容器，不是 UART/I2C 等真实外设本身。`iomuxc` 下保存的是各种设备的 PIN 配置组；真正的 UART、I2C 等外设节点在其他位置。

设备节点用 `pinctrl-names` 和 `pinctrl-N` 选择状态：

```dts
pinctrl-names = "default", "sleep";
pinctrl-0 = <&pinctrl_device_default>;
pinctrl-1 = <&pinctrl_device_sleep>;
```

即第 0 个名字与 `pinctrl-0` 对应，第 1 个名字与 `pinctrl-1` 对应。

## 3. 45.2 GPIO 子系统

### 3.1 GPIO 控制器与 GPIO 消费者

I.MX6ULL 内部的 `gpio1`、`gpio2` 等是**真实的 SoC 内部硬件控制器**，不是虚拟设备。每组通常管理 32 根 GPIO，例如 `GPIO1_IO00` 到 `GPIO1_IO31`。

```dts
gpio1: gpio@0209c000 {
    compatible = "fsl,imx6ul-gpio", "fsl,imx35-gpio";
    reg = <0x0209c000 0x4000>;
    gpio-controller;
    #gpio-cells = <2>;
    interrupt-controller;
    #interrupt-cells = <2>;
};
```

| 项 | 含义 |
| --- | --- |
| `gpio1:` | 标签，供别处写 `&gpio1` 引用 |
| `gpio@0209c000` | GPIO1 控制器节点，寄存器基址为 `0x0209c000` |
| `gpio-controller` | 声明该节点可向其他设备提供 GPIO |
| `#gpio-cells = <2>` | 每次引用 `&gpio1` 后，要写 GPIO 偏移和 flags 两个参数 |
| `interrupt-controller` | GPIO1 还可以向其他设备提供 GPIO 中断 |

例如：

```dts
led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;
```

翻译为：使用 GPIO1 控制器的第 3 根线，即 `GPIO1_IO03`；设备逻辑低电平有效。

注意：`GPIO_ACTIVE_LOW` 描述的是“对这个设备来说，什么物理电平表示有效”。LED 若低有效，驱动输出 `0` 就是点亮，输出 `1` 就是熄灭。

### 3.2 SD 卡检测例子

```dts
MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059

&usdhc1 {
    cd-gpios = <&gpio1 19 GPIO_ACTIVE_LOW>;
};
```

第一行让物理 PAD `UART1_RTS_B` 复用成 `GPIO1_IO19` 并配置电气属性；第二段告诉 USDHC1 驱动，SD 卡检测使用 GPIO1 的第 19 根线，低电平代表检测有效。

二者不是重复：前者是“PIN 怎么配置”，后者是“哪个设备使用哪根 GPIO”。

### 3.3 常用 GPIO API（本书 Linux 4.1 风格）

| API | 作用 |
| --- | --- |
| `of_get_named_gpio(np, prop, index)` | 将设备树 GPIO 属性转换为 Linux GPIO 编号 |
| `of_gpio_named_count(np, prop)` | 获取指定属性内 GPIO 项数量 |
| `gpio_request(gpio, label)` / `gpio_free(gpio)` | 申请 / 释放 GPIO |
| `gpio_direction_input(gpio)` | 设置为输入 |
| `gpio_direction_output(gpio, value)` | 设置为输出，并设定初始电平 |
| `gpio_get_value(gpio)` | 读取电平 |
| `gpio_set_value(gpio, value)` | 输出电平 |

本书使用的是 Linux 4.1 的传统整数 GPIO API；后续学习新内核时会见到 `gpiod_*` 描述符 API，但当前先把本章链路掌握扎实。

## 4. 45.2.4 设备树模板：把两类信息挂到设备上

先在 `iomuxc` 下定义 PIN 组：

```dts
pinctrl_test: testgrp {
    fsl,pins = <
        MX6UL_PAD_GPIO1_IO00__GPIO1_IO00 0x10B0
    >;
};
```

再在根节点创建设备节点：

```dts
test {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_test>;
    gpio = <&gpio1 0 GPIO_ACTIVE_LOW>;
};
```

| 内容 | 给谁看 | 回答的问题 |
| --- | --- | --- |
| `pinctrl_test` 中的 `fsl,pins` | pinctrl 驱动 | 物理 PIN 如何复用、如何配置电气属性？ |
| `pinctrl-0 = <&pinctrl_test>` | 内核 pinctrl 框架 | 该设备 default 状态用哪一组 PIN 配置？ |
| `gpio = <&gpio1 0 ...>` | 设备驱动 / GPIO 框架 | 驱动实际操作哪个 GPIO、什么极性？ |

驱动侧主线：

```text
of_find_node_by_path("/test")
  -> of_get_named_gpio(nd, "gpio", 0)
  -> gpio_request()
  -> gpio_direction_input()/gpio_direction_output()
  -> gpio_get_value()/gpio_set_value()
```

## 5. 45.4 GPIO LED 实验

### 5.1 设备树

在 `&iomuxc` 的 `imx6ul-evk` 下添加 LED 的 PIN 组：

```dts
pinctrl_led: ledgrp {
    fsl,pins = <
        MX6UL_PAD_GPIO1_IO03__GPIO1_IO03 0x10B0
    >;
};
```

在根节点添加设备节点：

```dts
gpioled {
    compatible = "atkalpha-gpioled";
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_led>;
    led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;
    status = "okay";
};
```

最容易踩坑：一根 PIN 在同一时刻只能服务一种功能。`GPIO1_IO03` 若仍被触摸屏 `pinctrl_tsc` 或 `tsc` 节点使用，LED 驱动会产生资源冲突。要同时检查：

- 是否有其他 `fsl,pins` 把它复用为别的功能；
- 是否有其他设备节点引用了 `&gpio1 3`。

### 5.2 驱动相对第 44 章的变化

第 44 章：驱动读取 `reg`，调用 `of_iomap()`，再亲自读写寄存器。

第 45 章：驱动读取 `led-gpio`，调用 GPIO API，不再关心 GPIO1 寄存器的具体地址。

```text
设备树 led-gpio
  -> of_get_named_gpio()
  -> gpio_direction_output(led_gpio, 1)
  -> gpio_set_value(led_gpio, 0/1)
```

字符设备的 `open/write/release` 框架与第 42、44 章基本不变；变化的核心只有“硬件资源获取和 GPIO 操作方式”。

## 6. 个人提问与易混点

### 6.1 `mux_reg`、`conf_reg` 是不是决定具体功能？

不是。它们只决定“改哪个寄存器”。真正决定功能的是 `mux_mode`，真正决定电气特性的是 `config`。

### 6.2 `IOMUXC_SW_PAD_CTL_PAD_UART1_RTS_B` 是什么？

它是 `UART1_RTS_B` 这根物理 PAD 的电气属性寄存器。它与 `IOMUXC_SW_MUX_CTL_PAD_UART1_RTS_B` 是一对：前者管“怎么电气工作”，后者管“复用成什么功能”。

### 6.3 为什么同一个 PIN 的寄存器偏移不变？

因为寄存器属于芯片硬件版图的一部分。相同物理 PIN 的 MUX/PAD 寄存器地址固定；切换功能只是向同一个 MUX 寄存器写不同 `mux_mode`。

### 6.4 `iomuxc`、`imx6ul-evk`、外设节点是什么关系？

`iomuxc` 是真实 IOMUXC 硬件模块；其中的 `imx6ul-evk` 主要组织板级 PIN 组；UART/I2C/LED 等设备节点则描述实际使用这些 PIN 的设备。PIN 配置组不是一个独立外设。

### 6.5 `gpio1` 是虚拟的吗？

不是。`gpio1` 对应 I.MX6ULL 内部真实 GPIO1 控制器。设备树中也有 `chosen`、`aliases`、总线节点、pinctrl 组等逻辑/容器节点，因此“设备树节点”不等于“每个节点都是一个独立外设”。

### 6.6 驱动到底在做什么？

驱动不是单独让整块板子工作，而是让某个硬件控制器或外部设备按 Linux 方式工作。pinctrl 负责 PIN，GPIO/UART/I2C 等控制器驱动负责硬件控制器，设备驱动负责具体设备；众多驱动共同让整块板子可用。

### 6.7 为什么 `pinctrl-0` 和 `led-gpio` 都要写？

`pinctrl-0` 配置物理 PIN；`led-gpio` 把 GPIO 资源交给 LED 驱动使用。一个回答“PIN 应该是什么状态”，另一个回答“驱动该操作哪根 GPIO”。二者缺一不可。

## 7. 面试速查

| 问题 | 简答 |
| --- | --- |
| pinctrl 和 GPIO 的区别？ | pinctrl 配置 PIN 复用与电气属性；GPIO 框架负责 GPIO 的申请、方向和电平读写。 |
| `&gpio1 3 GPIO_ACTIVE_LOW` 怎么解释？ | GPIO1 控制器、第 3 根线 GPIO1_IO03、设备低电平有效。 |
| `#gpio-cells = <2>` 表示什么？ | GPIO 控制器被引用时，phandle 后必须再给 GPIO 偏移和 flags 两个 cell。 |
| 为什么不能只写 `led-gpio`？ | 引脚可能仍是别的复用功能；还必须有 pinctrl 把物理 PAD 配成 GPIO。 |
| 第 44 与 45 章区别？ | 第 44 章驱动直接映射并操作寄存器；第 45 章让内核 pinctrl/GPIO 框架完成底层细节，驱动使用统一 API。 |
| GPIO 控制器是 provider 还是 consumer？ | `gpio1` 是 provider；LED、按键、SD 卡控制器等引用 GPIO 的节点是 consumer。 |

## 8. 一句话总结

第 45 章把“手写寄存器 LED 驱动”升级为 Linux 标准分层方式：设备树描述 PIN 组和 GPIO 资源，pinctrl 配置物理引脚，GPIO 子系统提供统一读写接口，具体驱动只表达设备行为。

## 9. 下一步

第 46 章蜂鸣器与本章 LED 的软件结构几乎一致，重点练习自己完成：确认硬件连接的 GPIO、写 pinctrl 组、在设备节点引用 GPIO、通过 `of_get_named_gpio()` 和 `gpio_set_value()` 控制电平。
