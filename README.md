## ReadmeFig文件夹是图床，只是用来显示README文件。
## 作者软件版本 Win10, MATLAB_2020B, Keil MDK 5.31
## 测试过的JLink版本 V9, V10, V11, Pro
# 齐天大圣的哥哥(淘宝店铺/B站/知乎同名)

本文介绍SimCoJLink工具的开发背景、使用方法以及注意事项。SimCoJLink是在Simulink环境下的一组JLink驱动模块，利用JLink调试器盒子快速读写ARM系MCU的数据，并且不占用任何额外引脚与CPU时间，下发参数和上传信号只是将数据拷贝到指定内存位置即可。JLink
Pro传输速度可以高达1.3MB/s，JLink
V9也最高支持400KB/s，完全能够胜任电机控制的算法调试需求。配合Simulink强大的建模能力，可以方便快速的搭建上位机测控模型，绘制信号波形，下发控制命令。
<font color=red>该工具只提供编译文件，不提供源代码，也不会收费。</font>

按照载波频率20kHz上传变量，假设上传4个浮点数据，传输的带宽为
"20e3\* 4\* 4Byte/s = 320e3Byte/s /1024 = 312.5KB/s"

# SimCoJLink介绍

SimCoJLink库共有三个模块: SimCoJLink SET，SimCoJLink
Transmit，SimCoJLink
Receive。分别是对SimCoJLink进行相关设置，发送数据到MCU，读取MCU上传数据的模块。

# SimCoJLink使用方法

## SimCoJLink SET

SimCoJLink SET总共有五个参数可以设置，JLink Interface,
Start Search Address, RTT Search Length, JLink Speed,
JLink Target Core。

-   <font color=blue>JLink Interface</font>:
    JLink调试接口选择，根据电路的连接,选择常用的SWD或者JTAG

-   JLink RTT Block
    Search(<font color =red>搜索MCU内_SEGGER_RTT结构体地址</font>，即MCU内交互内存)

    -   <font color=blue>Start Search Address(HEX)</font>：开始搜索的起始内存地址，可以直接设置MCU的RAM起始内存地址，必须是$0x$开头的16进制。
    -   <font color=blue>RTT Search
        Length(HEX)</font>:内存搜索的最大长度，必须是$0x$开头的16进制，最大值是$0x20000$，超过这个最大值设置会报错。

-   <font color=blue>JLink
    Speed(KHz)</font>:JLink与MCU交互接口的频率，默认是50000KHz，可以不用修改，如果无法支持这个频率，JLink驱动会自动降低。最大设置80000，必须是正整数。

-   <font color=blue>JLink Target Core</font>:
    MCU的核心类型，根据MCU的数据手册选择，例如Cortex-M4,Cortex-M3,Cortex-M0等等。

![SimCoJLink SET 参数界面](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/SimCoJLink_SET.png)

## SimCoJLink Receive

SimCoJLink
Receive是按照设置的采样时间去处理信号，例如设置0.1s，那么每隔0.1s将处理刷新一次数据，这种特殊的信号刷新方式需要更改Simulink示波器模块的设置为，<font color=red>列作为通道(基于帧)</font>。在0.1s时间接收到的所有数据中，应该以$"TT:"$三个字符开始，后面跟上的是所有的数据，最后再加上帧尾的字符串$"ET!"$。具体的MCU端发送逻辑可参考提供的示例。

![SimCoJLink Receive 参数界面](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/SimCoJLink_Receive.png)

SimCoJLink Receive总共有五个参数可以设置，Frame Header, Frame
Terminator, Data Type, Data Size, Block Sample Time(s)。

-   <font color=blue>Frame
    Header</font>:一个采样时间内所有数据称为一帧数据，每帧数据的帧头必须是$"TT:"$这三个字符。此参数无法更改。如果MCU发送的帧头不是此字符串，上位机模型将无法正确解析数据。

-   <font color=blue>Frame Terminator</font>:
    每帧数据的帧尾必须是$"ET!"$这三个字符，此参数无法更改。如果MCU发送的帧尾不是这三个字符，可能收到的数据不是完整的一帧，Simulink通知栏会发出警告，但是不会报错终止模型运行。

-   <font color=blue>Data Type</font>:
    MCU上传变量的数据类型，数据类型一定要设置正确，否则无法正常解析数据。如果MCU上传的是$float$类型，选择$single$，在$Simulink$里面，<font color=red>$single$代表单精度浮点数</font>，和$float$同样意义。

-   <font color=blue>Data
    Size</font>:一帧数据的数据尺寸，此参数是一个数组，前面的数字代表MCU每次发送的变量个数，后面的数字代表此模块采样周期内总共接收的数据次数。如MCU发送频率是10KHz，每次发送3个$uint16$的数据，则在采样周期0.1s时间，收到$1000$次数据，每次数据有三个变量，所以设置为数组$[3\quad 1000]$\
    前面的数字，变量的个数，最大值限制为20。\
    后面的数字，接收变量的次数，最大值限制为5000。

-   <font color=blue>Block Sample Time(s)</font>:
    模块的采样时间，也就是计算周期。此参数必须与MCU上传帧格式对应。MCU发送帧头的周期必须与这里的计算周期相同。默认设置0.1s，然后计算MCU发送的帧方式编码，并设置此模块的Data
    Size。

### MCU端示例代码

    // 10KHz
    void UploadParameter(void)
    {
        uint16_t TransBuff[10];
        static uint16_t count;
        uint8_t TransHeader[] = { 'T','T',':'};
        uint8_t TransTerminator[] = {'E', 'T', '!'};

        TransBuff[0] = Phase;
        TransBuff[1] = Phase1;
        TransBuff[2] = Phase2;

        if( count == 0)
        {
            count++;
            SEGGER_RTT_WriteNoLock(0, TransHeader, 3);
            SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
        }
        else if( count == 999)
        {
            count = 0;
            SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
            SEGGER_RTT_WriteNoLock(0, TransTerminator, 3);
        }
        else
        {
            count++;
            SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
        }
    }

## SimCoJLink Transmit

SimCoJLink
Transmit是PC机下发参数到MCU端的模块，下发的参数要求准确，所以下发参数帧末尾会加上CRC16校验码，用来校验数据，校验出错可以放弃该帧数据。
SimCoJLink Transmit 总共有两个参数，都是默认参数，不可修改。

-   <font color=blue>Frame
    Header</font>:此参数默认是$TT:$，每帧下发的参数开头三个字符都是这个帧头。

-   <font color=blue>Block Sample
    Time(s)</font>:参数默认是-1，该参数设置的模块的执行周期，-1代表的是从前面输入的模块继承采样时间。前面模块的采样时间是多少，该模块采样时间和它相等。

![SimCoJLink Transmit 参数界面](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/SimCoJLink_Transmit.png)

-   <font color=red>发送模块执行频率不可比接收模块执行频率高！可以相等。</font>

-   <font color=red>发送帧末尾的2Byte数据是校验值，只校验实际传送的变量，不包含帧头。</font>校验方法和Modbus RTU的CRC16计算方法相同，MCU端示例代码有此校验函数代码。

# SimCoJLink安装与示例

## SimCoJLink安装步骤

![MATLAB添加搜索目录步骤](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/AddDirPS.png)

![查看Simulink库](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/SimCoJLink_LibPS.png)

1.  将所提供的安装包中的JLink_x64.dll文件拷贝到MATLAB安装目录下的"\bin\win64"下，方便运行的时候查找该dll文件。\
    例如在作者电脑上的目录是:"C:\Program Files\Polyspace\2020b\bin\win64"。

2.  将所提供安装包内的SimCoJLink_SimulinkLib文件夹添加到MATLAB的搜索目录，<font color=red>不可含中文</font>。见(图)

3.  打开Simulink的Library查看是否成功添加SimCoJLink库。见(图)

## 示例说明

下载的安装包子目录SimCoJLink_Example下，存在两个测试工程实例。一个主控芯片是\
STM32G474RE的ST官方开发板NUCLEO-G474RE，内核为Cortex-M4。另外一个主控芯片是\
STM32G071RB的官方开发板NUCLEO-G071RB，内核为Cortex-M0。

测试流程：打开Keil工程，下载代码，RUN让程序跑起来。打开上位机Simulink模型，开始监控数据。

这两块开发板自带STLINK，需要把STLINK的SWD线与主控芯片断开，或者将STLINK电源断开，保证STLINK不会干扰SWD的调试线与JLINK连接。具体查看ST官网NUCLEO原理图。

<font color=red>作者实际测试电路板硬件修改情况</font>

-   NUCLEO-G474RE : 去除R9，R10，R13，R18。JP5选5V\_PWR，JLink连接CN4。USB端口供电。

-   NUCLEO-G071RB :
    去除R24电阻，JP2选择CHG，USB插上供电电源，只负责供电。JLink按照线序连接CN11。

# MCU端编码概述

## MCU端编码流程

1.  将作者提供包中SGEEER_MCU目录的文件,见图 ,拷入工程，并添加头文件搜索目录。

    ![RAM大小设置](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/RTT_RAM.png)

2.  设置用来传输信号的RAM大小。<font color=red>在SEGGER_RTT_Conf.h文件95行与99行</font>。该内存大小是MCU用来暂存发送和接收数据的内存区域，理论上是越大越好。因MCU发送到PC速度较快，占用内存相对设置大点，默认可以设置1024Byte。PC发送到MCU，速度较慢，可以设置相对小，设置三倍或者五倍实际每帧数据大小即可，默认设置128Byte。见图。

    ![初始化函数](https://github.com/jiafan1949/SimCoJLink/blob/main/ReadmeFig/RTT_Init.png)

3.  初始化相关的结构体与内存等。<font color=red>在开始发送和接收数据之前初始化</font>。调用图中两个函数。

        // 10KHz  MCU发送到PC
        void UploadParameter(void)
        {
            uint16_t TransBuff[10];
            static uint16_t count;
            uint8_t TransHeader[] = { 'T','T',':'};
            uint8_t TransTerminator[] = {'E', 'T', '!'};

            TransBuff[0] = Phase;
            TransBuff[1] = Phase1;
            TransBuff[2] = Phase2;

            if( count == 0)
            {
                count++;
                SEGGER_RTT_WriteNoLock(0, TransHeader, 3);
                SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
            }
            else if( count == 999)
            {
                count = 0;
                SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
                SEGGER_RTT_WriteNoLock(0, TransTerminator, 3);
            }
            else
            {
                count++;
                SEGGER_RTT_WriteNoLock(0, (uint8_t *)TransBuff, 3*2);
            }
        }

        // while循环中执行或其它低频任务中执行，MCU接收
        // 读取已接收到数据字节数，大于一帧总字节数，开始处理
        if(SEGGER_RTT_GetAvailReadSpace(0) > 5*2+5-1)
        {
            uint8_t RTT_RecvBuffer[15];

            // 从RTT内存读取数据15字节数据到RTT_Buffer内
            SEGGER_RTT_ReadNoLock( 0, RTT_RecvBuffer, 15);

            // 计算CRC16校验码，只对实际数据校验，不包含帧头
            uint16_t CRC16_Result = usMBCRC16(RTT_RecvBuffer+3, 10);
            uint16_t RecvResult = (RTT_RecvBuffer[13] << 8) +
                                    RTT_RecvBuffer[14];
                
            if(CRC16_Result == RecvResult)
            {
                // 校验通过，将5个uint16数据拷贝到memTemp数组中
                memcpy(memTemp, RTT_RecvBuffer+3, 10);
            }
        }   

4.  MCU按照固定频率发送数据，并检查接收数据缓存大小接收数据。
    如示例STM32G071中，发送接收函数见上图代码块。MCU发送到PC按照10KHz频率，每次发送3个uint16变量类型，每1000次为一帧。MCU接收PC数据，每0.1s一帧，每帧数据共有15字节，帧头"TT:"3个字节，5个uint16是10个字节，帧尾是CRC16校验码2个字节。

## MCU端常用API函数

-   SEGGER_RTT_Init

        /***************************************************************
        *  函数描述:
        *    对RTT相关结构体与内存初始化。
        *
        ***************************************************************/
        void SEGGER_RTT_Init (void)

-   SEGGER_RTT_SetTerminal

        /***************************************************************
        *  函数描述:
        *    对RTT的终端进行设置。
        *
        *  输入参数
        *    TerminalId  : 终端号，SimCoJLink必须选择是0。
        *
        *  返回值
        *    >=0，成功设置。
        *    <0 ，设置失败。
        ***************************************************************/
        int SEGGER_RTT_SetTerminal (unsigned char TerminalId)

-   SEGGER_RTT_WriteNoLock

        /***************************************************************
        *  函数描述:
        *    将指定字节数目的内存数据尝试拷贝到RTT上传内存区域，
        *    等待PC机读取到电脑端。如果拷贝不成功，不会死锁等待，
        *    正常退出函数。
        *
        *  输入参数
        *    BufferIndex  : 对于SimCoJLink工具，该参数是0。
        *    pBuffer      : 待拷贝内存的指针。
        *    NumBytes     : 待拷贝内存的字节数目。
        *
        *  返回值
        *    成功写入的字节数目。
        *
        *  注意点
        *    执行该函数之前，必须已调用过初始化函数，
        *    在本说明书4.1节已说明。
        ***************************************************************/
        unsigned SEGGER_RTT_WriteNoLock(unsigned BufferIndex, 
                    const void* pBuffer, unsigned NumBytes)

-   SEGGER_RTT_ReadNoLock

        /***************************************************************
        *  函数描述:
        *    从PC写入到MCU的内存区域读取指定字节数目的数据。
        *    如果读取不成功，不会死锁等待，正常退出函数。
        *
        *  输入参数
        *    BufferIndex  : 对于SimCoJLink工具，该参数是0。
        *    pData        : 待写入的内存指针。
        *    BufferSizde  : 读取的字节数目。
        *
        *  返回值
        *    成功读取的字节数目。
        *
        *  注意点
        *    执行该函数之前，必须已调用过初始化函数，
        *    在本说明书4.1节已说明。
        ***************************************************************/
        unsigned SEGGER_RTT_ReadNoLock(unsigned BufferIndex, 
                        void* pData, unsigned BufferSize)

-   usMBCRC16

        /***************************************************************
        *  函数描述:
        *    计算CRC16校验码
        *
        *  输入参数
        *    pucFrame  : 待校验内存的指针。
        *    usLen     : 待校验内存的字节数目。
        *
        *  返回值
        *    CRC16校验码。
        ***************************************************************/
        unsigned short usMBCRC16(unsigned char * pucFrame,
                        unsigned short usLen)

-   SEGGER_RTT_GetAvailReadSpace

        /***************************************************************
        *  函数描述:
        *    获取PC到MCU端的RAM内存，已经写入的字节数目。
        *
        *  输入参数
        *    BufferIndex  : 对于SimCoJLink工具，该参数是0。
        *
        *  返回值
        *    已经写入的字节数目。
        ***************************************************************/
        unsigned SEGGER_RTT_GetAvailReadSpace (unsigned BufferIndex)

-   SEGGER_RTT_GetAvailWriteSpace

        /***************************************************************
        *  函数描述:
        *    获取MCU到PC端的RAM内存，剩余可写入的字节数目。
        *
        *  输入参数
        *    BufferIndex  : 对于SimCoJLink工具，该参数是0。
        *
        *  返回值
        *    剩余可写入的字节数目。
        *
        *  注意点
        *    返回值是1时，说明可写入的字节为1，已经无法继续写入新数据，
        *    解决方法  1. 更换速度更高的JLink
        *             2. 扩展RAM内存配置，见本说明书4.1
        *             3. 降低发送频率和字节数目
        ***************************************************************/
        unsigned SEGGER_RTT_GetAvailWriteSpace (unsigned BufferIndex)

# 广告

如果对[Simulink代码生成与电机控制](https://item.taobao.com/item.htm?spm=a230r.1.14.39.7b7964ach4MYTv&id=650785163641&ns=1&abbucket=20#detail)感兴趣，侧重永磁无感FOC方向，可以查看链接。
