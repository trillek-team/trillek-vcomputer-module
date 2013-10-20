#ifndef __RC1600_OPCODES__
#define __RC1600_OPCODES__ 1

#define IS_PAR3(x)  (((x) & 0x8000) != 0)
#define IS_PAR2(x)  (((x) & 0xC000) == 0x4000)
#define IS_PAR1(x)  (((x) & 0xE000) == 0x2000)
#define IS_PAR1L(x) (((x) & 0xF000) == 0x1000)
#define IS_NOPAR(x) (((x) & 0xF000) == 0)

namespace CPU {
/**
 * 3 parameters op codes
 */
enum PAR3_OPCODE {
    LOAD_LIT        = 0,
    LOAD            = 1,
    LOADB_LIT       = 2,
    LOADB           = 3,

    STORE_LIT       = 4,
    STORE           = 5,
    STOREB_LIT      = 6,
    STOREB          = 7
};

enum PAR2_OPCODE {
    ADD             = 0x00,
    ADD_LIT         = 0x01,
    SUB             = 0x02,
    SUB_LIT         = 0x03,
    ADDC            = 0x04,
    SUBC            = 0x05,
    SUBC_LIT        = 0x06,

    AND             = 0x07,
    OR              = 0x08,
    XOR             = 0x09,

    SLL             = 0x0A,
    SLL_LIT         = 0x0B,
    SRL             = 0x0C,
    SRL_LIT         = 0x0D,
    SRA             = 0x0E,
    SRA_LIT         = 0x0F,

    ROTL            = 0x10,
    ROTL_LIT        = 0x11,
    ROTR            = 0x12,
    ROTR_LIT        = 0x13,

    UMUL            = 0x14,
    UMUL_LIT        = 0x15,
    UDIV            = 0x16,
    UDIV_LIT        = 0x17,
    UMOD            = 0x18,
    UMOD_LIT        = 0x19,

    MUL             = 0x1A,
    MUL_LIT         = 0x1B,
    DIV             = 0x1C,
    DIV_LIT         = 0x1D,
    MOD             = 0x1E,
    MOD_LIT         = 0x1F,

    SWP             = 0x20,

    CPY             = 0x21,
    SET             = 0x22,
    
    BEQ             = 0x23,   
    BEQ_LIT         = 0x24,    
    BNEQ            = 0x25,  
    BNEQ_LIT        = 0x26,
    BG              = 0x27,
    BG_LIT          = 0x28,
    BGE             = 0x29,
    BGE_LIT         = 0x2A,
    BUG             = 0x2B,
    BUG_LIT         = 0x2C,
    BUGE            = 0x2D,
    BUGE_LIT        = 0x2E,  
    BBITS           = 0x2F,
    BCLEAR          = 0x30,

    INP             = 0x31,
    INP_LIT         = 0x32,
    OUT             = 0x33,
    OUT_LIT         = 0x34,

    // Jumps
    LJMP            = 0x3C,
    LCALL           = 0x3D,

    JMP             = 0x3E,
    CALL            = 0x3F
};

enum PAR1_OPCODE {
    NOT             = 0x00,
    NEG             = 0x01,

    XCHG            = 0x02,
    SXTBD           = 0x03,

    PUSH            = 0x04,
    POP             = 0x05,

    GETPC           = 0x06,

    SETFLAGS        = 0x07,
    GETFLAGS        = 0x08,

    SETY            = 0x09,
    SETY_LIT        = 0x0A,
    GETY            = 0x0B,

    INT             = 0x0C,
    INT_LIT         = 0x0D,

    SETIA           = 0x0E,
    SETIA_LIT       = 0x0F,
    GETIA           = 0x10,

    SETCS           = 0x11,
    SETCS_LIT       = 0x12,
    GETCS           = 0x13,

    SETDS           = 0x14,
    SETDS_LIT       = 0x15,
    GETDS           = 0x16,

    SETSS           = 0x17,
    SETSS_LIT       = 0x18,
    GETSS           = 0x19,

    SETIS           = 0x1A,
    SETIS_LIT       = 0x1B,
    GETIS           = 0x1C,

};

enum PAR1L_OPCODE {
    JMP_REL         = 0,
    CALL_REL        = 1
};

enum NOPAR_OPCODE {
    NOP             = 0x000,
    SLEEP           = 0x001,

    RET             = 0x002,
    LRET            = 0x003,
    RFI             = 0x004,

    BOVF            = 0x008,
    BOCF            = 0x009
};

} // End of namespace CPU

#endif // __RC1600_OPCODES__
