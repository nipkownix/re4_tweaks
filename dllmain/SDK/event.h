#pragma once
#include "basic_types.h"

struct DatTblWork
{
  char Name_0[48];
  uint8_t FlagBe8_30;
  uint8_t Etc_31;
  int16_t Count_32;
  uint8_t* Dat_34;
  void* Unit_38;
};
assert_size(DatTblWork, 0x3C);

class DatTbl
{
public:
  int NumDatTbl_0;
  DatTblWork* PDatTbl_4;
};
assert_size(DatTbl, 8);

struct EvdInfo
{
  char NameFile_0[32];
  char RoomNo_20[8];
  char EventNo_28[8];
  int SerialNo_30;
  int EvtFlag_34;
  int _pad32_38[2];
};
assert_size(EvdInfo, 0x40);

// EvdHead is file format of EVD files (EVD files also contain EvpHead instances)
struct EvdHead
{
  EvdInfo Info_0;
  int OffsetPac_40;
  int SizePac_44;
  int NumBinTbl_48;
  int OffsetBinTbl_4C;
};
assert_size(EvdHead, 0x50);

enum EvdPacketType : __int32
{
  EvdPacketType_BeginEvt = 0x0,
  EvdPacketType_SetPl = 0x1,
  EvdPacketType_SetEm = 0x2,
  EvdPacketType_SetOm = 0x3,
  EvdPacketType_SetParts = 0x4,
  EvdPacketType_SetList = 0x5,
  EvdPacketType_Cam = 0x6,
  EvdPacketType_CamPos = 0x7,
  EvdPacketType_CamDammy = 0x8,
  EvdPacketType_Pos = 0x9,
  EvdPacketType_PosPl = 0xA,
  EvdPacketType_Mot = 0xB,
  EvdPacketType_Shp = 0xC,
  EvdPacketType_Esp = 0xD,
  EvdPacketType_Lit = 0xE,
  EvdPacketType_Str = 0xF,
  EvdPacketType_Se = 0x10,
  EvdPacketType_Mes = 0x11,
  EvdPacketType_Func = 0x12,
  EvdPacketType_ParentOn = 0x13,
  EvdPacketType_ParentOff = 0x14,
  EvdPacketType_EndPl = 0x15,
  EvdPacketType_EndEm = 0x16,
  EvdPacketType_EndOm = 0x17,
  EvdPacketType_EndParts = 0x18,
  EvdPacketType_EndList = 0x19,
  EvdPacketType_EndEvt = 0x1A,
  EvdPacketType_EndPac = 0x1B,
  EvdPacketType_SetEff = 0x1C,
  EvdPacketType_Fade = 0x1D,
  EvdPacketType_Fog = 0x1E,
  EvdPacketType_Focus = 0x1F,
  EvdPacketType_SetMdt = 0x20,
  EvdPacketType_MaxPackets = 0x21,
};

struct EvpHead
{
  EvdPacketType Type_0;
  uint32_t FlagCommon_4;
  int16_t NoCut_8;
  int16_t Frame_A;
  int16_t Size_C;
  uint16_t NoPac_E;
};
assert_size(EvpHead, 0x10);

enum EventFlag_mb : unsigned __int32
{
  EventFlag_Unk1 = 0x1,
  EventFlag_Unk2 = 0x2,
  EventFlag_Unk3 = 0x4,
  EventFlag_Unk4 = 0x8,
  EventFlag_Unk5 = 0x10,
  EventFlag_ActBtnInited = 0x20,
  EventFlag_Unk7 = 0x40,
  EventFlag_Unk8 = 0x80,
  EventFlag_Unk9 = 0x100,
  EventFlag_Unk10 = 0x200,
  EventFlag_Unk11 = 0x400,
  EventFlag_Unk12 = 0x800,
  EventFlag_Unk13 = 0x1000,
  EventFlag_MdtLoaded = 0x2000,
  EventFlag_Unk15 = 0x4000,
  EventFlag_Unk16 = 0x8000,
  EventFlag_Unk17 = 0x10000,
  EventFlag_Unk18 = 0x20000,
  EventFlag_EffLoaded = 0x40000,
  EventFlag_Unk20 = 0x80000,
  EventFlag_Unk21 = 0x100000,
  EventFlag_Unk22 = 0x200000,
  EventFlag_Unk23 = 0x400000,
  EventFlag_Unk24 = 0x800000,
  EventFlag_Unk25 = 0x1000000,
  EventFlag_Unk26 = 0x2000000,
  EventFlag_Unk27 = 0x4000000,
  EventFlag_Unk28 = 0x8000000,
  EventFlag_Unk29 = 0x10000000,
  EventFlag_Unk30 = 0x20000000,
  EventFlag_Unk31 = 0x40000000,
  EventFlag_Unk32 = 0x80000000,
};

enum ACTION_TYPE32_mb : unsigned __int32
{
  ActBtn_Talk = 0x0,
  ActBtn_Check = 0x1,
  ActBtn_JumpOut = 0x2,
  ActBtn_JumpIn = 0x3,
  ActBtn_JumpDown = 0x4,
  ActBtn_JumpOver = 0x5,
  ActBtn_Push = 0x6,
  ActBtn_Kick = 0x7,
  ActBtn_ClimbUp = 0x8,
  ActBtn_ClimbDown = 0x9,
  ActBtn_KnockDown = 0xA,
  ActBtn_RaiseUp = 0xB,
  ActBtn_Jump = 0xC,
  ActBtn_Look = 0xD,
  ActBtn_LookDown = 0xE,
  ActBtn_BackToTheWall = 0xF,
  ActBtn_Open = 0x10,
  ActBtn_Swim = 0x11,
  ActBtn_JumpOver2 = 0x12,
  ActBtn_Crouch = 0x13,
  ActBtn_Operate = 0x14,
  ActBtn_Help = 0x15,
  ActBtn_Piggyback = 0x16,
  ActBtn_Throw = 0x17,
  ActBtn_Sprint = 0x18,
  ActBtn_Climb = 0x19,
  ActBtn_Jump2 = 0x1A,
  ActBtn_SlideDown = 0x1B,
  ActBtn_Catch = 0x1C,
  ActBtn_PullUp = 0x1D,
  ActBtn_Wait = 0x1E,
  ActBtn_StandBack = 0x1F,
  ActBtn_Hide = 0x20,
  ActBtn_FollowMe = 0x21,
  ActBtn_GiveMeAHand = 0x22,
  ActBtn_GetOn = 0x23,
  ActBtn_GetOff = 0x24,
  ActBtn_Dodge = 0x25,
  ActBtn_Hide2 = 0x26,
  ActBtn_Crawl = 0x27,
  ActBtn_Take = 0x28,
  ActBtn_Cut = 0x29,
  ActBtn_Rotate = 0x2A,
  ActBtn_ShakeOff = 0x2B,
  ActBtn_Suplex = 0x2C,
  ActBtn_Dummy = 0x2D,
  ActBtn_Begin = 0x2E,
  ActBtn_Save = 0x2F,
  ActBtn_Unk48 = 0x30,
  ActBtn_Accelerate = 0x31,
  ActBtn_Accelerate2 = 0x32,
  ActBtn_SendAshley = 0x33,
  ActBtn_QuestionMark = 0x34,
  ActBtn_KnockOver = 0x35,
  ActBtn_Respond = 0x36,
  ActBtn_Infiltrate = 0x37,
  ActBtn_FanKick = 0x38,
  ActBtn_BackKick = 0x39,
  ActBtn_Knee = 0x3A,
  ActBtn_NeckBreaker = 0x3B,
  ActBtn_ThrustPunch = 0x3C,
  ActBtn_ChikyoChagi = 0x3D,
  ActBtn_Jump3 = 0x3E,
  ActBtn_Ring = 0x3F,
  ActBtn_HopDown = 0x40,
  ActBtn_Fire = 0x41,
  ActBtn_Rotate2 = 0x42,
  ActBtn_HookShot = 0x43,
  ActBtn_aaaStart = 0x44,
  ActBtn_ActBtnLast = 0x50,
  ActBtn_ActBtnCount = 0x51,
  ActBtn_MessageStart = 0x80,
  ActBtn_Dummy2 = 0x88,
  ActBtn_Zoom = 0x95,
  ActBtn_Look2 = 0xA3,
  ActBtn_MessageEnd = 0xB6,
};

struct EvtActBtnCtrl
{
  int Flag_0;
  int Count_4;
  ACTION_TYPE32_mb ActType_8;
};
assert_size(EvtActBtnCtrl, 0xC);

class Event : cUnit
{
public:
  int8_t EndRNo0_C;
  int8_t EndRNo1_D;
  int8_t EndRNo2_E;
  int8_t EndRNo3_F;
  uint8_t Id_10;
  uint8_t Type_11;
  uint8_t pad_12[2];
  uint32_t NoWork_14;
  char Name_18[32];
  EvdHead* PEvd_38;
  EvpHead* PPac_3C;
  EvpHead* PPacOld_40;
  EventFlag_mb StatusFlag_44;
  DatTbl ModTbl_48;
  Mtx MatCamOya_50;
  cModel* PPl_80;
  cModel* PModOya_84;
  cModel** PModList_88;
  void** PFuncTbl_8C;
  float NowTotalFrame_90;
  uint32_t MaxTotalFrame_94;
  float NowFrame_98;
  uint32_t MaxFrame_9C;
  int NowCut_A0;
  int MaxCut_A4;
  int BakNowTotalFrame_A8;
  int BakMaxTotalFrame_AC;
  int BakNowFrame_B0;
  int BakMaxFrame_B4;
  int BakNowCut_B8;
  int BakMaxCut_BC;
  int NowStr_C0[2];
  int SndId_C8[2];
  int EvtCancelCut_D0;
  float TimerMes_D4;
  int NoEvt_D8;
  int NoLit_DC;
  int FFNowFrame_E0;
  EvtActBtnCtrl EvtActBtn_E4;
  uint32_t FuncType_F0;
  int EmListNo_F4;
  uint32_t pDatFog_F8;
  uint32_t pDatFocus_FC;
  int MesNoOld_100;
  float DelTimer_104;
  int ChangeNoStr_108;
  int ChangeNowCut_10C;
};
assert_size(Event, 0x110);

struct ReadWork
{
  uint8_t EmId_0;
  uint8_t SwapFlag_1;
  uint8_t dummy_2[2];
};
assert_size(ReadWork, 4);

class EventMgr;

enum AliveEvtType : int
{
  AliveEvtTypeNormal = 0x0,
  AliveEvtTypeEndSleepNoSkip = 0x1,
};

typedef bool(__fastcall* EventMgr__IsAliveEvt_Fn)(EventMgr* thisptr, void* unused, char* pName, Event** ppEvt, AliveEvtType aliveEvtType);
extern EventMgr__IsAliveEvt_Fn EventMgr__IsAliveEvt;

class EventMgr : public cManager<Event>
{
public:
  char NowExeEvtName_1C[48];
  ReadWork ReadWkTbl_4C[8];
  char NameTmp_6C[32];
  class cDataUnit* pUnit_8C[32];
  uint8_t EmIdTbl[32];
  uint32_t EmWindowFcvTbl_12C[3];
  DatTbl EvdTbl_138;
  DatTbl BinTbl_140;
  DatTbl FuncTbl_148;
  DatTbl ReadTbl_150;

  inline bool IsAliveEvt(char* pName, Event** ppEvt, AliveEvtType aliveEvtType)
  {
    return EventMgr__IsAliveEvt(this, nullptr, pName, ppEvt, aliveEvtType);
  }
};
assert_size(EventMgr, 0x158);

extern EventMgr* EvtMgr;
