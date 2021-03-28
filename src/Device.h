#ifndef EET_DEVICE_H
#define EET_DEVICE_H

#include <list>
#include <unordered_set>
#include <memory>
#include <bitset>

#include "Protocol.h"

namespace Eet {
/**
 * 1. Call pushMsg() to tell device to take msg into account
 * 2. Call update() to tell device to update device's state and errors
 */
  class Device {
    public:
      virtual ~Device() = default;
      explicit Device(Protocol::DeviceType deviceType);

      uint16_t pushMsg(const Protocol::Can::RawMsg &rawMsg);
      char getDeviceId() const;
      char getErrors() const;
      Protocol::DeviceType getDeviceType();
      Protocol::SlaveState getSlaveState();
      Protocol::ApproveState getApproveState();
      Protocol::CmdType getCmdType();
      virtual Protocol::Can::RawMsg getActivateMsg();
      Protocol::Can::RawMsg getHeartbeatMsg();
      virtual Protocol::Can::RawMsg getCmdMsg();
      bool isDuplicatedDeviceId();
      bool isAnyError();
      bool isAnyConLost();
      bool isAnyCon();
      bool isAnyActiveSlave();

      virtual void setDeviceId(char id) = 0;
      virtual void setNumOfMasters(size_t num) = 0;
      virtual void setNumOfSlaves(size_t num) = 0;
      virtual void update() = 0;
      virtual void setCmdType(Protocol::CmdType cmdType) = 0;

    protected:
      bool isConWithSomeSlavesLost();
      bool isConWithAllSlavesLost();
      bool isConWithSomeMastersLost();
      bool isConWithAllMastersLost();
      bool isNoConnection();
      bool isNoActiveSlave();

      virtual void pushActivate(const Protocol::Msg::Activate &msg) = 0;
      virtual void pushHeartbeat(const Protocol::Msg::Heartbeat &msg) = 0;
      virtual void pushCmd(const Protocol::Msg::Cmd &msg) = 0;

      Protocol::DeviceType m_deviceType;
      char m_deviceId;
      char m_errors;
      size_t m_numOfMasters;
      size_t m_numOfSlaves;
      bool m_isAnyDuplicatedId;
      bool m_isAnyActiveSlave;
      Protocol::SlaveState m_slaveState;
      Protocol::CmdType m_cmdType;
      /*
       * For Master - waiting for approve means waiting for any active
       *              Slave response with corresponding cmd type
       * For Slave  - waiting for approve means waiting for teammate button tap
       */
      Protocol::ApproveState m_approveState;

      std::bitset<Protocol::DeviceId::MAX_DEVICE_ID> ma_activeSlaves;
      std::bitset<Protocol::DeviceId::MAX_DEVICE_ID> ma_respondedSlaves;
      std::bitset<Protocol::DeviceId::MAX_DEVICE_ID> ma_respondedMasters;

    private:
      uint16_t registerResponderId(char deviceId, Protocol::DeviceType type);
  };


  class Slave final : public Device {
    public:
      Slave();
      void setDeviceId(char id) override;
      void setNumOfMasters(size_t num) override;
      void setNumOfSlaves(size_t num) override;
      void update() override;
      void activate();

      bool isActivating();
      bool isAnyActiveSlaveResponded();
      Protocol::SlaveState getState();
      void approve(Protocol::CmdType cmdType);

    private:
      void pushActivate(const Protocol::Msg::Activate &msg) override;
      void pushHeartbeat(const Protocol::Msg::Heartbeat &msg) override;
      void pushCmd(const Protocol::Msg::Cmd &msg) override;
      void setCmdType(Protocol::CmdType cmdType) override; // should be called on cmd msg from Master
      Protocol::Can::RawMsg getCmdMsg() override; // should not be called ever

      bool m_isActivating;
  };


  class Master final : public Device {
    public:
      explicit Master();
      void setDeviceId(char id) override;
      void setNumOfMasters(size_t num) override;
      void setNumOfSlaves(size_t num) override;
      void update() override;
      void setCmdType(Protocol::CmdType cmdType) override; // should be called on button tap

    private:
      void pushActivate(const Protocol::Msg::Activate &msg) override;
      void pushHeartbeat(const Protocol::Msg::Heartbeat &msg) override;
      void pushCmd(const Protocol::Msg::Cmd &msg) override;
      Protocol::Can::RawMsg getActivateMsg() override; // should not be called ever
  };

} // end namespace Eet

#endif // EET_DEVICE_H
