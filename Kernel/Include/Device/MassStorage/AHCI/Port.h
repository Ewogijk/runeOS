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

    /// Device type identified from the SIG register after reset — AHCI 1.3.1 §3.3.13
    ///
    /// SIG is populated from the first D2H Register FIS sent by the device after COMRESET.
    /// Compare the raw SIG value against these constants to determine what is attached.
    ///
    /// - ATA   (0x00000101): ATA device (hard disk, SSD).
    /// - ATAPI (0xEB140101): ATAPI device (optical drive, tape).
    /// - SEMB  (0xC33C0101): Enclosure Management Bridge.
    /// - PM    (0x96690101): Port Multiplier.
    DECLARE_TYPED_ENUM(SATADeviceType, U32, SATA_DEVICE_TYPES, 0x0) // NOLINT

#define INTERFACE_POWER_MANAGEMENT_TYPES(X)                                                        \
    X(InterfacePowerManagement, DEVICE_ABSENT, 0x0)                                                \
    X(InterfacePowerManagement, IPM_ACTIVE, 0x1)                                                   \
    X(InterfacePowerManagement, IPM_PARTIAL, 0x2)                                                  \
    X(InterfacePowerManagement, IPM_SLUMBER, 0x6)                                                  \
    X(InterfacePowerManagement, DEVICE_SLEEP, 0x8)

    /// Current interface power-management state — AHCI 1.3.1 §3.3.10 (SSTS.IPM, bits [11:8])
    ///
    /// - DEVICE_ABSENT (0x0): No device or communication not established.
    /// - IPM_ACTIVE    (0x1): Interface in active state.
    /// - IPM_PARTIAL   (0x2): Interface in Partial power-management state.
    /// - IPM_SLUMBER   (0x6): Interface in Slumber power-management state.
    /// - DEVICE_SLEEP  (0x8): Interface in DevSleep power-management state.
    DECLARE_TYPED_ENUM(InterfacePowerManagement,
                       U8,
                       INTERFACE_POWER_MANAGEMENT_TYPES,
                       0x10) // NOLINT

#define INTERFACE_SPEED_TYPES(X)                                                                   \
    X(InterfaceSpeed, DEVICE_ABSENT, 0x0)                                                          \
    X(InterfaceSpeed, GEN1_1DOT5Gbps, 0x1)                                                         \
    X(InterfaceSpeed, GEN2_3GBPS, 0x2)                                                             \
    X(InterfaceSpeed, GEN3_6GBPS, 0x3)

    /// Negotiated interface speed — AHCI 1.3.1 §3.3.10 (SSTS.SPD, bits [7:4])
    ///
    /// - DEVICE_ABSENT   (0x0): No device or communication not established.
    /// - GEN1_1DOT5Gbps  (0x1): Gen 1 — 1.5 Gbps.
    /// - GEN2_3GBPS      (0x2): Gen 2 — 3 Gbps.
    /// - GEN3_6GBPS      (0x3): Gen 3 — 6 Gbps.
    DECLARE_TYPED_ENUM(InterfaceSpeed, U8, INTERFACE_SPEED_TYPES, 0x4) // NOLINT

#define DEVICE_DETECTION_VALUES(X)                                                                 \
    X(DeviceDetection, DEVICE_ABSENT, 0x0)                                                         \
    X(DeviceDetection, DEVICE_DETECTED, 0x1)                                                       \
    X(DeviceDetection, DEVICE_ACTIVE, 0x3)                                                         \
    X(DeviceDetection, DEVICE_INACTIVE, 0x4)

    /// Device detection state reported by SSTS.DET — AHCI 1.3.1 §3.3.10 (bits [3:0])
    ///
    /// - DEVICE_ABSENT   (0x0): No device detected, no Phy communication established.
    /// - DEVICE_DETECTED (0x1): Device presence detected but Phy not yet established.
    /// - DEVICE_ACTIVE   (0x3): Device detected and Phy communication established.
    /// - DEVICE_INACTIVE (0x4): Phy in offline mode (SCTL.DET was set to 4).
    DECLARE_TYPED_ENUM(DeviceDetection, U8, DEVICE_DETECTION_VALUES, 0x8) // NOLINT

    /**
     * Port Command List Base Address register (CLB) — AHCI 1.3.1 §3.3.1
     *
     * Holds the lower 32 bits of the physical address of the command list.
     * The command list must be 1 KB aligned; bits [9:0] are always zero.
     */
    union CommandListBaseAddress {
        U32 AsUInt32 = 0;
        struct {
            U32 Reserved : 10;
            /// Physical base address bits [31:10] of the 1 KB-aligned command list.
            U32 Base     : 22;
        };
    };

    /**
     * Port FIS Base Address register (FB) — AHCI 1.3.1 §3.3.3
     *
     * Holds the lower 32 bits of the physical address of the Received FIS structure.
     * The structure must be 256-byte aligned; bits [7:0] are always zero.
     */
    union FISBaseAddress {
        U32 AsUInt32 = 0;
        struct {
            U32 Reserved : 8;
            /// Physical base address bits [31:8] of the 256-byte-aligned Received FIS structure.
            U32 Base     : 24;
        };
    };

    /**
     * Port Interrupt Status register (IS) — AHCI 1.3.1 §3.3.5
     *
     * Each bit is set by hardware when the corresponding event occurs and cleared by software
     * writing 1 to it (RWC), except UFS which is read-only and cleared only by a port reset.
     * Enabled bits in IE will propagate to the HBA-level interrupt status register.
     */
    struct InterruptStatus {
        /// D2H Register FIS received with the Interrupt bit set.
        U32 DHRS      : 1;
        /// PIO Setup FIS received with the Interrupt bit set.
        U32 PSS       : 1;
        /// DMA Setup FIS received with the Interrupt bit set.
        U32 DSS       : 1;
        /// Set Device Bits FIS received with the Interrupt bit set, or the Error bit was set.
        U32 SDBS      : 1;
        /// Unknown FIS type received and stored in the Unknown FIS buffer (read-only; cleared by reset).
        U32 UFS       : 1;
        /// PRD with the Interrupt on Completion (I) bit set was processed.
        U32 DPS       : 1;
        /// Device presence on the port has changed (PhyRdy changed or cold-plug event).
        U32 PCS       : 1;
        /// Mechanical presence switch changed state and cleared CMD.MPSS.
        U32 DMPS      : 1;
        U32 Reserved0 : 14;
        /// PhyRdy signal changed from 1 to 0 while CMD.ST was set (indicates device removal).
        U32 PRCS      : 1;
        /// Command was issued to a port multiplier port that is not present or not configured.
        U32 IPMS      : 1;
        /// HBA received more bytes from the device than the PRD table described (overflow).
        U32 OFS       : 1;
        uint32_t Reserved1 : 1;
        /// Non-fatal Serial ATA interface error (see SERR for details).
        uint32_t INFS      : 1;
        /// Fatal Serial ATA interface error; port is unusable until reset.
        uint32_t IFS       : 1;
        /// HBA encountered a data integrity error during host-bus (DMA) access.
        uint32_t HBDS      : 1;
        /// HBA encountered a fatal host-bus error unrelated to data (e.g., address decode failure).
        uint32_t HBFS      : 1;
        /// TFD.STS.ERR is set, indicating the device reported a command error.
        uint32_t TFES      : 1;
        /// Cold-presence detect signal changed while CMD.CPD is set.
        uint32_t CPDS      : 1;
    };

    /**
     * Port Interrupt Enable register (IE) — AHCI 1.3.1 §3.3.6
     *
     * Each bit enables the corresponding IS bit to assert the port interrupt.
     * Bits that are 0 mask the interrupt; bits that are 1 allow it through.
     * Bit layout mirrors InterruptStatus exactly.
     */
    union InterruptEnable {
        uint32_t AsUInt32 = 0;
        struct {
            /// Enable interrupt on D2H Register FIS with Interrupt bit set.
            uint32_t DHRE      : 1;
            /// Enable interrupt on PIO Setup FIS with Interrupt bit set.
            uint32_t PSE       : 1;
            /// Enable interrupt on DMA Setup FIS with Interrupt bit set.
            uint32_t DSE       : 1;
            /// Enable interrupt on Set Device Bits FIS with Interrupt or Error bit set.
            uint32_t SDBE      : 1;
            /// Enable interrupt on unknown FIS type received.
            uint32_t UFE       : 1;
            /// Enable interrupt when a PRD with I=1 is processed.
            uint32_t DPE       : 1;
            /// Enable interrupt on port connect change (PhyRdy or cold-plug).
            uint32_t PCE       : 1;
            /// Enable interrupt on mechanical presence switch state change.
            uint32_t DMPE      : 1;
            uint32_t Reserved0 : 14;
            /// Enable interrupt on PhyRdy change while ST=1.
            uint32_t PRCE      : 1;
            /// Enable interrupt on incorrect port multiplier configuration.
            uint32_t IPME      : 1;
            /// Enable interrupt on PRD overflow.
            uint32_t OFE       : 1;
            uint32_t Reserved1 : 1;
            /// Enable interrupt on non-fatal Serial ATA interface error.
            uint32_t INFE      : 1;
            /// Enable interrupt on fatal Serial ATA interface error.
            uint32_t IFE       : 1;
            /// Enable interrupt on host-bus data error.
            uint32_t HBDE      : 1;
            /// Enable interrupt on fatal host-bus error.
            uint32_t HBFE      : 1;
            /// Enable interrupt when TFD.STS.ERR is set.
            uint32_t TFEE      : 1;
            /// Enable interrupt on cold-presence detect signal change.
            uint32_t CPDE      : 1;
        };
    };

    /**
     * Port Command and Status register (CMD) — AHCI 1.3.1 §3.3.7
     *
     * Controls the port's command-list DMA engine, FIS receive engine, power state,
     * and miscellaneous port capabilities. Many bits are read-only status indicators.
     */
    struct CommandAndStatus {
        /// Start: 1 = enable command-list DMA processing; 0 = pause (wait for CR to clear).
        uint32_t ST        : 1;
        /// Spin-Up Device: initiates staggered spin-up when CAP.SSS is set.
        uint32_t SUD       : 1;
        /// Power On Device: asserts power to the device when CMD.CPD is set.
        uint32_t POD       : 1;
        /// Command List Override: clears BSY and DRQ in TFD without a reset; auto-cleared by HBA.
        uint32_t CLO       : 1;
        /// FIS Receive Enable: allows received FISes to be posted to the Received FIS structure.
        uint32_t FRE       : 1;
        uint32_t Reserved0 : 3;
        /// Current Command Slot: index of the command slot the HBA is currently issuing (read-only).
        uint32_t CCS       : 5;
        /// Mechanical Presence Switch State: reflects current switch position (read-only).
        uint32_t MPSS      : 1;
        /// FIS Receive Running: set while the FIS receive DMA engine is active (read-only).
        uint32_t FR        : 1;
        /// Command List Running: set while the command-list DMA engine is active (read-only).
        uint32_t CR        : 1;
        /// Cold Presence State: reflects the cold-presence detect pin level (read-only).
        uint32_t CPS       : 1;
        /// Port Multiplier Attached: set when a port multiplier is connected.
        uint32_t PMA       : 1;
        /// Hot Plug Capable Port: set by firmware if the port supports hot-plug (read-only).
        uint32_t HPCP      : 1;
        /// Mechanical Presence Switch Attached to Port (read-only).
        uint32_t MPSP      : 1;
        /// Cold Presence Detection: set when the port has a cold-presence detect pin (read-only).
        uint32_t CPD       : 1;
        /// External SATA Port: set when the port connector is externally accessible (read-only).
        uint32_t ESP       : 1;
        /// FIS-Based Switching Capable Port (read-only).
        uint32_t FBSCP     : 1;
        /// Aggressive Power State Transition Enable: allows automatic PARTIAL/SLUMBER transitions.
        uint32_t APSTE     : 1;
        /// Device Is ATAPI: controls LED behavior; set for ATAPI devices.
        uint32_t ATAPI     : 1;
        /// Drive LED on ATAPI Enable: when set, activity LED is driven for ATAPI commands.
        uint32_t DLAE      : 1;
        /// Aggressive Link Power Management Enable: enables automatic link power-management.
        uint32_t ALPE      : 1;
        /// Aggressive Slumber/Partial: when ALPE=1, prefer Slumber over Partial for transitions.
        uint32_t ASP       : 1;
        /// Interface Communication Control: request a specific link power state (write to transition).
        uint32_t ICC       : 4;
    };

    /**
     * Port Task File Data register (TFD) — AHCI 1.3.1 §3.3.8
     *
     * Read-only shadow of the device's Status and Error registers, updated by the HBA
     * each time a D2H Register FIS is received. BSY and DRQ in STS must both be 0
     * before software may set CMD.ST to 1.
     */
    struct TaskFileData {
        struct {
            /// Error bit (ERR): set when the device completed with an error; check ERR register.
            U8 ERR : 1;
            /// Command-specific status bits [2:1].
            U8 CS0 : 2;
            /// Data Transfer Requested (DRQ): device is ready to transfer data.
            U8 DRQ : 1;
            /// Command-specific status bits [6:4].
            U8 CS1 : 3;
            /// Busy (BSY): device is executing a command; other Status bits are not valid.
            U8 BSY : 1;
        } STS;
        /// Latest value of the ATA Error register; updated whenever DRQ or BSY changes.
        U8  ERR;
        U16 Reserved;
    };

    /**
     * Port Signature register (SIG) — AHCI 1.3.1 §3.3.13
     *
     * Loaded from the first D2H Register FIS received after COMRESET. Compare the raw
     * 32-bit value against the SATADeviceType constants to identify the attached device.
     */
    union Signature {
        uint32_t AsUInt32 = 0;
        struct {
            /// Sector Count register value from the initial D2H FIS.
            uint32_t SectorCountRegister : 8;
            /// LBA Low register value from the initial D2H FIS.
            uint32_t LBALowRegister      : 8;
            /// LBA Mid register value from the initial D2H FIS.
            uint32_t LBAMidRegister      : 8;
            /// LBA High register value from the initial D2H FIS.
            uint32_t LBAHighRegister     : 8;
        };
    };

    /**
     * Port Serial ATA Status register (SSTS) — AHCI 1.3.1 §3.3.10
     *
     * Read-only register reflecting the current physical link state. Software polls
     * SSTS.DET after initiating COMRESET to confirm that the link has been re-established.
     */
    struct SerialATAStatus {
        /// Device detection state — see DeviceDetection enum.
        uint32_t DET      : 4;
        /// Negotiated interface speed — see InterfaceSpeed enum.
        uint32_t SPD      : 4;
        /// Interface power-management state — see InterfacePowerManagement enum.
        uint32_t IPM      : 4;
        uint32_t Reserved : 20;
    };

    /**
     * Port Serial ATA Control register (SCTL) — AHCI 1.3.1 §3.3.11
     *
     * Controls link initialization and power-management policy. To perform COMRESET,
     * write DET=1 (assert reset), wait ≥1 ms, then write DET=0 (deassert).
     */
    struct SerialATAControl {
        /// Device Detection Initialization: write 1 to assert COMRESET, 0 to deassert.
        uint32_t DET      : 4;
        /// Speed Allowed: limit the maximum negotiated speed (0 = no restriction).
        uint32_t SPD      : 4;
        /// Interface Power Management Allowed: bitmask disabling specific PM states
        /// (bit 1 = no PARTIAL, bit 2 = no SLUMBER, bit 4 = no DevSleep).
        uint32_t IPM      : 4;
        /// Select Power Management (port multiplier use only).
        uint32_t SPM      : 4;
        /// Port Multiplier Port (port multiplier use only).
        uint32_t PMP      : 4;
        uint32_t Reserved : 12;
    };

    /**
     * Port Serial ATA Error register (SERR) — AHCI 1.3.1 §3.3.12
     *
     * Accumulates Serial ATA error and diagnostic bits (RWC). Software must clear this
     * register after handling an error and before re-enabling the port. Bits in DIAG
     * reflect physical-layer events; bits in ERR reflect protocol/data errors.
     */
    union SerialATAError {
        uint32_t AsUInt32 = 0;
        struct {
            struct {
                /// Recovered Data Integrity Error: error recovered by the interface.
                U16 I         : 1;
                /// Recovered Communications Error: communications error recovered by retries.
                U16 M         : 1;
                U16 Reserved0 : 6;
                /// Transient Data Integrity Error: transient error not recovered by the interface.
                U16 T         : 1;
                /// Persistent Communication or Data Integrity Error: unrecoverable data or link error.
                U16 C         : 1;
                /// Protocol Error: violation of the SATA protocol was detected.
                U16 P         : 1;
                /// Internal Error: HBA-internal error (e.g., fatal DMA fault).
                U16 E         : 1;
                U16 Reserved1 : 4;
            } ERR;

            struct {
                /// PhyRdy Change: PhyRdy signal changed state.
                U16 N        : 1;
                /// Phy Internal Error: error internal to the Phy layer.
                U16 I        : 1;
                /// Comm Wake: Comm Wake signal was detected on the link.
                U16 W        : 1;
                /// 10B to 8B Decode Error: at least one 10B/8B decode error occurred.
                U16 B        : 1;
                /// Disparity Error: a disparity error was detected on the received data stream.
                U16 D        : 1;
                /// CRC Error: a CRC error was detected on the received FIS.
                U16 C        : 1;
                /// Handshake Error: a handshake error (e.g., R_ERR) was received.
                U16 H        : 1;
                /// Link Sequence Error: incorrect primitive sequence was received.
                U16 S        : 1;
                /// Transport State Transition Error: invalid transport-layer state was detected.
                U16 T        : 1;
                /// Unknown FIS Type: a FIS with an unrecognized type was received.
                U16 F        : 1;
                /// Exchanged: device identity changed (e.g., hot-swap or port multiplier switch).
                U16 X        : 1;
                U16 Reserved : 5;
            } DIAG;
        };
    };

    /**
     * Port Serial ATA Notification register (SNTF) — AHCI 1.3.1 §3.3.14
     *
     * Used with port multipliers that support Asynchronous Notification (AN).
     * Each bit corresponds to a port multiplier port that has signalled a notification.
     * Software clears bits by writing 1 (RWC).
     */
    struct SerialATANotification {
        /// PM Notify: one bit per port multiplier port (bits [15:0]); set when a device sent AN.
        U16 PMN;
        U16 Reserved;
    };

    /**
     * Port FIS-Based Switching Control register (FBS) — AHCI 1.3.1 §3.3.15
     *
     * Controls FIS-Based Switching (FBS) operation when a port multiplier is attached.
     * FBS allows independent command queuing to each device behind the port multiplier.
     */
    struct FISBasedSwitchingControl {
        /// Enable FIS-Based Switching; requires a port multiplier that supports FBS.
        uint32_t EN        : 1;
        /// Device Error Clear: write 1 to clear SDE and DEV fields; auto-cleared by hardware.
        uint32_t DEC       : 1;
        /// Single Device Error: set when exactly one device behind the PM encountered an error.
        uint32_t SDE       : 1;
        uint32_t Reserved0 : 5;
        /// Device To Issue: port multiplier port number to route the next command to.
        uint32_t DEV       : 4;
        /// Active Device Optimization: hint to HBA for optimizing PM port switching order.
        uint32_t ADO       : 4;
        /// Device With Error: port multiplier port that triggered a fatal error.
        uint32_t DWE       : 4;
        uint32_t Reserved1 : 12;
    };

    /**
     * Port Device Sleep register (DEVSLP) — AHCI 1.3.1 §3.3.16
     *
     * Controls the DevSleep power-management state, which allows the device to enter
     * a very low power mode. DITO and MDAT are exchanged with the device during
     * the DevSleep negotiation sequence.
     */
    struct DeviceSleep {
        /// Aggressive Device Sleep Enable: HBA automatically transitions device to DevSleep.
        uint32_t ADSE     : 1;
        /// Device Sleep Present: indicates the DEVSLP signal pin is wired (read-only).
        uint32_t DSP      : 1;
        /// Device Sleep Exit Timeout in ms (0 = 20 ms default; otherwise value × 1 ms).
        uint32_t DETO     : 8;
        /// Minimum Device Attach Time in ms; device must be ready within this window after DEVSLP exit.
        uint32_t MDAT     : 5;
        /// Device Idle to Sleep Timeout in ms; idle time before ADSE triggers DevSleep entry.
        uint32_t DITO     : 10;
        /// DITO Multiplier: effective DITO = DITO × 2^DM (allows coarser timeouts).
        uint32_t DM       : 4;
        uint32_t Reserved : 3;
    };

    /**
     * HBA Port register set — AHCI 1.3.1 §3.3
     *
     * Memory-mapped register block for one AHCI port. The HBA exposes up to 32 of these,
     * each mapped at GHC.PI-determined offsets starting at BAR5 + 0x100 + (port × 0x80).
     * Software must ensure CLB and FB point to valid memory before setting CMD.FRE or CMD.ST.
     */
    struct HBAPort {
        /// Command List Base Address lower 32 bits (CLB); 1 KB aligned.
        CommandListBaseAddress   CLB;
        /// Command List Base Address upper 32 bits (CLBU); forms 64-bit address with CLB.
        uint32_t                 CLBU{};
        /// FIS Base Address lower 32 bits (FB); 256-byte aligned.
        FISBaseAddress           FB;
        /// FIS Base Address upper 32 bits (FBU); forms 64-bit address with FB.
        uint32_t                 FBU{};
        /// Interrupt Status (IS); write 1 to clear individual bits.
        InterruptStatus          IS{};
        /// Interrupt Enable (IE); controls which IS bits assert the port interrupt.
        InterruptEnable          IE;
        /// Command and Status (CMD).
        CommandAndStatus         CMD{};
        uint32_t                 Reserved{};
        /// Task File Data (TFD); read-only shadow of ATA Status and Error registers.
        TaskFileData             TFD{};
        /// Signature (SIG); identifies the device type from the initial D2H Register FIS.
        Signature                SIG;
        /// Serial ATA Status (SSTS); read-only link state (DET, SPD, IPM).
        SerialATAStatus          SSTS{};
        /// Serial ATA Control (SCTL); controls link initialization and PM policy.
        SerialATAControl         SCTL{};
        /// Serial ATA Error (SERR); accumulated error and diagnostic bits (RWC).
        SerialATAError           SERR;
        /// Serial ATA Active (SACT); bitmask of command slots with outstanding NCQ commands.
        uint32_t                 SACT{};
        /// Command Issue (CI); write a bit to issue the corresponding command slot; cleared by HBA on completion.
        uint32_t                 CI{};
        /// Serial ATA Notification (SNTF); port multiplier Asynchronous Notification bits (RWC).
        SerialATANotification    SNTF{};
        /// FIS-Based Switching Control (FBS).
        FISBasedSwitchingControl FBS{};
        /// Device Sleep (DEVSLP).
        DeviceSleep              DEVSLP{};
        Array<U32, 10>           Reserved2; // Registers 0x48–0x6F // NOLINT
        /// Vendor-specific registers (VS); 16 bytes at offset 0x70.
        Array<U32, 4>            VS;
    };

    /**
     * Received FIS structure — AHCI 1.3.1 §4.2.1
     *
     * Memory region pointed to by HBAPort.FB / HBAPort.FBU. The HBA writes incoming
     * FISes to their designated slots here. Must be 256-byte aligned and at least 256 bytes.
     */
    struct ReceivedFIS {
        /// Most recent DMA Setup FIS received (offset 0x00, 28 bytes).
        DMASetupFIS  DMA;
        Array<U8, 4> Reserved0;

        /// Most recent PIO Setup FIS received (offset 0x20, 20 bytes).
        PIOSetupFIS   PIO;
        Array<U8, 12> Reserved1; // NOLINT

        /// Most recent Register D2H FIS received (offset 0x40, 20 bytes).
        RegisterDevice2HostFIS D2H;
        Array<U8, 4>           Reserved3;

        /// Most recent Set Device Bits FIS received (offset 0x58, 8 bytes).
        SetDeviceBitsFIS dBits;

        /// Buffer for unrecognized FIS types (offset 0x60, 64 bytes).
        Array<U8, 64> UnknownFIS; // NOLINT
        Array<U8, 96> Reserved4;  // NOLINT
    };

    /**
     * Command Table Base Address — AHCI 1.3.1 §4.2.2 (CTBA field)
     *
     * Physical address of a CommandTable; must be 128-byte aligned.
     */
    union CommandTableBaseAddress {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t Reserved : 7;
            /// Physical base address bits [31:7] of the 128-byte-aligned CommandTable.
            uint32_t Base     : 25;
        };
    };

    /**
     * Command Header — AHCI 1.3.1 §4.2.2
     *
     * One entry in the command list (32 entries, each 32 bytes). Describes a single
     * command to be issued: where its CommandTable lives, the transfer direction,
     * and how many PRDT entries it contains.
     */
    struct CommandHeader {
        /// Command FIS Length in DW (dwords); valid range 2–16 (8–64 bytes).
        uint32_t CFL       : 5 {};
        /// ATAPI: set for ATAPI commands; causes HBA to send ATAPI Command from ACMD.
        uint32_t A         : 1 {};
        /// Write: 1 = host-to-device (write); 0 = device-to-host (read).
        uint32_t W         : 1 {};
        /// Prefetchable: HBA may prefetch PRD entries and data before the command completes.
        uint32_t P         : 1 {};
        /// Reset: HBA sends a SYNC escape and then issues the FIS; used for software reset.
        uint32_t R         : 1 {};
        /// BIST: HBA sends a BIST Activate FIS and enters loopback mode.
        uint32_t B         : 1 {};
        /// Clear Busy on R_OK: HBA clears BSY in TFD when the device sends R_OK.
        uint32_t C         : 1 {};
        uint32_t Reserved0 : 1 {};
        /// Port Multiplier Port: target port multiplier port for this command.
        uint32_t PMP       : 4 {};
        /// PRDT Length: number of entries in the Physical Region Descriptor Table (0 = no data).
        uint32_t PRDTL : 16 {};

        /// PRDT Byte Count: total bytes transferred; written by HBA after command completion.
        uint32_t PRDBC{};

        /// Command Table Base Address (CTBA); 128-byte aligned physical address.
        CommandTableBaseAddress CTBA;
        /// Command Table Base Address Upper 32 bits (CTBAU); forms 64-bit address with CTBA.
        uint32_t CTBAU{};

        Array<U8, 4> Reserved1;
    };

    /**
     * Physical Region Descriptor Table Entry data base address — AHCI 1.3.1 §4.2.3.3
     *
     * Physical address of the data buffer for this PRDT entry; must be 2-byte aligned.
     */
    union DataBaseAddress {
        uint32_t AsUInt32 = 0;
        struct {
            uint32_t Reserved : 1;
            /// Physical data buffer address bits [31:1] (2-byte aligned).
            uint32_t DBA      : 31;
        };
    };

    /**
     * Physical Region Descriptor Table Entry (PRDT entry) — AHCI 1.3.1 §4.2.3.3
     *
     * Describes one contiguous physical memory buffer for a DMA transfer.
     * Each entry can transfer up to 4 MB - 2 bytes. The DBC value is byte_count - 1
     * (e.g., write 511 to transfer 512 bytes); bit [0] is always read back as 1.
     */
    struct PRDTEntry {
        /// Data Base Address lower 32 bits; 2-byte aligned.
        DataBaseAddress DBA;
        /// Data Base Address upper 32 bits; forms 64-bit address with DBA.
        uint32_t        DBAU{};
        uint32_t        Reserved0{};

        /// Data Byte Count minus 1 (22 bits); max value 0x3FFFFF = 4 MB - 1.
        uint32_t DBC       : 22 {};
        uint32_t Reserved1 : 9 {};
        /// Interrupt on Completion: when set, HBA sets IS.DPS after processing this entry.
        uint32_t I         : 1 {};
    };

    /**
     * Command Table — AHCI 1.3.1 §4.2.3
     *
     * Memory region pointed to by CommandHeader.CTBA/CTBAU. Contains the Command FIS,
     * an optional ATAPI command, and the Physical Region Descriptor Table.
     * Must be 128-byte aligned; this struct is padded to exactly 256 bytes.
     */
    struct CommandTable {
        /// Command FIS sent to the device (H2D Register FIS; up to 64 bytes; actual length from CommandHeader.CFL).
        RegisterHost2DeviceFIS CFIS;
        Array<U8, 44>          CFISPadding; // NOLINT

        /// ATAPI Command bytes (12 or 16 bytes; only used when CommandHeader.A=1). // NOLINT
        Array<U8, 16> ACMD;

        Array<U8, 48> Reserved; // NOLINT

        /// Physical Region Descriptor Table; actual entry count given by CommandHeader.PRDTL. // NOLINT
        Array<PRDTEntry, 1> PRDT;

        Array<U8, 112> Reserved1; // Pad to 256 bytes to ensure 128-byte alignment. // NOLINT
    };
} // namespace Rune::Device

#endif // RUNEOS_PORT_H
