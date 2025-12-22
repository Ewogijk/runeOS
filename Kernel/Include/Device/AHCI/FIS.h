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

#ifndef RUNEOS_FIS_H
#define RUNEOS_FIS_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Collections/Array.h>

namespace Rune::Device {
#define FIS_TYPES(X)                                                                               \
    X(FISType, DMA_SETUP, 0x41)                                                                    \
    X(FISType, PIO_SETUP, 0x5F)                                                                    \
    X(FISType, REG_H2D, 0x27)                                                                      \
    X(FISType, REG_D2H, 0x34)                                                                      \
    X(FISType, SET_D_BITS, 0xA1)

    DECLARE_TYPED_ENUM(FISType, U8, FIS_TYPES, 0x0) // NOLINT

#define H2D_COMMANDS(X)                                                                            \
    X(H2DCommand, IDENTIFY_DEVICE, 0xEC)                                                           \
    X(H2DCommand, READ_DMA_EXTENDED, 0x25)                                                         \
    X(H2DCommand, WRITE_DMA_EXTENDED, 0x35)

    DECLARE_TYPED_ENUM(H2DCommand, U8, H2D_COMMANDS, 0x0) // NOLINT

    /**
     * Handle data transfers between host and a SATA device.
     */
    struct DMASetupFIS {
        U8 FISType; // 0x41

        U8 PMPort       : 4;
        U8 Reserved0    : 1;
        U8 Direction    : 1; // 1: Transmitter -> Receiver, 0: Receiver -> Transmitter
        U8 Interrupt    : 1; // 1: Generate Interrupt on finish
        U8 AutoActivate : 1;

        Array<U8, 2> Reserved1;

        U32 DMABufferID; // Physical Address
        U32 DMABufferIDUpper;

        U32 Reserved2;

        U32 DMABufferOffset;  // Bits0-1 = 0
        U32 DMATransferCount; // Number of bytes to read/write, Bit0 = 0

        U32 Reserved3;
    };

    /**
     * Handle data transfers using PIO mode (via CPU ports).
     */
    struct PIOSetupFIS {
        U8 FISType; // 0x5F

        U8 PMPort    : 4;
        U8 Reserved0 : 1;
        U8 Direction : 1; // 1: Device -> Host, 0: Host -> Device
        U8 Interrupt : 1;
        U8 Reserved1 : 1;

        U8 Status; // Status on start
        U8 Error;  // Error on finish

        U8 LBALow; // Content of the LBA register of the command block
        U8 LBAMid;
        U8 LBAHigh;
        U8 Device; // Content of the device register of the command block

        U8 LBALowS; // Content of the LBA register of the shadow register block
        U8 LBAMidS;
        U8 LBAHighS;
        U8 Reserved2;

        U8 Count;  // Content of the count register of the command block
        U8 CountS; // Content of the count register of the shadow register block
        U8 Reserved3;
        U8 EStatus; // Value of the status register on finish

        U16 TransferCount; // Number of bytes to read/write in Data FIS, Bit0 = 0
        U16 Reserved4;
    };

    /**
     * Sent a command to a device.
     */
    struct RegisterHost2DeviceFIS {
        /// @brief Definition ACS-4, Chapter 7.21: set bit 6 -> 1, rest -> 0
        static constexpr U8 DEVICE_READ_DMA_EXT = 0x40;

        U8 FISType = FISType::REG_H2D;

        union {
            U8 AsUInt8 = 0;
            struct {
                U8 PMPort    : 4;
                U8 Reserved0 : 3;
                U8 C         : 1;
            };
        } DW0B1;

        U8 Command  = 0;
        U8 Features = 0;

        U8 LBALow  = 0;
        U8 LBAMid  = 0;
        U8 LBAHigh = 0;
        U8 Device  = 0;

        U8 LBALowE   = 0;
        U8 LBAMidE   = 0;
        U8 LBAHighE  = 0;
        U8 FeaturesE = 0;

        U8 Count   = 0;
        U8 CountE  = 0;
        U8 ICC     = 0;
        U8 Control = 0;

        U8 Auxiliary0 = 0;
        U8 Auxiliary1 = 0;
        U8 Auxiliary2 = 0;
        U8 Auxiliary3 = 0;

        static auto IdentifyDevice() -> RegisterHost2DeviceFIS {
            RegisterHost2DeviceFIS fis;
            fis.DW0B1.C = 1;
            fis.Command = H2DCommand::IDENTIFY_DEVICE;
            return fis;
        }

        static auto ReadDMAExtended(size_t lba, U16 sectors) -> RegisterHost2DeviceFIS {
            RegisterHost2DeviceFIS fis;
            fis.DW0B1.C  = 1;
            fis.Command  = H2DCommand::READ_DMA_EXTENDED;
            fis.LBALow   = byte_get(lba, 0);
            fis.LBAMid   = byte_get(lba, 1);
            fis.LBAHigh  = byte_get(lba, 2);
            fis.Device   = DEVICE_READ_DMA_EXT;
            fis.LBALowE  = byte_get(lba, 3);
            fis.LBAMidE  = byte_get(lba, 4);
            fis.LBAHighE = byte_get(lba, 5); // NOLINT
            fis.Count    = byte_get(sectors, 0);
            fis.CountE   = byte_get(sectors, 1);

            return fis;
        }

        static auto WriteDMAExtended(size_t lba, U16 sectors) -> RegisterHost2DeviceFIS {
            RegisterHost2DeviceFIS fis;
            fis.DW0B1.C  = 1;
            fis.Command  = H2DCommand::WRITE_DMA_EXTENDED;
            fis.LBALow   = byte_get(lba, 0);
            fis.LBAMid   = byte_get(lba, 1);
            fis.LBAHigh  = byte_get(lba, 2);
            fis.Device   = DEVICE_READ_DMA_EXT;
            fis.LBALowE  = byte_get(lba, 3);
            fis.LBAMidE  = byte_get(lba, 4);
            fis.LBAHighE = byte_get(lba, 5); // NOLINT
            fis.Count    = byte_get(sectors, 0);
            fis.CountE   = byte_get(sectors, 1);
            return fis;
        }
    };

    /**
     * What do i get from the device???
     */
    struct RegisterDevice2HostFIS {
        U8 FISType; // 0x34

        U8 PMPort    : 4;
        U8 Reserved0 : 2;
        U8 Interrupt : 1;
        U8 Reserved1 : 1;

        U8 Status; // Status on start
        U8 Error;  // Error on finish

        U8 LBALow; // Content of the LBA register of the command block
        U8 LBAMid;
        U8 LBAHigh;
        U8 Device; // Content of the device register of the command block

        U8 LBALowS; // Content of the LBA register of the shadow register block
        U8 LBAMidS;
        U8 LBAHighS;
        U8 Reserved2;

        U8           Count;     // Content of the count register of the command block
        U8           CountS;    // Content of the count register of the shadow register block
        Array<U8, 6> Reserved3; // NOLINT See
    };

    /**
     * Set bits in a mystery register.
     */
    struct SetDeviceBitsFIS {
        U8 FISType; // 0xA1

        U8 PMPort       : 4;
        U8 Reserved0    : 2;
        U8 Interrupt    : 1;
        U8 Notification : 1;

        U8 StatusLow  : 3; // New value for bits 0,1,2 of the shadow register block
        U8 Reserved1  : 1;
        U8 StatusHigh : 3; // New value for bits 4,5,6 of the shadow register block
        U8 Reserved2  : 1;

        U8 Error; // Error of the error register of shadow register block

        Array<U8, 4> ProtocolSpecific;
    };
} // namespace Rune::Device

#endif // RUNEOS_FIS_H
