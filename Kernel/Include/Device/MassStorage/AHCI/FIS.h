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

    /// FIS type codes — SATA 3.5a §10.5
    ///
    /// Each FIS begins with this byte so that both ends of the link can dispatch
    /// to the correct handler without inspecting further fields.
    ///
    /// - DMA_SETUP  (0x41): DMA Setup FIS — carries first-party DMA buffer descriptor.
    /// - PIO_SETUP  (0x5F): PIO Setup FIS — precedes each PIO data block, device-to-host.
    /// - REG_H2D    (0x27): Register Host-to-Device FIS — writes ATA command block registers.
    /// - REG_D2H    (0x34): Register Device-to-Host FIS — reports updated command block registers.
    /// - SET_D_BITS (0xA1): Set Device Bits FIS — updates shadow Status/Error bits (used by NCQ).
    DECLARE_TYPED_ENUM(FISType, U8, FIS_TYPES, 0x0) // NOLINT

#define H2D_COMMANDS(X)                                                                            \
    X(H2DCommand, IDENTIFY_DEVICE, 0xEC)                                                           \
    X(H2DCommand, READ_DMA_EXTENDED, 0x25)                                                         \
    X(H2DCommand, WRITE_DMA_EXTENDED, 0x35)

    /// ATA command codes issued via Register H2D FIS — ACS-4 Rev 18
    ///
    /// - IDENTIFY_DEVICE    (0xEC): Returns 512 bytes of device identification data (§7.13).
    /// - READ_DMA_EXTENDED  (0x25): Reads up to 65535 sectors via DMA using 48-bit LBA (§7.21).
    /// - WRITE_DMA_EXTENDED (0x35): Writes up to 65535 sectors via DMA using 48-bit LBA (§7.57).
    DECLARE_TYPED_ENUM(H2DCommand, U8, H2D_COMMANDS, 0x0) // NOLINT

    /**
     * DMA Setup FIS (type 0x41) — SATA 3.5a §10.5.9
     *
     * Transmitted device-to-host (or host-to-device in bi-directional protocols) to
     * communicate the DMA buffer descriptor for an upcoming first-party DMA transfer.
     * The receiver uses this information to set up the DMA engine before data moves.
     */
    struct DMASetupFIS {
        /// FIS type identifier; always 0x41.
        U8 FISType;

        /// Port multiplier port address (bits [3:0]).
        U8 PMPort    : 4;
        U8 Reserved0 : 1;
        /// Transfer direction: 1 = transmitter-to-receiver, 0 = receiver-to-transmitter.
        U8 Direction : 1;
        /// When set, the receiver shall generate an interrupt upon DMA transfer completion.
        U8 Interrupt : 1;
        /// Auto-activate: when set, the receiver activates the DMA engine without waiting for an
        /// Activate FIS.
        U8 AutoActivate : 1;

        Array<U8, 2> Reserved1;

        /// Lower 32 bits of the host physical address identifying the DMA buffer.
        U32 DMABufferID;
        /// Upper 32 bits of the host physical address identifying the DMA buffer.
        U32 DMABufferIDUpper;

        U32 Reserved2;

        /// Byte offset into the DMA buffer where the transfer starts; bits [1:0] shall be 0.
        U32 DMABufferOffset;
        /// Number of bytes to transfer; bit [0] shall be 0.
        U32 DMATransferCount;

        U32 Reserved3;
    };

    /**
     * PIO Setup FIS (type 0x5F) — SATA 3.5a §10.5.11
     *
     * Transmitted device-to-host before each PIO data block to provide the host with
     * the current command-block register state and the number of bytes to transfer.
     * One PIO Setup FIS is sent per data block in a multi-block PIO transfer.
     */
    struct PIOSetupFIS {
        /// FIS type identifier; always 0x5F.
        U8 FISType;

        /// Port multiplier port address (bits [3:0]).
        U8 PMPort    : 4;
        U8 Reserved0 : 1;
        /// Transfer direction: 1 = device-to-host (read), 0 = host-to-device (write).
        U8 Direction : 1;
        /// When set, the device requests an interrupt upon receipt/transfer completion.
        U8 Interrupt : 1;
        U8 Reserved1 : 1;

        /// Value of the Status register at the start of the data transfer.
        U8 Status;
        /// Value of the Error register; valid only when an error is indicated in Status.
        U8 Error;

        /// LBA bits [7:0] from the command block LBA Low register.
        U8 LBALow;
        /// LBA bits [15:8] from the command block LBA Mid register.
        U8 LBAMid;
        /// LBA bits [23:16] from the command block LBA High register.
        U8 LBAHigh;
        /// Contents of the Device register of the command block.
        U8 Device;

        /// LBA bits [31:24] from the shadow register block LBA Low Exp register.
        U8 LBALowS;
        /// LBA bits [39:32] from the shadow register block LBA Mid Exp register.
        U8 LBAMidS;
        /// LBA bits [47:40] from the shadow register block LBA High Exp register.
        U8 LBAHighS;
        U8 Reserved2;

        /// Sector count bits [7:0] from the command block Count register.
        U8 Count;
        /// Sector count bits [15:8] from the shadow register block Count Exp register.
        U8 CountS;
        U8 Reserved3;
        /// Value of the Status register at the end of the data transfer.
        U8 EStatus;

        /// Number of bytes to be transferred in the subsequent Data FIS; bit [0] shall be 0.
        U16 TransferCount;
        U16 Reserved4;
    };

    /**
     * Register Host to Device FIS (type 0x27) — SATA 3.5a §10.5.5
     *
     * Transmitted host-to-device to write the ATA command block registers on the device.
     * The C bit distinguishes a command register write (new ATA command) from a
     * control register write (e.g., software reset via SRST in the Device Control register).
     */
    struct RegisterHost2DeviceFIS {
        /// Device register value for 48-bit LBA commands per ACS-4 §7.21: bit 6 set, all others 0.
        static constexpr U8 DEVICE_READ_DMA_EXT = 0x40;

        /// FIS type identifier; always 0x27.
        U8 FISType = FISType::REG_H2D;

        union {
            U8 AsUInt8 = 0;
            struct {
                /// Port multiplier port address (bits [3:0]).
                U8 PMPort    : 4;
                U8 Reserved0 : 3;
                /// Command/Control: 1 = Command register write (new command), 0 = Control register
                /// write.
                U8 C : 1;
            };
        } DW0B1;

        /// ATA command code to execute (e.g., READ DMA EXT = 0x25).
        U8 Command = 0;
        /// Features register bits [7:0] (command-specific sub-function or flags).
        U8 Features = 0;

        /// LBA bits [7:0] — LBA Low register.
        U8 LBALow = 0;
        /// LBA bits [15:8] — LBA Mid register.
        U8 LBAMid = 0;
        /// LBA bits [23:16] — LBA High register.
        U8 LBAHigh = 0;
        /// Device/Head register; for 48-bit LBA commands bit 6 shall be 1 (ACS-4 §7.21).
        U8 Device = 0;

        /// LBA bits [31:24] — LBA Low Exp register (48-bit addressing).
        U8 LBALowE = 0;
        /// LBA bits [39:32] — LBA Mid Exp register (48-bit addressing).
        U8 LBAMidE = 0;
        /// LBA bits [47:40] — LBA High Exp register (48-bit addressing).
        U8 LBAHighE = 0;
        /// Features register bits [15:8] — Features Exp register (command-specific).
        U8 FeaturesE = 0;

        /// Sector count bits [7:0] — Count register.
        U8 Count = 0;
        /// Sector count bits [15:8] — Count Exp register (48-bit sector count).
        U8 CountE = 0;
        /// Isochronous Command Completion (ICC): time limit for command completion in 100 µs units.
        U8 ICC = 0;
        /// Device Control register: bit 2 (SRST) triggers software reset; bit 1 (nIEN) masks
        /// interrupts.
        U8 Control = 0;

        /// Auxiliary register bits [7:0] (command-specific; zero for most commands).
        U8 Auxiliary0 = 0;
        /// Auxiliary register bits [15:8].
        U8 Auxiliary1 = 0;
        /// Auxiliary register bits [23:16].
        U8 Auxiliary2 = 0;
        /// Auxiliary register bits [31:24].
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
     * Register Device to Host FIS (type 0x34) — SATA 3.5a §10.5.6
     *
     * Transmitted device-to-host to update the host-side shadow copy of the ATA
     * command block registers. Sent on command completion (with or without error)
     * and optionally on status changes. The host reads Status and Error to determine
     * the outcome of the last command.
     */
    struct RegisterDevice2HostFIS {
        /// FIS type identifier; always 0x34.
        U8 FISType;

        /// Port multiplier port address (bits [3:0]).
        U8 PMPort    : 4;
        U8 Reserved0 : 2;
        /// When set, the device requests a host interrupt (maps to HBA port IS.DHRS).
        U8 Interrupt : 1;
        U8 Reserved1 : 1;

        /// ATA Status register value; bit 0 (ERR) set indicates an error occurred.
        U8 Status;
        /// ATA Error register value; valid when Status bit 0 (ERR) is set.
        U8 Error;

        /// LBA bits [7:0] — shadow register block LBA Low register.
        U8 LBALow;
        /// LBA bits [15:8] — shadow register block LBA Mid register.
        U8 LBAMid;
        /// LBA bits [23:16] — shadow register block LBA High register.
        U8 LBAHigh;
        /// Shadow register block Device register.
        U8 Device;

        /// LBA bits [31:24] — shadow register block LBA Low Exp register.
        U8 LBALowS;
        /// LBA bits [39:32] — shadow register block LBA Mid Exp register.
        U8 LBAMidS;
        /// LBA bits [47:40] — shadow register block LBA High Exp register.
        U8 LBAHighS;
        U8 Reserved2;

        /// Sector count bits [7:0] — shadow register block Count register.
        U8 Count;
        /// Sector count bits [15:8] — shadow register block Count Exp register.
        U8           CountS;
        Array<U8, 6> Reserved3; // NOLINT
    };

    /**
     * Set Device Bits FIS (type 0xA1) — SATA 3.5a §10.5.7
     *
     * Transmitted device-to-host to update specific bits of the Status and Error registers
     * in the host-side shadow register block without completing a full command. Primarily
     * used by NCQ (Native Command Queuing) to report per-tag completion or errors while
     * other commands are still outstanding.
     */
    struct SetDeviceBitsFIS {
        /// FIS type identifier; always 0xA1.
        U8 FISType;

        /// Port multiplier port address (bits [3:0]).
        U8 PMPort    : 4;
        U8 Reserved0 : 2;
        /// When set, the device requests a host interrupt.
        U8 Interrupt : 1;
        /// Status Notification: when set, the SActive register has been updated by the device.
        U8 Notification : 1;

        /// New value for Status register bits [2:0] (ERR, IDX, CORR) in the shadow register block.
        U8 StatusLow : 3;
        U8 Reserved1 : 1;
        /// New value for Status register bits [6:4] (DRDY, DSC, DWF) in the shadow register block.
        U8 StatusHigh : 3;
        U8 Reserved2  : 1;

        /// New value for the Error register in the shadow register block.
        U8 Error;

        /// Protocol-specific payload (e.g., NCQ SActive bits indicating completed tags).
        Array<U8, 4> ProtocolSpecific;
    };
} // namespace Rune::Device

#endif // RUNEOS_FIS_H
