/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef RUNEOS_PORT_H
#define RUNEOS_PORT_H

#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>

#include <Device/AHCI/FIS.h>

namespace Rune::Device {
#define SATA_DEVICE_TYPES(X)                                                                       \
    X(SATADeviceType, ATA, 0x00000101)                                                             \
    X(SATADeviceType, ATAPI, 0xEB140101)                                                           \
    X(SATADeviceType, SEMB, 0xC33C0101)                                                            \
    X(SATADeviceType, PM, 0x96690101)

    DECLARE_TYPED_ENUM(SATADeviceType, U32, SATA_DEVICE_TYPES, 0x0) // NOLINT

#define INTERFACE_POWER_MANAGEMENT_TYPES(X)                                                        \
    X(InterfacePowerManagement, DEVICE_ABSENT, 0x0)                                                \
    X(InterfacePowerManagement, IPM_ACTIVE, 0x1)                                                   \
    X(InterfacePowerManagement, IPM_PARTIAL, 0x2)                                                  \
    X(InterfacePowerManagement, IPM_SLUMBER, 0x6)                                                  \
    X(InterfacePowerManagement, DEVICE_SLEEP, 0x8)

    DECLARE_TYPED_ENUM(InterfacePowerManagement,
                       U8,
                       INTERFACE_POWER_MANAGEMENT_TYPES,
                       0x10) // NOLINT

#define INTERFACE_SPEED_TYPES(X)                                                                   \
    X(InterfaceSpeed, DEVICE_ABSENT, 0x0)                                                          \
    X(InterfaceSpeed, GEN1_1DOT5Gbps, 0x1)                                                         \
    X(InterfaceSpeed, GEN2_3GBPS, 0x2)                                                             \
    X(InterfaceSpeed, GEN3_6GBPS, 0x3)

    DECLARE_TYPED_ENUM(InterfaceSpeed, U8, INTERFACE_SPEED_TYPES, 0x4) // NOLINT

#define DEVICE_DETECTION_VALUES(X)                                                                 \
    X(DeviceDetection, DEVICE_ABSENT, 0x0)                                                         \
    X(DeviceDetection, DEVICE_DETECTED, 0x1)                                                       \
    X(DeviceDetection, DEVICE_ACTIVE, 0x3)                                                         \
    X(DeviceDetection, DEVICE_INACTIVE, 0x4)

    DECLARE_TYPED_ENUM(DeviceDetection, U8, DEVICE_DETECTION_VALUES, 0x8) // NOLINT

    union CommandListBaseAddress {
        U32 AsUInt32 = 0;
        struct {
            U32 Reserved : 10;
            U32 Base     : 22;
        };
    };

    union FISBaseAddress {
        U32 AsUInt32 = 0;
        struct {
            U32 Reserved : 8;
            U32 Base     : 24;
        };
    };

    struct InterruptStatus {
        U32      DHRS      : 1; // Device to Host Register FIS Interrupt
        U32      PSS       : 1; // PIO Setup FIS Interrupt
        U32      DSS       : 1; // DMA Setup FIS Interrupt
        U32      SDBS      : 1; // Set Device Bits Interrupt
        U32      UFS       : 1; // Unknown FIS Interrupt
        U32      DPS       : 1; // Descriptor Processed
        U32      PCS       : 1; // Port Connect Change Status
        U32      DMPS      : 1; // Device Mechanical Presence Status
        U32      Reserved0 : 14;
        U32      PRCS      : 1; // PhyRdy Change Status
        U32      IPMS      : 1; // Incorrect Port Multiplier Status
        U32      OFS       : 1; // Overflow Status
        uint32_t Reserved1 : 1;
        uint32_t INFS      : 1; // Interface Non-fatal Error Status
        uint32_t IFS       : 1; // Interface Fatal Error Status
        uint32_t HBDS      : 1; // Host Bus Data Error Status
        uint32_t HBFS      : 1; // Host Bus Fatal Error Status
        uint32_t TFES      : 1; // Task File Error Status
        uint32_t CPDS      : 1; // Cold Port Detect Status
    };

    union InterruptEnable {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t DHRE      : 1; // Device to Host Register FIS Interrupt Enable
            uint32_t PSE       : 1; // PIO Setup FIS Interrupt Enable
            uint32_t DSE       : 1; // DMA Setup FIS Interrupt Enable
            uint32_t SDBE      : 1; // Set Device Bits Interrupt Enable
            uint32_t UFE       : 1; // Unknown FIS Interrupt Enable
            uint32_t DPE       : 1; // Descriptor Processed Enable
            uint32_t PCE       : 1; // Port Change Interrupt Enable
            uint32_t DMPE      : 1; // Device Mechanical Presence Enable
            uint32_t Reserved0 : 14;
            uint32_t PRCE      : 1; // PhyRdy Change Interrupt Enable
            uint32_t IPME      : 1; // Incorrect Port Multiplier Enable
            uint32_t OFE       : 1; // Overflow Enable
            uint32_t Reserved1 : 1;
            uint32_t INFE      : 1; // Interface Non-fatal Error Enable
            uint32_t IFE       : 1; // Interface Fatal Error Enable
            uint32_t HBDE      : 1; // Host Bus Data Error Enable
            uint32_t HBFE      : 1; // Host Bus Fatal Error Enable
            uint32_t TFEE      : 1; // Task File Error Enable
            uint32_t CPDE      : 1; // Cold Presence Detect Enable
        };
    };

    struct CommandAndStatus {
        uint32_t ST        : 1; // Start (Enable Command List processing)
        uint32_t SUD       : 1;
        uint32_t POD       : 1;
        uint32_t CLO       : 1;
        uint32_t FRE       : 1; // FIS Receive Enable
        uint32_t Reserved0 : 3;
        uint32_t CCS       : 5;
        uint32_t MPSS      : 1;
        uint32_t FR        : 1; // FIS Receive Running
        uint32_t CR        : 1; // Command List Running
        uint32_t CPS       : 1;
        uint32_t PMA       : 1;
        uint32_t HPCP      : 1;
        uint32_t MPSP      : 1;
        uint32_t CPD       : 1;
        uint32_t ESP       : 1;
        uint32_t FBSCP     : 1;
        uint32_t APSTE     : 1;
        uint32_t ATAPI     : 1;
        uint32_t DLAE      : 1;
        uint32_t ALPE      : 1;
        uint32_t ASP       : 1;
        uint32_t ICC       : 4;
    };

    struct TaskFileData {
        struct {
            U8 ERR : 1;
            U8 CS0 : 2;
            U8 DRQ : 1;
            U8 CS1 : 3;
            U8 BSY : 1;
        } STS;
        U8  ERR;
        U16 Reserved;
    };

    union Signature {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t SectorCountRegister : 8;
            uint32_t LBALowRegister      : 8;
            uint32_t LBAMidRegister      : 8;
            uint32_t LBAHighRegister     : 8;
        };
    };

    struct SerialATAStatus {
        uint32_t DET      : 4;
        uint32_t SPD      : 4;
        uint32_t IPM      : 4;
        uint32_t Reserved : 20;
    };

    struct SerialATAControl {
        uint32_t DET      : 4;
        uint32_t SPD      : 4;
        uint32_t IPM      : 4;
        uint32_t SPM      : 4;
        uint32_t PMP      : 4;
        uint32_t Reserved : 12;
    };

    union SerialATAError {
        uint32_t AsUInt32 = 0;
        struct {
            struct {
                U16 I         : 1;
                U16 M         : 1;
                U16 Reserved0 : 6;
                U16 T         : 1;
                U16 C         : 1;
                U16 P         : 1;
                U16 E         : 1;
                U16 Reserved1 : 4;
            } ERR;

            struct {
                U16 N        : 1;
                U16 I        : 1;
                U16 W        : 1;
                U16 B        : 1;
                U16 D        : 1;
                U16 C        : 1;
                U16 H        : 1;
                U16 S        : 1;
                U16 T        : 1;
                U16 F        : 1;
                U16 X        : 1;
                U16 Reserved : 5;
            } DIAG;
        };
    };

    struct SerialATANotification {
        U16 PMN;
        U16 Reserved;
    };

    struct FISBasedSwitchingControl {
        uint32_t EN        : 1;
        uint32_t DEC       : 1;
        uint32_t SDE       : 1;
        uint32_t Reserved0 : 5;
        uint32_t DEV       : 4;
        uint32_t ADO       : 4;
        uint32_t DWE       : 4;
        uint32_t Reserved1 : 12;
    };

    struct DeviceSleep {
        uint32_t ADSE     : 1;
        uint32_t DSP      : 1;
        uint32_t DETO     : 8;
        uint32_t MDAT     : 5;
        uint32_t DITO     : 10;
        uint32_t DM       : 4;
        uint32_t Reserved : 3;
    };

    /**
     * HBA port according to AHCI 1.3.1
     */
    struct HBAPort {
        CommandListBaseAddress   CLB;
        uint32_t                 CLBU{};
        FISBaseAddress           FB;
        uint32_t                 FBU{};
        InterruptStatus          IS{};
        InterruptEnable          IE;
        CommandAndStatus         CMD{};
        uint32_t                 Reserved{};
        TaskFileData             TFD{};
        Signature                SIG;
        SerialATAStatus          SSTS{};
        SerialATAControl         SCTL{};
        SerialATAError           SERR;
        uint32_t                 SACT{}; // Serial ATA Active
        uint32_t                 CI{};   // Command Issue
        SerialATANotification    SNTF{};
        FISBasedSwitchingControl FBS{};
        DeviceSleep              DEVSLP{};
        Array<U32, 10>           Reserved2; // Registers 0x48-0x6F // NOLINT
        Array<U32, 4>            VS;        // Vendor specific
    };

    struct ReceivedFIS {
        DMASetupFIS  DMA;
        Array<U8, 4> Reserved0;

        PIOSetupFIS   PIO;
        Array<U8, 12> Reserved1; // NOLINT

        RegisterDevice2HostFIS D2H;
        Array<U8, 4>           Reserved3;

        SetDeviceBitsFIS dBits;

        Array<U8, 64> UnknownFIS; // NOLINT
        Array<U8, 96> Reserved4; // NOLINT
    };

    union CommandTableBaseAddress {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t Reserved : 7;
            uint32_t Base     : 25;
        };
    };

    struct CommandHeader {
        uint32_t CFL       : 5 {}; // Command FIS Length, In DW. Range: 2 <= L <= 16
        uint32_t A         : 1 {}; // ATAPI
        uint32_t W         : 1 {}; // 1: Write, 0: Read
        uint32_t P         : 1 {}; // Prefetchable
        uint32_t R         : 1 {}; // Reset
        uint32_t B         : 1 {}; // BIST
        uint32_t C         : 1 {}; // Clear
        uint32_t Reserved0 : 1 {};
        uint32_t PMP       : 4 {}; // Port Multiplier Port
        uint32_t PRDTL : 16 {};    // Physical Region Descriptor Table Length, in entries where each
                                   // entry is 4 DW

        uint32_t PRDBC{}; // Physical Region Descriptor Byte Count, number of bytes transferred

        CommandTableBaseAddress
                 CTBA;    // Command Table Descriptor Base Address, physical, 128 byte aligned
        uint32_t CTBAU{}; // Upper 32 Bits

        Array<U8, 4> Reserved1;
    };

    union DataBaseAddress {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t Reserved : 1;
            uint32_t DBA      : 31;
        };
    };

    struct PRDTEntry {
        DataBaseAddress DBA;
        uint32_t        DBAU{};
        uint32_t        Reserved0{};

        uint32_t DBC       : 22 {};
        uint32_t Reserved1 : 9 {};
        uint32_t I         : 1 {};
    };

    struct CommandTable {
        RegisterHost2DeviceFIS CFIS; // Command FIS
        Array<U8, 44>          CFISPadding; // NOLINT

        Array<U8, 16> ACMD; // ATAPI Command // NOLINT

        Array<U8, 48> Reserved; // NOLINT

        Array<PRDTEntry, 1> PRDT; // Physical Region Descriptor Table // NOLINT

        Array<U8, 112> Reserved1; // Pad to 256 bytes to ensure 128 byte alignment. // NOLINT
    };
} // namespace Rune::Device

#endif // RUNEOS_PORT_H
