
################
# SETUP ENV
################

SHELL_CONFIG=~/.bashrc

if [ -z "${FREERTOS_KERNEL_PATH}" ]; then
    echo ENV variable FREERTOS_KERNEL_PATH is not setup
    echo checking submodules in current dir
    if [ -f "FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake" ]; then
        echo freeRTOS Kernel submodule exists in repo. setting up ENV variavle
        echo export FREERTOS_KERNEL_PATH=$PWD/FreeRTOS-Kernel >> $SHELL_CONFIG
    else
        echo "didn't clone the freeRTOS kernel submodule"
        git submodule update --init --recursive && echo export FREERTOS_KERNEL_PATH=$PWD/FreeRTOS-Kernel >> $SHELL_CONFIG
    fi
else
    echo ENV variable FREERTOS_KERNEL_PATH: $FREERTOS_KERNEL_PATH
fi

if [ -z "${PICO_SDK_PATH}" ]; then
    echo ENV variable PICO_SDK_PATH is not setup
    echo checking submodules in current dir
    if [ -f "pico-sdk/external/pico_sdk_import.cmake" ]; then
        echo pico-sdk exists in repo. setting up ENV variavle
        echo export PICO_SDK_PATH=$PWD/pico-sdk >> $SHELL_CONFIG
    else
        echo "didn't clone the freeRTOS kernel submodule"
        git submodule update --init --recursive && echo export PICO_SDK_PATH=$PWD/pico-sdk >> $SHELL_CONFIG
    fi
else
    echo ENV variable PICO_SDK_PATH: ${PICO_SDK_PATH}
fi

source $SHELL_CONFIG
