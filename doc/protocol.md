## Messages

| CAN message type | CAN ID DEC | CAN ID HEX | CAN ID BIN | Description                             |
|:----------------:|:----------:|:----------:|:----------:|:----------------------------------------|
| ACTIVATE         | 16         | 0x10       | 00010000   | Slave activation                        |
| HEARTBEAT        | 32         | 0x20       | 00100000   |                                         |
| CMD              | 64         | 0x40       | 01000000   | Command from Master                     |

| CAN message type | 0                                 | 1                | 2                                  |
|:----------------:|:---------------------------------:|:----------------:|:----------------------------------:|
| ACTIVATE         | Device ID [0-5] Device Type [6-7] | Errors byte[0-7] |                                    |
| HEARTBEAT        | Device ID [0-5] Device Type [6-7] | Errors byte[0-7] | IsActive[0] IsApproved[1] Cmd[2-7] |
| CMD              | Device ID [0-5] Device Type [6-7] | Errors byte[0-7] |                           Cmd[2-7] |

#### Device ID

Each EET network supposed to have up to 12 devices, therefore total number of devices limited to 36.

#### Device Type

Device type is hardcoded in program.

There are two types of devices:

    1. Master
    2. Slave

#### Errors byte

| Error bit         | Description                                 |
|:-----------------:|:--------------------------------------------|
| 0                 | There is a device with the same Device ID   |
| 1                 | Connection with some Slaves is lost         |
| 2                 | Connection with all Slaves is lost          |
| 3                 | Connection with some Masters is lost        |
| 4                 | Connection with all Masters is lost         |
| 5                 | Connection with all devices is lost         |
| 6                 | No active Slave                             |
| 7                 | RESERVE                                     |

#### IsActive (Slave activation bit)

    0 - NOT_ACTIVE
    1 - ACTIVE

#### Cmd (Command from Master)

    0  - Complete
    1  - Get Ready
    2  - Full Ahead
    3  - Half Ahead
    4  - Slow Ahead
    5  - Dead Slow Ahead
    6  - Stop
    7  - Dead Slow Astern
    8  - Slow Astern
    9  - Half Astern
    10 - Full Astern

## Log messages

    If recieved CAN message:
    $<LogMsgType>,<tDeviceId>,<CanId>,<CanDlc>,<oDeviceId>,<oDuplicatedId>,<DeviceType>,<eDuplicatedDeviceId>,<eConWithSomeSlavesLost>,<eConWithAllSlavesLost>,<eConWithSomeMastersLost>,<eConWithAllMastersLost>,<eNoConnection>,<eNoActiveSlave>,<SlaveState>,<ApproveState>,<CmdType>\r\n
    
    If device state updated:
    $<LogMsgType>,<tDeviceId>,<DeviceType>,<eDuplicatedDeviceId>,<eConWithSomeSlavesLost>,<eConWithAllSlavesLost>,<eConWithSomeMastersLost>,<eConWithAllMastersLost>,<eNoConnection>,<eNoActiveSlave>,<SlaveState>,<ApproveState>,<CmdType>\r\n

#### LogMsgType

    Log message type (defines which device state is described):
    O (Other device)
    T (This device)

#### tDeviceId

    This Device ID
    Can be 1 to 12 or "INVALID_DEVICE_ID"

#### CanId

    A (Activate)
    H (Heartbeat)
    C (Cmd)
    INVALID_CAN_ID

#### CanDlc

    DLC (Data length code)
    Can be INVALID_CAN_DLC or valid DLC.
    
#### oDeviceId
    
    Other device ID
    Can be 1 to 12 or "O_INVALID_DEVICE_ID"
    
#### oDuplicatedId

    Got message from device with the same Device ID.
    Can be:
    O_DUPLICATED_ID (Other with the same Device ID)
    (Empty if Device IDs are different)
    
#### DeviceType

    MASTER
    SLAVE
    INVALID_DEVICE_TYPE
  
#### eDuplicatedDeviceId

    Device is observing other device with the same Device ID.
    Can be:
    E_DUPLICATED_DEVICE_ID
    (Empty if there is no error)

#### eConWithSomeSlavesLost

    Device has lost connection with at least one Slave.
    Can be:
    E_SOME_SLAVES_LOST
    (Empty if there is no error)

#### eConWithAllSlavesLost

    Device has lost connection with all Slaves.
    Can be:
    E_ALL_SLAVES_LOST
    (Empty if there is no error)

#### eConWithSomeMastersLost

    Device has lost connection with at least one Master.
    Can be:
    E_SOME_MASTERS_LOST
    (Empty if there is no error)

#### eConWithAllMastersLost

    Device has lost connecton with all Masters.
    Can be:
    E_ALL_MASTERS_LOST
    (Empty if there is no error)

#### eNoConnection

    Device has lost all connections.
    Can be:
    E_NO_CONNECTION
    (Empty if there is no error)

#### eNoActiveSlave
  
    There is no active slave.
    Can be:
    E_NO_ACTIVE_SLAVE
    (Empty if there is no error)
  
#### SlaveState

    ACTIVE
    NOT_ACTIVE
    INVALID_SLAVE_STATE
    (Empty if Master or CanId=A or CanId=C)
    
#### ApproveState

    APPROVED
    NOT_APPROVED
    INVALID_APPROVE_STATE
    (Empty if CanId=A or CanId=C)
    
#### CmdType

    COMPLETE
    GET_READY
    FULL_AHEAD
    HALF_AHEAD
    SLOW_AHEAD
    DEAD_SLOW_AHEAD
    STOP
    DEAD_SLOW_ASTERN
    SLOW_ASTERN
    HALF_ASTERN
    FULL_ASTERN
    INVALID_CMD_TYPE
    (Empty if CanId=A)
