#include "ARMDisassembler.h"

#include <glog/logging.h>

#include <algorithm>
#include <cstring>
#include <sstream>

namespace remill {

struct ARMDisassembler::PrivateData final {
  std::size_t address_size;
};

/// \todo this looks horrible
ARMDisassembler::ARMDisassembler(bool is_64_bits)
    : CapstoneDisassembler(is_64_bits ? CS_ARCH_ARM64 : CS_ARCH_ARM,
                           is_64_bits ? CS_MODE_LITTLE_ENDIAN : CS_MODE_ARM),
      d(new PrivateData) {
  d->address_size = (is_64_bits ? 64 : 32);
}

ARMDisassembler::~ARMDisassembler() {}

std::string ARMDisassembler::RegName(std::uintmax_t reg_id) const noexcept {
  if (reg_id == 0) return std::string();

  return cs_reg_name(GetCapstoneHandle(), reg_id);
}

std::string ARMDisassembler::InstructionPredicate(
    const CapInstrPtr &caps_instr) const noexcept {
  uint32_t cond_code = 0;

  if (AddressSize() == 32) {
    auto arm = &(caps_instr->detail->arm);
    cond_code = static_cast<uint32_t>(arm->cc);
  } else {
    auto arm64 = &(caps_instr->detail->arm64);
    cond_code = static_cast<uint32_t>(arm64->cc);
  }

  switch (cond_code) {
    case ARM64_CC_INVALID:
      return "";
    case ARM64_CC_EQ:
      return "EQ";
    case ARM64_CC_NE:
      return "NE";
    case ARM64_CC_HS:
      return "HS";
    case ARM64_CC_LO:
      return "LO";
    case ARM64_CC_MI:
      return "MI";
    case ARM64_CC_PL:
      return "PL";
    case ARM64_CC_VS:
      return "VS";
    case ARM64_CC_VC:
      return "VC";
    case ARM64_CC_HI:
      return "HI";
    case ARM64_CC_LS:
      return "LS";
    case ARM64_CC_GE:
      return "GE";
    case ARM64_CC_LT:
      return "LT";
    case ARM64_CC_GT:
      return "GT";
    case ARM64_CC_LE:
      return "LE";
    case ARM64_CC_AL:
      return "AL";
    case ARM64_CC_NV:
      return "";
    default:
      return "";
  }
}

bool ARMDisassembler::PostDisasmHook(const CapInstrPtr &cap_instr) const
    noexcept {
  // strip the instruction name
  for (size_t i = 0; i < std::strlen(cap_instr->mnemonic); i++) {
    if (cap_instr->mnemonic[i] == '.') {
      cap_instr->mnemonic[i] = 0;
      break;
    }
  }

  return true;
}

bool ARMDisassembler::PostDecodeHook(
    const std::unique_ptr<Instruction> &rem_instr,
    const CapInstrPtr &cap_instr) const noexcept {
  static_cast<void>(rem_instr);
  static_cast<void>(cap_instr);

  return true;
}

bool ARMDisassembler::RegName(std::string &name, std::uintmax_t reg_id) const
    noexcept {
  name = RegName(reg_id);
  if (name.empty()) return false;

  return true;
}

bool ARMDisassembler::RegSize(std::size_t &size, const std::string &name) const
    noexcept {
  return true;
}

bool ARMDisassembler::InstrOps(std::vector<Operand> &op_list,
                               const CapInstrPtr &cap_instr) const noexcept {
  if (d->address_size == 32) {
    auto arm_ = &(cap_instr->detail->arm);
    auto num_operands = arm_->op_count;

    for (auto i = 0U; i < num_operands; ++i) {
      auto arm_operand = arm_->operands[i];
      switch (arm_operand.type) {
        case ARM_OP_INVALID:
          break;
        case ARM_OP_REG:
          break;
        case ARM_OP_IMM:
          break;
        case ARM_OP_MEM:
          break;
        case ARM_OP_FP:
          break;
        case ARM_OP_CIMM:
          break;
        case ARM_OP_PIMM:
          break;
        case ARM_OP_SETEND:
          break;
        case ARM_OP_SYSREG:
          break;
      }
    }
  } else if (d->address_size == 64) {
    auto arm64_ = &(cap_instr->detail->arm64);
    auto num_operands = arm64_->op_count;

    for (auto i = 0U; i < num_operands; ++i) {
      auto arm64_operand = arm64_->operands[i];

      switch (arm64_operand.type) {
        case ARM64_OP_INVALID:  // = CS_OP_INVALID (Uninitialized).
          break;
        case ARM64_OP_REG: {
          Operand op;
          op.type = Operand::kTypeRegister;
          op.size = 8;
          op.reg.size = 8;
          op.reg.name = RegName(arm64_operand.reg);
          std::transform(op.reg.name.begin(), op.reg.name.end(),
                         op.reg.name.begin(), ::toupper);
          op_list.push_back(op);
          break;
        }
        case ARM64_OP_IMM: {  // = CS_OP_IMM (Immediate operand).
          Operand op;
          op.type = Operand::kTypeImmediate;
          op.action = Operand::kActionRead;
          op.size = 64;
          op.imm.is_signed = true;
          op.imm.val = arm64_operand.imm;
          op_list.push_back(op);
          break;
        }
        case ARM64_OP_MEM: {  // = CS_OP_MEM (Memory operand).
          Operand op;
          op.type = Operand::kTypeAddress;
          op.size = 64;
          op.addr.base_reg.name = RegName(arm64_operand.mem.base);
          op.addr.index_reg.name = RegName(arm64_operand.mem.index);
          op.addr.displacement = arm64_operand.mem.disp;
          op_list.push_back(op);
          break;
        }
        case ARM64_OP_FP:  // = CS_OP_FP (Floating-Point operand).
          break;
        case ARM64_OP_CIMM:
          break;
        case ARM64_OP_REG_MRS:  // MRS register operand.
          break;
        case ARM64_OP_REG_MSR:  // MSR register operand.
          break;
        case ARM64_OP_PSTATE:  // PState operand.
          break;
        case ARM64_OP_SYS:  // SYS operand for IC/DC/AT/TLBI instructions.
          break;
        case ARM64_OP_PREFETCH:  // Prefetch operand (PRFM).
          break;
        case ARM64_OP_BARRIER:  // Memory barrier operand (ISB/DMB/DSB
                                // instructions).
          break;
      }
    }
  }

  return true;
}

std::size_t ARMDisassembler::AddressSize() const noexcept {
  return d->address_size;
}

Instruction::Category ARMDisassembler::InstrCategory(
    const CapInstrPtr &cap_instr) const noexcept {
  // arm
  if (d->address_size == 32) {
    auto instr_details = cap_instr->detail->arm;

    if (cap_instr->id == ARM_INS_BL || cap_instr->id == ARM_INS_BLX)
      return Instruction::kCategoryDirectFunctionCall;

    else if (cap_instr->id == ARM_INS_BX &&
             instr_details.operands[0].type == ARM_OP_REG &&
             instr_details.operands[0].reg == ARM_REG_LR)
      return Instruction::kCategoryFunctionReturn;

    // arm64
  } else {
    if (cap_instr->id == ARM64_INS_BL)
      return Instruction::kCategoryDirectFunctionCall;

    else if (cap_instr->id == ARM64_INS_RET)
      return Instruction::kCategoryFunctionReturn;
  }
  return Instruction::kCategoryInvalid;
}

std::string ARMDisassembler::SemFuncName(
    const CapInstrPtr &cap_instr, const std::vector<Operand> &op_list) const
    noexcept {
  bool sbit = false;
  std::string mnemonic = cap_instr->mnemonic;
  std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

  std::stringstream function_name;
  function_name << mnemonic;
  function_name << InstructionPredicate(cap_instr);

  if (AddressSize() == 32) {
    auto arm = &(cap_instr->detail->arm);
    sbit = arm->update_flags;
  } else {
    auto arm64 = &(cap_instr->detail->arm64);
    sbit = arm64->update_flags;
  }

  // Add S bit state with the function name
  if (sbit) function_name << "_S1";

  for (const Operand &operand : op_list) {
    switch (operand.type) {
      case Operand::kTypeInvalid: {
        LOG(FATAL) << "Invalid operand type";
      }

      case Operand::kTypeRegister: {
        function_name << "_R" << (operand.reg.size * 8);
        break;
      }

      case Operand::kTypeImmediate: {
        function_name << "_I" << (operand.imm.is_signed ? "i" : "u") << "64";
        break;
      }

      case Operand::kTypeAddress: {
        function_name << "_M" << (operand.addr.address_size * 8);
        break;
      }
    }
  }

  return function_name.str();
}

void ARMDisassembler::EnableThumbMode(bool enabled) noexcept {
  cs_option(GetCapstoneHandle(), CS_OPT_MODE,
            enabled ? CS_MODE_THUMB : CS_MODE_ARM);
}

}  //  namespace remill
