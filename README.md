# emmk-v5

------------

## Change

###### V1.0.0-241128
- [Init] 初始化

###### V1.0.1a-241205
- [Add] 同步V4各类组件 

###### V1.0.1-241202
- [Add] 增加支持SWM18X驱动
- [Add] 增加支持DS18B20驱动

###### V1.0.2-241210
- [Add] 增加支持CW32L010驱动

###### V1.1.0-241211
- [Chg] 更新SWM18X驱动
- [Chg] kernel, 针对klptf增加配置选项
- [Chg] cMOdule, 适配新驱动模型

###### V1.1.1-241211
- [Chg] 更新SWM18X驱动

###### V1.1.2-241211
- [Chg] 更新CW32L010驱动
- [Add] cModule, rdax协议增加防拆

###### V1.1.3-241211
- [Chg] 更新SWM18X驱动

###### V1.1.4-241212
- [Chg] 更新SWM18X驱动
- [FIX] cModule, 修复MQTT初始化可能导致死循环的问题

###### V1.1.5-241212
- [Chg] 更新CW32L010驱动

###### V1.1.6-241213
- [Chg] cModule, pt-jw-x, 增加支持PH配置

###### V1.1.7-241213
- [ADD] 同步最新驱动模型到MM32F03X
- [FIX] 更新CW32L010驱动中复用配置有误

###### V1.1.8-241213
- [CHG] 更新MM32F03X驱动

###### V1.1.9-241217
- [CHG] 更新MM32F03X驱动

###### V1.1.10-241218
- [CHG] 更新CW32L010驱动

###### V1.1.11-241220
- [CHG] Device, 更新GP8211驱动

###### V1.1.12-241220
- [Chg] 更新SWM18X驱动

###### V1.1.13-241220
- [ADD] Device, 增加支持MCP4725

###### V1.1.14-241220
- [Chg] 更新SWM18X驱动

###### V1.1.15-241221
- [Chg] Device, 更新MCP4725

###### V1.1.16-241221
- [FIX] Device, MCP4725修复驱动

###### V1.1.17-241224
- [add] middleware-xmenu, 增加菜单滚动不循环的功能

###### V1.1.18-241226
- [CHG] 更新CW32L010驱动

###### V1.1.19-241226
- [CHG] cMdoule, pt-rdax, 更新

###### V1.2.0-241226
- [ADD] kdgpio, 增加驱动函数

###### V1.2.1-241227
- [ADD] kdgpio, 增加驱动函数
- [CHG] 更新CW32L010驱动
- [CHG] cModule, 调整初始化失败后的复位/上电顺序

###### V1.2.2-250109
- [CHG] cMdoule, 更新重试代码

###### V1.2.3-250110
- [FIX] cMdoule, 修复ml307a-http代码中判断发送成功延时过短
- [ADD] cMdoule, ml307a-http, 增加NTP判断时间重试次数

###### V1.2.4-250110
- [ADD] cMdoule, 增加支持nt26e模组

###### V1.25-250120
- [ADD] 增加GD32E10X驱动

###### V1.26-250120
- [CHG] cModule-ml307, 强制指定115200波特率

###### V1.27-250207
- [ADD] cModule, pt-rm01, 增加支持浊度传感器数值

###### V1.28-250207
- [CHG] cModule, pt-rdax, 加密替换为mbedtls

###### V1.29a-250207
- [ADD] 增加支持CX32/ZB32/CLM32驱动支持
- [ADD] cMdoule, 增加支持power/reset脚有效电平回调

###### V1.30a-250208
- [ADD] cModule, rm01协议浊度数据增加电量

###### V1.30b-250208
- [FIX] CX32/ZB32/CLM32驱动, 修订错误

###### V1.30c-250210
- [FIX] CX32/ZB32/CLM32驱动, 修订错误

###### V1.30d-2502111
- [FIX] CW32驱动, 修订错误
- [ADD] CW32驱动, 增加STTY(BTIM)驱动支持

###### V1.30e-250211
- [FIX] CW32驱动, 修订错误

###### V1.30f-250212
- [FIX] CW32驱动, 修订错误

###### V1.30G-250212
- [FIX] CW32驱动, 修订错误

###### V1.31a-250221
- [FIX] ESP32驱动升级
- [FIX] BL0910兼容升级

###### V1.32a-250222
- [CHG] LWMEM_V2.1 -> LWMEM_V2.2
- [CHG] LWRB_V3.1.0 -> LWRB_V3.2.0

###### V1.32b-250224
- [CHG] CX32相关丢弃, 更变为ZB32支持

###### V1.33a-250224
- [CHG] CX32相关丢弃, 更变为ZB32支持
- [CHG] 更新遗留代码
- [ADD] ZB32支持L030, L003

###### V1.34a-250225
- [FIX] ZB32驱动修复相关问题
- [FIX] CW32驱动修复相关问题

###### V1.34b-250225
- [CHG] cModule, xg307dat发送命令简化

###### V1.35a-250227
- [ADD] Middleware, 增加xbitmap小组件
###### V1.35b-250306
- [ADD] cModule, pt-rdax, g307dat协议增加上报温度
###### V1.35c-250317
- [ADD] cModule, 复位上电功能修改, 第1/3次同时复位+上电
###### V1.36a-250318
- [CHG] cModule, 超时功能以及队列溢出修改
- [CHG] CW32, PWM初始化驱动优化
###### V1.36b-250319
- [CHG] CW32, PWM初始化驱动优化

###### V1.37a-250409
- [CHG] cjson, 关闭realloc功能(防止低ram单片机溢出)
- [ADD] cMoudle, 增加NB63

###### V1.37b-250414
- [CHG] cModule, pt-rm01, ph_ad->ph_ad_x10
- [ADD] cModule, pt-rm01, 增加支持orp

###### V1.38a-250416
- [CHG] 内存重载增加可选realloc
- [ADD] lib, 增加可选lwprtinf

###### V1.39a-250417
Add: 增加支持PY32F07x
###### V1.39b-250417
Chg: PY32F07x驱动更新

###### V1.40a-250427
Add: cModule-g307dat, 增加NTP_URC

###### V1.40b-250427
Chg: CW32, 更新RTC驱动
###### V1.40c-250427
Add: cModule. pt-rdax, 增加mid字段

###### V1.41a-250506
Chg: hal, driver, esp32> 更新驱动
Chg: hal, device, ds18b20> 驱动解耦
Chg: middleware, xfilter, wa> 支持debug
###### V1.41b-250508
Chg: hal, driver, zb32> 更新kdimtd驱动

###### V1.42a-250512
Add: hal, device> 添加RF24驱动支持
###### V1.42b-250512
Add: hal, driver, cw32> 更新SPI驱动

###### V1.43a-250516
Add: hal, driver, ch59x> 增加支持
Add: 增加支持RISCV32_CH系列的编译器选项
Add: middleware, mbedtls> 3.6.0

###### V1.43a-250517
Add: hal, device, cs1237> 增加设备支持
###### V1.44a-250517
Add: hal, device, ev1527> 增加设备支持
Fix: hal, device, cs1237> 修复结构体错误

###### V1.5.1-250523
Add: middleware, modbus> mbs操作回调增加支持开始结束事件

###### V1.5.2-250526
Add: middleware, rilat> 增加配置头文件
Add: cModule, jw-ac> 增加OTA能力
Add: cModule, ec800m> HTTP增加重试3次

###### V1.6.0-250527
Add: middleware, modbus> 增加mbs/mbm报文配置功能
###### V1.6.1.0-250528
Add: middleware, modbus> 增加mbs.userdata字段
Add: middleware, modbus> 增加mbs read/write功能

###### V1.6.2.0-250607
Add: hal, device, mcp4725> 修改结构体以及函数

###### V1.6.3.0-250613
Chg: middleware, xbutton> 修改外部依赖

###### V1.6.4.0-250614
add: middleware, xfilter> 增加卡尔曼滤波

###### V1.6.4.1-250616
chg: middleware, xbutton> 旋转编码器中断增加外部调用

###### V1.6.5.0-250618
add: hal, device, ev1527> 增加软件模拟发送1527编码

###### V1.7.0.0-250618
chg: 整合离线代码
add: hal, driver, gd32e10x> 增加gd32e10x驱动支持

###### V1.7.0.1-250621
fix: 修复ev1527 接收发送代码

###### V1.7.0.2-250627
chg: 修改xmenu模块的头文件引用

###### V1.7.1.0-250703
add: hal, driver, gd32el235> 增加gd32el235驱动支持

###### V1.7.1.1-250703
add: hal, driver, zb32l003> 增加UART中断发送的函数

###### V1.7.1.2-250703
add: hal, driver, cw32l010> 增加UART中断发送的函数

###### V1.7.2.0-250708
add: hal, component, cModule> 增加GNSS-simple格式支持
add: hal, component, cModule> 增加ML307系列的GNSS支持

###### V1.7.2.1-250715
fix: hal, driver, gd32el235> 修复驱动若干问题
add: component, cModule> 添加GNSS-NMEA格式回调
add: component, cModule> 添加IdleRefresh功能

------------
