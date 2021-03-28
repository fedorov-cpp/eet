#ifndef EET_PROTOCOL_H
#define EET_PROTOCOL_H

#include <cstdint>

namespace Eet {
  namespace Protocol {
    enum class DeviceType : uint8_t {
        INVALID = UINT8_MAX,
        MASTER = 1,
        SLAVE
    };

    enum ErrorBit {
      DUPLICATED_DEVICE_ID = 0,
      CON_WITH_SOME_SLAVES_LOST,
      CON_WITH_ALL_SLAVES_LOST,
      CON_WITH_SOME_MASTERS_LOST,
      CON_WITH_ALL_MASTERS_LOST,
      NO_CONNECTION,
      NO_ACTIVE_SLAVE
    };

    enum class SlaveState : uint8_t {
        INVALID = UINT8_MAX,
        NOT_ACTIVE = 0,
        ACTIVE
    };

    enum class ApproveState: uint8_t {
        INVALID = UINT8_MAX,
        NOT_APPROVED = 0,
        APPROVED
    };

    enum class CmdType : uint8_t {
        INVALID = UINT8_MAX,
        COMPLETE = 0,
        GET_READY,
        FULL_AHEAD,
        HALF_AHEAD,
        SLOW_AHEAD,
        DEAD_SLOW_AHEAD,
        STOP,
        DEAD_SLOW_ASTERN,
        SLOW_ASTERN,
        HALF_ASTERN,
        FULL_ASTERN
    };

    namespace DeviceId {
      constexpr char INVALID = -1;
      constexpr char MIN_DEVICE_ID = 1;
      constexpr char MAX_DEVICE_ID = 12;
      bool isCorrectId(char id);
    }

    namespace Can {
      enum class Id : char {
          ACTIVATE = 0b00010000,
          HEARTBEAT = 0b00100000,
          CMD = 0b01000000
      };

      struct RawMsg {
        uint32_t m_canId;
        uint32_t m_dlc;
        uint32_t m_dataL;
        uint32_t m_dataH;
      };
    }

    namespace Msg {
      enum Errors {
          INVALID_DEVICE_ID = 0,
          DUPLICATED_DEVICE_ID,
          INVALID_CAN_ID,
          INVALID_CAN_DLC,
          INVALID_DEVICE_TYPE,
          INVALID_SLAVE_STATE,
          INVALID_APPROVE_STATE,
          INVALID_CMD_TYPE
      };

      // common fields for all messages
      struct CommonFields {
        CommonFields(char id, DeviceType type, uint8_t e);
        explicit CommonFields(uint32_t data);

        uint16_t isNotValid();

        const char m_deviceId;
        const DeviceType m_deviceType;
        uint8_t m_errors;
      };

      struct IMsg {
        virtual ~IMsg() = default;
        IMsg(const CommonFields &commonFields, Can::Id canId);
        virtual uint16_t isNotValid() = 0;

        CommonFields m_commonFields;
        const Can::Id m_canId;
      };

      struct Activate final : IMsg {
        explicit Activate(const CommonFields &commonFields);
        explicit operator Can::RawMsg();
        uint16_t isNotValid() override;

        static constexpr uint32_t DLC = 2;
      };

      struct Heartbeat final : IMsg {
        Heartbeat(const CommonFields &commonFields,
                  SlaveState slaveState, ApproveState approveState,
                  CmdType cmdType);
        explicit operator Can::RawMsg();
        uint16_t isNotValid() override;

        SlaveState m_slaveState;
        ApproveState m_approveState;
        CmdType m_cmdType;
        static constexpr uint32_t DLC = 3;
      };

      struct Cmd final : IMsg {
        Cmd(const CommonFields &commonFields, CmdType cmdType);
        explicit operator Can::RawMsg();
        uint16_t isNotValid() override;

        CmdType m_cmdType;
        static constexpr uint32_t DLC = 3;
      };

      struct LogMsg {
        static const uint32_t LOG_MSG_MAX_SIZE = 512UL;

        LogMsg(char deviceId, uint16_t errors, const Can::RawMsg &msg);
        LogMsg(char deviceId, DeviceType deviceType, char errors,
               SlaveState slaveState, ApproveState approveState, CmdType cmdType);
        void clean();

        char m_msg[LOG_MSG_MAX_SIZE];
        uint32_t m_msgSize;
      };
    } // end namespace Msg
  } // end namespace Protocol
} // end namespace Eet

#endif // EET_PROTOCOL_H
