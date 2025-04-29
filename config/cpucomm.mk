# COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
# ALL RIGHTS RESERVED

PRODUCT_PACKAGES += CpuComService
PRODUCT_PACKAGES += cpucomdaemon
#PRODUCT_PACKAGES += cpucomdaemon-tests
#PRODUCT_PACKAGES += libcpucom-tests
#PRODUCT_PACKAGES += libcpucominternal-tests

BOARD_SEPOLICY_DIRS += vendor/melco/efw/CpuComm/sepolicy

PRODUCT_BOOT_JARS += \
    com.mitsubishielectric.ahu.efw.lib.cpucomservice

# BOARD_SEPOLICY_DIRS += vendor/melco/efw/CpuComm/tools/VcpuEmulator/sepolicy
# PRODUCT_PACKAGES += vcpuemulator
# PRODUCT_PACKAGES += emulator-cli
# PRODUCT_PACKAGES += emulator-vcpu.config
# PRODUCT_PACKAGES += emulator-mcpu.config
