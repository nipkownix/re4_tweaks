#include <unordered_map>
#include <string>
#include "GameFlags.h"

const char* Flags_DEBUG_Names[] = {
	"DBG_TEST_MODE", // 0x80000000 (@ 0x0)
	"DBG_SCR_TEST", // 0x40000000 (@ 0x0)
	"DBG_BACK_CLIP", // 0x20000000 (@ 0x0)
	"DBG_DBG_CAM", // 0x10000000 (@ 0x0)
	"DBG_SAT_DISP", // 0x8000000 (@ 0x0)
	"DBG_EAT_DISP", // 0x4000000 (@ 0x0)
	"DBG_EVENT_TOOL", // 0x2000000 (@ 0x0)
	"DBG_SLOW_ON", // 0x1000000 (@ 0x0)
	"DBG_SHADOW_POLYGON", // 0x800000 (@ 0x0)
	"DBG_SCE_AT_DISP", // 0x400000 (@ 0x0)
	"DBG_SCR2_TEST", // 0x200000 (@ 0x0)
	"DBG_SHADOW_FRAME", // 0x100000 (@ 0x0)
	"DBG_MIRROR_POLYGON", // 0x80000 (@ 0x0)
	"DBG_GROUND_DISP", // 0x40000 (@ 0x0)
	"DBG_SKELETON_DISP", // 0x20000 (@ 0x0)
	"DBG_ESPTOOL_ONSCR", // 0x10000 (@ 0x0)
	"DBG_CINESCO_OFF", // 0x8000 (@ 0x0)
	"DBG_RTP_DISP", // 0x4000 (@ 0x0)
	"DBG_ROOM_WIRE_DISP", // 0x2000 (@ 0x0)
	"DBG_EM_YARARE_DISP", // 0x1000 (@ 0x0)
	"DBG_CAM_AREA_OFF", // 0x800 (@ 0x0)
	"DBG_CLOTH_AT_DISP", // 0x400 (@ 0x0)
	"DBG_WIND_ON", // 0x200 (@ 0x0)
	"DBG_ESPTOOL_MEM_USE", // 0x100 (@ 0x0)
	"DBG_TEX_RENDER_ALL", // 0x80 (@ 0x0)
	"DBG_ESPTOOL_ONEM", // 0x40 (@ 0x0)
	"DBG_EMINFO_DISP", // 0x20 (@ 0x0)
	"DBG_LIGHT_TOOL", // 0x10 (@ 0x0)
	"DBG_EM_COUNT", // 0x8 (@ 0x0)
	"DBG_1D", // 0x4 (@ 0x0)
	"DBG_EM3F_ADA_SADLER", // 0x2 (@ 0x0)
	"DBG_CAM_ANTIGRO_OFF", // 0x1 (@ 0x0)
	"DBG_COCKPIT_TOOL", // 0x80000000 (@ 0x4)
	"DBG_BOUNDING_DISP", // 0x40000000 (@ 0x4)
	"DBG_ADJUST_CAM", // 0x20000000 (@ 0x4)
	"DBG_FLAT_FLOOR", // 0x10000000 (@ 0x4)
	"DBG_OBJ_SKELETON_", // 0x8000000 (@ 0x4)
	"DBG_DRAW_SH_TEX", // 0x4000000 (@ 0x4)
	"DBG_EM_NO_ATK", // 0x2000000 (@ 0x4)
	"DBG_NO_EST_CALL", // 0x1000000 (@ 0x4)
	"DBG_IN_ESP_TOOL", // 0x800000 (@ 0x4)
	"DBG_TERM_TOOL", // 0x400000 (@ 0x4)
	"DBG_WARN_LEVEL_LOW", // 0x200000 (@ 0x4)
	"DBG_ID_TOOL", // 0x100000 (@ 0x4)
	"DBG_TITLE_CHECK", // 0x80000 (@ 0x4)
	"DBG_CAST_ERR_NO_DISP", // 0x40000 (@ 0x4)
	"DBG_EMW_ERR_NO_DISP", // 0x20000 (@ 0x4)
	"DBG_TIMER_STOP", // 0x10000 (@ 0x4)
	"DBG_SCREEN_SHOT", // 0x8000 (@ 0x4)
	"DBG_SERIES_SHOT", // 0x4000 (@ 0x4)
	"DBG_ASHLEY_NO_CATCH", // 0x2000 (@ 0x4)
	"DBG_RENSYA", // 0x1000 (@ 0x4)
	"DBG_CERO", // 0x800 (@ 0x4)
	"DBG_35", // 0x400 (@ 0x4)
	"DBG_36", // 0x200 (@ 0x4)
	"DBG_37", // 0x100 (@ 0x4)
	"DBG_Wii_PAD", // 0x80 (@ 0x4)
	"DBG_39", // 0x40 (@ 0x4)
	"DBG_3A", // 0x20 (@ 0x4)
	"DBG_3B", // 0x10 (@ 0x4)
	"DBG_3C", // 0x8 (@ 0x4)
	"DBG_3D", // 0x4 (@ 0x4)
	"DBG_3E", // 0x2 (@ 0x4)
	"DBG_3F", // 0x1 (@ 0x4)
	"DBG_ROOMJMP", // 0x80000000 (@ 0x8)
	"DBG_PROC_BAR", // 0x40000000 (@ 0x8)
	"DBG_SCA_VIEW", // 0x20000000 (@ 0x8)
	"DBG_OBA_VIEW", // 0x10000000 (@ 0x8)
	"DBG_SLOW_MODE", // 0x8000000 (@ 0x8)
	"DBG_NO_SCE_EXE", // 0x4000000 (@ 0x8)
	"DBG_SINGLE_DISK", // 0x2000000 (@ 0x8)
	"DBG_BUGCHECK_MODE", // 0x1000000 (@ 0x8)
	"DBG_NO_DEATH", // 0x800000 (@ 0x8)
	"DBG_INF_BULLET", // 0x400000 (@ 0x8)
	"DBG_NO_ENEMY", // 0x200000 (@ 0x8)
	"DBG_BGM_STOP", // 0x100000 (@ 0x8)
	"DBG_SE_STOP", // 0x80000 (@ 0x8)
	"DBG_PL_LOCK_FOLLOW", // 0x40000 (@ 0x8)
	"DBG_EM_NO_DEATH", // 0x20000 (@ 0x8)
	"DBG_KAIOUKEN", // 0x10000 (@ 0x8)
	"DBG_PAD_INFO", // 0x8000 (@ 0x8)
	"DBG_UNDER_CONST", // 0x4000 (@ 0x8)
	"DBG_EM_WEAK", // 0x2000 (@ 0x8)
	"DBG_EM_LIFE_DISP", // 0x1000 (@ 0x8)
	"DBG_SHADOW_LIGHT", // 0x800 (@ 0x8)
	"DBG_CAPTION_OFF", // 0x400 (@ 0x8)
	"DBG_TEST_MODE_CK", // 0x200 (@ 0x8)
	"DBG_LIGHT_ERR_CHECK", // 0x100 (@ 0x8)
	"DBG_EST_CALL_CHK", // 0x80 (@ 0x8)
	"DBG_GX_WARN_ALL", // 0x40 (@ 0x8)
	"DBG_GX_WARN_MIDIUM", // 0x20 (@ 0x8)
	"DBG_GX_WARN_SEVERE", // 0x10 (@ 0x8)
	"DBG_PL_NOHIT", // 0x8 (@ 0x8)
	"DBG_SE_ERR_ALL", // 0x4 (@ 0x8)
	"DBG_QUICK_EM_STOP", // 0x2 (@ 0x8)
	"DBG_AV_TEST", // 0x1 (@ 0x8)
	"DBG_INF_BULLET2", // 0x80000000 (@ 0xc)
	"DBG_BATTLE_CAM", // 0x40000000 (@ 0xc)
	"DBG_PL_KLAUSER", // 0x20000000 (@ 0xc)
	"DBG_MAP_ZOOM_ORG", // 0x10000000 (@ 0xc)
	"DBG_SCISSOR_OFF", // 0x8000000 (@ 0xc)
	"DBG_LOG_OFF", // 0x4000000 (@ 0xc)
	"DBG_SCR_CHECK", // 0x2000000 (@ 0xc)
	"DBG_OBJ_SERVER", // 0x1000000 (@ 0xc)
	"DBG_START_ST2", // 0x800000 (@ 0xc)
	"DBG_ERRORL_CK", // 0x400000 (@ 0xc)
	"DBG_APP_USE_DBMEM", // 0x200000 (@ 0xc)
	"DBG_REFRACT_CK", // 0x100000 (@ 0xc)
	"DBG_EM_NO_DIE_FLAG", // 0x80000 (@ 0xc)
	"DBG_START_ST3", // 0x40000 (@ 0xc)
	"DBG_START_LAST", // 0x20000 (@ 0xc)
	"DBG_DOOR_SET_MODE", // 0x10000 (@ 0xc)
	"DBG_EFF_NUM_DISP", // 0x8000 (@ 0xc)
	"DBG_SET_HITMARK_ALL", // 0x4000 (@ 0xc)
	"DBG_FOG_FAR_GREEN", // 0x2000 (@ 0xc)
	"DBG_SSCRN_PROC_BAR", // 0x1000 (@ 0xc)
	"DBG_NO_ETC_SET", // 0x800 (@ 0xc)
	"DBG_NO_DEATH2", // 0x400 (@ 0xc)
	"DBG_NO_PARASITE", // 0x200 (@ 0xc)
	"DBG_ESP_CHK", // 0x100 (@ 0xc)
	"DBG_NO_EVENT", // 0x80 (@ 0xc)
	"DBG_NO_LASER_LINE", // 0x40 (@ 0xc)
	"DBG_DATA_113", // 0x20 (@ 0xc)
	"DBG_SHOP_FULL", // 0x10 (@ 0xc)
	"DBG_ADA_OMAKE_EV", // 0x8 (@ 0xc)
	"DBG_FOG_FAR_COLOR", // 0x4 (@ 0xc)
	"DBG_TOOL_EMINFO_DISP", // 0x2 (@ 0xc)
	"DBG_CHAR_MACHINE", // 0x1 (@ 0xc)
};

const char* Flags_STOP_Names[] = {
	"SPF_KEY", // 0x80000000 (@ 0x0)
	"SPF_CAMERA", // 0x40000000 (@ 0x0)
	"SPF_EM", // 0x20000000 (@ 0x0)
	"SPF_PL", // 0x10000000 (@ 0x0)
	"SPF_ESP", // 0x8000000 (@ 0x0)
	"SPF_OBJ", // 0x4000000 (@ 0x0)
	"SPF_CTRL", // 0x2000000 (@ 0x0)
	"SPF_LIGHT", // 0x1000000 (@ 0x0)
	"SPF_SCE", // 0x800000 (@ 0x0)
	"SPF_SCE_AT", // 0x400000 (@ 0x0)
	"SPF_CCHG", // 0x200000 (@ 0x0)
	"SPF_PL_CCHG", // 0x100000 (@ 0x0)
	"SPF_NOTSUBSCR", // 0x80000 (@ 0x0)
	"SPF_WATER", // 0x40000 (@ 0x0)
	"SPF_SPECULAR", // 0x20000 (@ 0x0)
	"SPF_EARTHQUAKE", // 0x10000 (@ 0x0)
	"SPF_VIBRATION", // 0x8000 (@ 0x0)
	"SPF_CINESCO", // 0x4000 (@ 0x0)
	"SPF_MIST", // 0x2000 (@ 0x0)
	"SPF_SUBCHAR", // 0x1000 (@ 0x0)
	"SPF_SE", // 0x800 (@ 0x0)
	"SPF_EVT", // 0x400 (@ 0x0)
	"SPF_BLOCK", // 0x200 (@ 0x0)
	"SPF_ACTBTN", // 0x100 (@ 0x0)
	"SPF_DATAREAD_AT", // 0x80 (@ 0x0)
	"SPF_ID_SYSTEM", // 0x40 (@ 0x0)
	"SPF_ESP_AREA", // 0x20 (@ 0x0)
	"SPF_1B",
	"SPF_1C",
	"SPF_1D",
	"SPF_1E",
	"SPF_1F"
};

const char* Flags_STATUS_Names[] = {
	"STA_BG_OFF", // 0x80000000 (@ 0x0)
	"STA_PL_CHECK", // 0x40000000 (@ 0x0)
	"STA_PL_CHECK2", // 0x20000000 (@ 0x0)
	"STA_MOVIE_ON", // 0x10000000 (@ 0x0)
	"STA_CUTCHG", // 0x8000000 (@ 0x0)
	"STA_MOVIE2_ON", // 0x4000000 (@ 0x0)
	"STA_SSCRN_ENABLE", // 0x2000000 (@ 0x0)
	"STA_CINESCO", // 0x1000000 (@ 0x0)
	"STA_PL_FIRE", // 0x800000 (@ 0x0)
	"STA_09", // 0x400000 (@ 0x0)
	"STA_ACT_DONT_FIRE", // 0x200000 (@ 0x0)
	"STA_DIEDEMO", // 0x100000 (@ 0x0)
	"STA_BLUR", // 0x80000 (@ 0x0)
	"STA_SUB_SCRN", // 0x40000 (@ 0x0)
	"STA_SHOOTING", // 0x20000 (@ 0x0)
	"STA_PAD_SENSITIVE", // 0x10000 (@ 0x0)
	"STA_LOOK_THROUGH", // 0x8000 (@ 0x0)
	"STA_PL_ACTION", // 0x4000 (@ 0x0)
	"STA_PL_INVISIBLE", // 0x2000 (@ 0x0)
	"STA_EVENT", // 0x1000 (@ 0x0)
	"STA_ASHLEY_HIDE", // 0x800 (@ 0x0)
	"STA_CAM_SHOULDER", // 0x400 (@ 0x0)
	"STA_WATER_ALIVE", // 0x200 (@ 0x0)
	"STA_CAMERA", // 0x100 (@ 0x0)
	"STA_BLACKOUT", // 0x80 (@ 0x0)
	"STA_SCOPE_CAMERA", // 0x40 (@ 0x0)
	"STA_RIDE_GONDOLA", // 0x20 (@ 0x0)
	"STA_WIND_ON", // 0x10 (@ 0x0)
	"STA_PL_JUMP_OFF", // 0x8 (@ 0x0)
	"STA_MIRROR", // 0x4 (@ 0x0)
	"STA_SAND_ALIVE", // 0x2 (@ 0x0)
	"STA_SELF_SHADOW", // 0x1 (@ 0x0)
	"STA_PL_SE_FOOT", // 0x80000000 (@ 0x4)
	"STA_PL_SE_WHISTLE", // 0x40000000 (@ 0x4)
	"STA_SE_BURST", // 0x20000000 (@ 0x4)
	"STA_SUSPEND", // 0x10000000 (@ 0x4)
	"STA_TEX_RENDER", // 0x8000000 (@ 0x4)
	"STA_THERMO_GRAPH", // 0x4000000 (@ 0x4)
	"STA_CAMERA_IN_ROOM", // 0x2000000 (@ 0x4)
	"STA_NO_LIGHTMASK", // 0x1000000 (@ 0x4)
	"STA_PL_SPEAR_SET", // 0x800000 (@ 0x4)
	"STA_PL_SWIM", // 0x400000 (@ 0x4)
	"STA_PL_BOAT", // 0x200000 (@ 0x4)
	"STA_WATER_CAMERA", // 0x100000 (@ 0x4)
	"STA_PL_SWIM_CAMERA", // 0x80000 (@ 0x4)
	"STA_PL_LADDER", // 0x40000 (@ 0x4)
	"STA_CRITICAL", // 0x20000 (@ 0x4)
	"STA_TAKEAWAY", // 0x10000 (@ 0x4)
	"STA_PL_CATCHED", // 0x8000 (@ 0x4)
	"STA_SHADOW_EQCOL", // 0x4000 (@ 0x4)
	"STA_PL_CATCHHOLD", // 0x2000 (@ 0x4)
	"STA_NEARCLIP_TOUCH", // 0x1000 (@ 0x4)
	"STA_CAMERA_SET_ROOM", // 0x800 (@ 0x4)
	"STA_ROOM_RAIN", // 0x400 (@ 0x4)
	"STA_USE_CAST_SHADOW", // 0x200 (@ 0x4)
	"STA_PROC_SHD_TEX", // 0x100 (@ 0x4)
	"STA_ALPHA_DRAW2", // 0x80 (@ 0x4)
	"STA_SET_BG_COLOR", // 0x40 (@ 0x4)
	"STA_ESPGEN45_SET", // 0x20 (@ 0x4)
	"STA_EFFEM2D_TEXRND", // 0x10 (@ 0x4)
	"STA_SUB_LADDER", // 0x8 (@ 0x4)
	"STA_SUBCHAR_CTRL", // 0x4 (@ 0x4)
	"STA_ITEM_GET", // 0x2 (@ 0x4)
	"STA_LASERSITE_NOADD", // 0x1 (@ 0x4)
	"STA_PL_DONT_FIRE", // 0x80000000 (@ 0x8)
	"STA_PL_EM_ACTION", // 0x40000000 (@ 0x8)
	"STA_SUB_CATCHED", // 0x20000000 (@ 0x8)
	"STA_CUT_CHANGE", // 0x10000000 (@ 0x8)
	"STA_NO_FENCE", // 0x8000000 (@ 0x8)
	"STA_SSCRN_REQUEST", // 0x4000000 (@ 0x8)
	"STA_ESP_COMPULSION_NOSUSPEND", // 0x2000000 (@ 0x8)
	"STA_PL_MISS_SHOT", // 0x1000000 (@ 0x8)
	"STA_SUB_BULLDOZER", // 0x800000 (@ 0x8)
	"STA_LIT_NO_UPDATE", // 0x400000 (@ 0x8)
	"STA_MAP_DISABLE", // 0x200000 (@ 0x8)
	"STA_USE_SHADOW_LIGHT", // 0x100000 (@ 0x8)
	"STA_EVENT_SYSYTEM", // 0x80000 (@ 0x8)
	"STA_INTO_SHOP", // 0x40000 (@ 0x8)
	"STA_TIMER_NO_PAUSE", // 0x20000 (@ 0x8)
	"STA_EFFAREA_USE_CAM", // 0x10000 (@ 0x8)
	"STA_TITLE", // 0x8000 (@ 0x8)
	"STA_MAP_CK_UNLIMIT", // 0x4000 (@ 0x8)
	"STA_RIFLE_READY", // 0x2000 (@ 0x8)
	"STA_LOWPOLY_USE", // 0x1000 (@ 0x8)
	"STA_TERMINAL", // 0x800 (@ 0x8)
	"STA_HIGH_TEMPERATURE_THERMO", // 0x400 (@ 0x8)
	"STA_56", // 0x200 (@ 0x8)
	"STA_57", // 0x100 (@ 0x8)
	"STA_58", // 0x80 (@ 0x8)
	"STA_59", // 0x40 (@ 0x8)
	"STA_5A", // 0x20 (@ 0x8)
	"STA_5B", // 0x10 (@ 0x8)
	"STA_5C", // 0x8 (@ 0x8)
	"STA_5D", // 0x4 (@ 0x8)
	"STA_5E", // 0x2 (@ 0x8)
	"STA_5F", // 0x1 (@ 0x8)
	"STA_SAVEDATA_NO_UPDATE", // 0x80000000 (@ 0xc)
	"STA_BEHIND_CAM", // 0x40000000 (@ 0xc)
	"STA_62", // 0x20000000 (@ 0xc)
	"STA_SCISSOR", // 0x10000000 (@ 0xc)
	"STA_SLOW", // 0x8000000 (@ 0xc)
	"STA_SUB_ASHLEY", // 0x4000000 (@ 0xc)
	"STA_BIG_MARKER", // 0x2000000 (@ 0xc)
	"STA_EVENT_CANCEL", // 0x1000000 (@ 0xc)
	"STA_KLAUSER_TRANSFORM", // 0x800000 (@ 0xc)
	"STA_ELEVATOR", // 0x400000 (@ 0xc)
	"STA_6A", // 0x200000 (@ 0xc)
	"STA_6B", // 0x100000 (@ 0xc)
	"STA_6C", // 0x80000 (@ 0xc)
	"STA_6D", // 0x40000 (@ 0xc)
	"STA_6E", // 0x20000 (@ 0xc)
	"STA_6F", // 0x10000 (@ 0xc)
	"STA_DIV_EVENT_SET", // 0x8000 (@ 0xc)
	"STA_DIV_EVENT_ING", // 0x4000 (@ 0xc)
	"STA_NOW_LOADING", // 0x2000 (@ 0xc)
	"STA_SCREEN_STOP_RESERVE", // 0x1000 (@ 0xc)
	"STA_TYPEWRITER", // 0x800 (@ 0xc)
	"STA_75", // 0x400 (@ 0xc)
	"STA_76", // 0x200 (@ 0xc)
	"STA_77", // 0x100 (@ 0xc)
	"STA_78", // 0x80 (@ 0xc)
	"STA_79", // 0x40 (@ 0xc)
	"STA_7A", // 0x20 (@ 0xc)
	"STA_7B", // 0x10 (@ 0xc)
	"STA_7C", // 0x8 (@ 0xc)
	"STA_7D", // 0x4 (@ 0xc)
	"STA_7E", // 0x2 (@ 0xc)
	"STA_7F", // 0x1 (@ 0xc)
};

const char* Flags_SYSTEM_Names[] = {
	"SYS_OMAKE_ADA_GAME", // 0x80000000 (@ 0x0)
	"SYS_OMAKE_ETC_GAME", // 0x40000000 (@ 0x0)
	"SYS_EXCEPTION", // 0x20000000 (@ 0x0)
	"SYS_RENDER_END", // 0x10000000 (@ 0x0)
	"SYS_SP_USED", // 0x8000000 (@ 0x0)
	"SYS_SOFT_RESET", // 0x4000000 (@ 0x0)
	"SYS_DATA_READ", // 0x2000000 (@ 0x0)
	"SYS_ROOMJUMP", // 0x1000000 (@ 0x0)
	"SYS_INVISIBLE", // 0x800000 (@ 0x0)
	"SYS_DOOR_AFTER", // 0x400000 (@ 0x0)
	"SYS_DOORDEMO", // 0x200000 (@ 0x0)
	"SYS_TRANS_STOP", // 0x100000 (@ 0x0)
	"SYS_CONTINUE", // 0x80000 (@ 0x0)
	"SYS_SET_BLACK", // 0x40000 (@ 0x0)
	"SYS_SN_PC_READ", // 0x20000 (@ 0x0)
	"SYS_SN_PC_READ_TOOL", // 0x10000 (@ 0x0)
	"SYS_SSCRN_EXEC", // 0x8000 (@ 0x0)
	"SYS_SCREEN_SHOT", // 0x4000 (@ 0x0)
	"SYS_NEW_GAME", // 0x2000 (@ 0x0)
	"SYS_PS2_ADA_GAME", // 0x1000 (@ 0x0)
	"SYS_SCISSOR_ON", // 0x800 (@ 0x0)
	"SYS_SCREEN_STOP", // 0x400 (@ 0x0)
	"SYS_CARD_ACCESS", // 0x200 (@ 0x0)
	"SYS_LOAD_GAME", // 0x100 (@ 0x0)
	"SYS_CONTINUE_AFTER", // 0x80 (@ 0x0)
	"SYS_START_EVT_SKIP", // 0x40 (@ 0x0)
	"SYS_HARD_MODE", // 0x20 (@ 0x0)
	"SYS_MESSAGE_INIT", // 0x10 (@ 0x0)
	"SYS_PUBLICITY_VER", // 0x8 (@ 0x0)
	"SYS_SAVEDATA_EXIST", // 0x4 (@ 0x0)
	"SYS_PAL", // 0x2 (@ 0x0)
	"SYS_DTV480P", // 0x1 (@ 0x0)
};

const char* Flags_ITEM_SET_Names[] = {
	"ITF_DUMMY", // = 0 0x80000000 (@ 0x0)
	"ITF_R108_ITEM", // = 1 0x40000000 (@ 0x0)
	"ITF_R105_ITEM", // = 2 0x20000000 (@ 0x0)
	"ITF_FN57", // = 3 0x10000000 (@ 0x0)
	"ITF_R10A_ITEM06", // = 4 0x8000000 (@ 0x0)
	"ITF_R102_ITEM", // = 5 0x4000000 (@ 0x0)
	"ITF_R103_ITEM00", // = 6 0x2000000 (@ 0x0)
	"ITF_R103_ITEM01", // = 7 0x1000000 (@ 0x0)
	"ITF_R10B_ITEM", // = 8 0x800000 (@ 0x0)
	"ITF_R11C_ITEM", // = 9 0x400000 (@ 0x0)
	"ITF_R216_ITEM", // = 10 0x200000 (@ 0x0)
	"ITF_R207_GOLDEN_SWORD", // = 11 0x100000 (@ 0x0)
	"ITF_R207_SILVER_SWORD", // = 12 0x80000 (@ 0x0)
	"ITF_ITEM_5F", // = 13 0x40000 (@ 0x0)
	"ITF_ITEM_59", // = 14 0x20000 (@ 0x0)
	"ITF_ITEM_5D", // = 15 0x10000 (@ 0x0)
	"ITF_ITEM_61", // = 16 0x8000 (@ 0x0)
	"ITF_R20E_SALAZAR_CREST", // = 17 0x4000 (@ 0x0)
	"ITF_R209_KEY", // = 18 0x2000 (@ 0x0)
	"ITF_R21A_ITEM", // = 19 0x1000 (@ 0x0)
	"ITF_R103_FILE", // = 20 0x800 (@ 0x0)
	"ITF_R11D_ITEM", // = 21 0x400 (@ 0x0)
	"ITF_R11E_ITEM", // = 22 0x200 (@ 0x0)
	"ITF_R117_ITEM", // = 23 0x100 (@ 0x0)
	"ITF_R10D_ITEM", // = 24 0x80 (@ 0x0)
	"ITF_R309_KEY", // = 25 0x40 (@ 0x0)
	"ITF_R308_THERMO_RIFLE", // = 26 0x20 (@ 0x0)
	"ITF_R40E_SAMPLE00", // = 27 0x10 (@ 0x0)
	"ITF_R40B_SAMPLE00", // = 28 0x8 (@ 0x0)
	"ITF_R40C_SAMPLE00", // = 29 0x4 (@ 0x0)
	"ITF_R31C_CREST_A", // = 30 0x2 (@ 0x0)
	"ITF_FILE_07", // = 31 0x1 (@ 0x0)
	"ITF_FILE_08", // = 32 0x80000000 (@ 0x4)
	"ITF_R410_SAMPLE00", // = 33 0x40000000 (@ 0x4)
	"ITF_22", // = 34 0x20000000 (@ 0x4)
	"ITF_R40D_SAMPLE00", // = 35 0x10000000 (@ 0x4)
	"ITF_R332_ADA_ROCKET", // = 36 0x8000000 (@ 0x4)
	"ITF_ST2A_00", // = 37 0x4000000 (@ 0x4)
	"ITF_ST2A_01", // = 38 0x2000000 (@ 0x4)
	"ITF_ST2A_02", // = 39 0x1000000 (@ 0x4)
	"ITF_ST2A_03", // = 40 0x800000 (@ 0x4)
	"ITF_ST2A_04", // = 41 0x400000 (@ 0x4)
	"ITF_ST2A_05", // = 42 0x200000 (@ 0x4)
	"ITF_ST2A_06", // = 43 0x100000 (@ 0x4)
	"ITF_ST2A_07", // = 44 0x80000 (@ 0x4)
	"ITF_ST2A_08", // = 45 0x40000 (@ 0x4)
	"ITF_ST2A_09", // = 46 0x20000 (@ 0x4)
	"ITF_ST2A_10", // = 47 0x10000 (@ 0x4)
	"ITF_ST2A_11", // = 48 0x8000 (@ 0x4)
	"ITF_ST2B_00", // = 49 0x4000 (@ 0x4)
	"ITF_ST2B_01", // = 50 0x2000 (@ 0x4)
	"ITF_ST2B_02", // = 51 0x1000 (@ 0x4)
	"ITF_ST2C_00", // = 52 0x800 (@ 0x4)
	"ITF_ST3B_00", // = 53 0x400 (@ 0x4)
	"ITF_ST3B_01", // = 54 0x200 (@ 0x4)
	"ITF_ST3B_02", // = 55 0x100 (@ 0x4)
	"ITF_ST3D_00", // = 56 0x80 (@ 0x4)
	"ITF_ST3D_01", // = 57 0x40 (@ 0x4)
	"ITF_ST3D_02", // = 58 0x20 (@ 0x4)
	"ITF_ST3D_03", // = 59 0x10 (@ 0x4)
	"ITF_R400_00", // = 60 0x8 (@ 0x4)
	"ITF_R400_01", // = 61 0x4 (@ 0x4)
	"ITF_R400_02", // = 62 0x2 (@ 0x4)
	"ITF_R400_03", // = 63 0x1 (@ 0x4)
	"ITF_R400_04", // = 64 0x80000000 (@ 0x8)
	"ITF_R402_00", // = 65 0x40000000 (@ 0x8)
	"ITF_R402_01", // = 66 0x20000000 (@ 0x8)
	"ITF_R402_02", // = 67 0x10000000 (@ 0x8)
	"ITF_R402_03", // = 68 0x8000000 (@ 0x8)
	"ITF_R402_04", // = 69 0x4000000 (@ 0x8)
	"ITF_R402_05", // = 70 0x2000000 (@ 0x8)
	"ITF_R403_00", // = 71 0x1000000 (@ 0x8)
	"ITF_R403_01", // = 72 0x800000 (@ 0x8)
	"ITF_R403_02", // = 73 0x400000 (@ 0x8)
	"ITF_R403_03", // = 74 0x200000 (@ 0x8)
	"ITF_R403_04", // = 75 0x100000 (@ 0x8)
	"ITF_R403_05", // = 76 0x80000 (@ 0x8)
	"ITF_R403_06", // = 77 0x40000 (@ 0x8)
	"ITF_R403_07", // = 78 0x20000 (@ 0x8)
	"ITF_R403_08", // = 79 0x10000 (@ 0x8)
	"ITF_R403_09", // = 80 0x8000 (@ 0x8)
	"ITF_R403_10", // = 81 0x4000 (@ 0x8)
	"ITF_R403_11", // = 82 0x2000 (@ 0x8)
	"ITF_R404_00", // = 83 0x1000 (@ 0x8)
	"ITF_R404_01", // = 84 0x800 (@ 0x8)
	"ITF_R404_02", // = 85 0x400 (@ 0x8)
	"ITF_R404_03", // = 86 0x200 (@ 0x8)
	"ITF_R404_04", // = 87 0x100 (@ 0x8)
	"ITF_R404_05", // = 88 0x80 (@ 0x8)
	"ITF_R404_06", // = 89 0x40 (@ 0x8)
	"ITF_R404_07", // = 90 0x20 (@ 0x8)
	"ITF_R404_08", // = 91 0x10 (@ 0x8)
	"ITF_R404_09", // = 92 0x8 (@ 0x8)
	"ITF_R404_10", // = 93 0x4 (@ 0x8)
	"ITF_R404_11", // = 94 0x2 (@ 0x8)
	"ITF_ST2A_12", // = 95 0x1 (@ 0x8)
	"ITF_R119_ITEM0", // = 96 0x80000000 (@ 0xC)
	"ITF_R119_ITEM1", // = 97 0x40000000 (@ 0xC)
	"ITF_R119_ITEM2", // = 98 0x20000000 (@ 0xC)
	"ITF_R119_ITEM3", // = 99 0x10000000 (@ 0xC)
	"ITF_R119_ITEM4", // = 100 0x8000000 (@ 0xC)
	"ITF_R119_ITEM5", // = 101 0x4000000 (@ 0xC)
	"ITF_R11E_ITEM0", // = 102 0x2000000 (@ 0xC)
	"ITF_R11E_ITEM1", // = 103 0x1000000 (@ 0xC)
	"ITF_R11E_ITEM2", // = 104 0x800000 (@ 0xC)
	"ITF_R11E_ITEM3", // = 105 0x400000 (@ 0xC)
	"ITF_R11E_ITEM4", // = 106 0x200000 (@ 0xC)
	"ITF_R11E_ITEM5", // = 107 0x100000 (@ 0xC)
	"ITF_R11E_ITEM6", // = 108 0x80000 (@ 0xC)
	"ITF_R11E_ITEM7", // = 109 0x40000 (@ 0xC)
	"ITF_R11E_ITEM8", // = 110 0x20000 (@ 0xC)
	"ITF_R11E_ITEM9", // = 111 0x10000 (@ 0xC)
	"ITF_R11E_ITEM10", // = 112 0x8000 (@ 0xC)
	"ITF_R11E_ITEM11", // = 113 0x4000 (@ 0xC)
	"ITF_R502_KEY", // = 114 0x2000 (@ 0xC)
	"ITF_R502_EMB", // = 115 0x1000 (@ 0xC)
	"ITF_R502_KEY_DUMMY", // = 116 0x800 (@ 0xC)
	"ITF_R502_EMB2", // = 117 0x400 (@ 0xC)
	"ITF_R50E_ITEM0", // = 118 0x200 (@ 0xC)
	"ITF_R50E_ITEM1", // = 119 0x100 (@ 0xC)
	"ITF_R50E_ITEM2", // = 120 0x80 (@ 0xC)
	"ITF_R50E_ITEM3", // = 121 0x40 (@ 0xC)
	"ITF_R508_STEEL_KEY", // = 122 0x20 (@ 0xC)
	"ITF_ADA_01_TRES_00", // = 123 0x10 (@ 0xC)
	"ITF_ADA_01_TRES_01", // = 124 0x8 (@ 0xC)
	"ITF_ADA_01_TRES_02", // = 125 0x4 (@ 0xC)
	"ITF_ADA_01_TRES_03", // = 126 0x2 (@ 0xC)
	"ITF_ADA_01_TRES_04", // = 127 0x1 (@ 0xC)
	"ITF_ADA_02_TRES_00", // = 128 0x80000000 (@ 0x10)
	"ITF_ADA_02_TRES_01", // = 129 0x40000000 (@ 0x10)
	"ITF_ADA_02_TRES_02", // = 130 0x20000000 (@ 0x10)
	"ITF_ADA_02_TRES_03", // = 131 0x10000000 (@ 0x10)
	"ITF_ADA_02_TRES_04", // = 132 0x8000000 (@ 0x10)
	"ITF_ADA_03_TRES_00", // = 133 0x4000000 (@ 0x10)
	"ITF_ADA_03_TRES_01", // = 134 0x2000000 (@ 0x10)
	"ITF_ADA_03_TRES_02", // = 135 0x1000000 (@ 0x10)
	"ITF_ADA_03_TRES_03", // = 136 0x800000 (@ 0x10)
	"ITF_ADA_03_TRES_04", // = 137 0x400000 (@ 0x10)
	"ITF_ADA_04_TRES_00", // = 138 0x200000 (@ 0x10)
	"ITF_ADA_04_TRES_01", // = 139 0x100000 (@ 0x10)
	"ITF_ADA_04_TRES_02", // = 140 0x80000 (@ 0x10)
	"ITF_ADA_04_TRES_03", // = 141 0x40000 (@ 0x10)
	"ITF_ADA_04_TRES_04", // = 142 0x20000 (@ 0x10)
	"ITF_ADA_05_TRES_00", // = 143 0x10000 (@ 0x10)
	"ITF_ADA_05_TRES_01", // = 144 0x8000 (@ 0x10)
	"ITF_ADA_05_TRES_02", // = 145 0x4000 (@ 0x10)
	"ITF_ADA_05_TRES_03", // = 146 0x2000 (@ 0x10)
	"ITF_ADA_05_TRES_04", // = 147 0x1000 (@ 0x10)
	"ITF_R51E_ROCKET", // = 148 0x800 (@ 0x10)
	"ITF_R50B_KEY", // = 149 0x400 (@ 0x10)
};

const char* Flags_SCENARIO_Names[] = {
	"SCF_NULL", // = 0 0x80000000 (@ 0x0)
	"SCF_R104_EVENT_4", // = 1 0x40000000 (@ 0x0)
	"SCF_R110_FLOOR_MOVE", // = 2 0x20000000 (@ 0x0)
	"SCF_ACTIVATOR_SP_MES", // = 3 0x10000000 (@ 0x0)
	"SCF_BLUE_HERB_MES", // = 4 0x8000000 (@ 0x0)
	"SCF_R102_VIRUS_OCCUR", // = 5 0x4000000 (@ 0x0)
	"SCF_R01E_TEST", // = 6 0x2000000 (@ 0x0)
	"SCF_R100_TEST00", // = 7 0x1000000 (@ 0x0)
	"SCF_R100_TEST01", // = 8 0x800000 (@ 0x0)
	"SCF_R100_TEST02", // = 9 0x400000 (@ 0x0)
	"SCF_R106_EVENT", // = 10 0x200000 (@ 0x0)
	"SCF_R117_FIND_ASHLEY", // = 11 0x100000 (@ 0x0)
	"SCF_R100_DOG_RUN", // = 12 0x80000 (@ 0x0)
	"SCF_ST1_SUB_MISSION", // = 13 0x40000 (@ 0x0)
	"SCF_R11C_BESIEGED_EVENT", // = 14 0x20000 (@ 0x0)
	"SCF_R201_EVENT00", // = 15 0x10000 (@ 0x0)
	"SCF_R108_PUZZLE_CLEAR", // = 16 0x8000 (@ 0x0)
	"SCF_R100_KILL_GANADE_1ST", // = 17 0x4000 (@ 0x0)
	"SCF_R101_ENTER", // = 18 0x2000 (@ 0x0)
	"SCF_R103_ENTER", // = 19 0x1000 (@ 0x0)
	"SCF_R106_ENTER", // = 20 0x800 (@ 0x0)
	"SCF_R106_CONFINEED_WITH_LUIS", // = 21 0x400 (@ 0x0)
	"SCF_R108_CHECK_DOOR", // = 22 0x200 (@ 0x0)
	"SCF_R10C_GET_CREST", // = 23 0x100 (@ 0x0)
	"SCF_NO_ASHLEY_DIST_CK", // = 24 0x80 (@ 0x0)
	"SCF_R11C_BESIEGED_END_EVENT", // = 25 0x40 (@ 0x0)
	"SCF_R103_CLOSE_COVER", // = 26 0x20 (@ 0x0)
	"SCF_R103_ITEM_IN_CESSPIT", // = 27 0x10 (@ 0x0)
	"SCF_R11B_END_SALAMANDER", // = 28 0x8 (@ 0x0)
	"SCF_ST1_MAP_DAY", // = 29 0x4 (@ 0x0)
	"SCF_ST1_MAP_NIGHT", // = 30 0x2 (@ 0x0)
	"SCF_ST2_MAP", // = 31 0x1 (@ 0x0)
	"SCF_ST3_MAP", // = 32 0x80000000 (@ 0x4)
	"SCF_R217_PUZZLE_CLEAR", // = 33 0x40000000 (@ 0x4)
	"SCF_R104_MEET_MERCHANT", // = 34 0x20000000 (@ 0x4)
	"SCF_R206_ASHLEY_RESCUE", // = 35 0x10000000 (@ 0x4)
	"SCF_R101_IMPRISON", // = 36 0x8000000 (@ 0x4)
	"SCF_R103_OPEN_COVER", // = 37 0x4000000 (@ 0x4)
	"SCF_R20D_END_OF_ASHLEY_PLAY", // = 38 0x2000000 (@ 0x4)
	"SCF_ST1_NIGHT", // = 39 0x1000000 (@ 0x4)
	"SCF_ST2_IN", // = 40 0x800000 (@ 0x4)
	"SCF_CONTACT_MERCHANT", // = 41 0x400000 (@ 0x4)
	"SCF_R10E_STOCK_DAY", // = 42 0x200000 (@ 0x4)
	"SCF_R10E_STOCK_NIGHT", // = 43 0x100000 (@ 0x4)
	"SCF_R108_OPERATOR", // = 44 0x80000 (@ 0x4)
	"SCF_R204_ASHLEY_SPLIT", // = 45 0x40000 (@ 0x4)
	"SCF_R11C_OPERATOR", // = 46 0x20000 (@ 0x4)
	"SCF_ST3_IN", // = 47 0x10000 (@ 0x4)
	"SCF_ST1_SUB_PERFECT", // = 48 0x8000 (@ 0x4)
	"SCF_R104_MERCHANT_MARK", // = 49 0x4000 (@ 0x4)
	"SCF_ST1_NIGHT_LV_ADD", // = 50 0x2000 (@ 0x4)
	"SCF_R307_REGENERATER_APPEAR", // = 51 0x1000 (@ 0x4)
	"SCF_R316_TO_R30A_CUTBACK_EVENT", // = 52 0x800 (@ 0x4)
	"SCF_R30D_ENTER", // = 53 0x400 (@ 0x4)
	"SCF_R332_BOSS_DIE", // = 54 0x200 (@ 0x4)
	"SCF_ADA_COUNT_DOWN_START", // = 55 0x100 (@ 0x4)
	"SCF_ST3_COUNT_DOWN_START", // = 56 0x80 (@ 0x4)
	"SCF_R213_ASHLEY_LOST", // = 57 0x40 (@ 0x4)
	"SCF_R317_LEON_WOUND", // = 58 0x20 (@ 0x4)
	"SCF_R120_EVENT_CANCEL", // = 59 0x10 (@ 0x4)
	"SCF_R22C_BONUS_1", // = 60 0x8 (@ 0x4)
	"SCF_R22C_BONUS_2", // = 61 0x4 (@ 0x4)
	"SCF_R22C_BONUS_3", // = 62 0x2 (@ 0x4)
	"SCF_R22C_BONUS_4", // = 63 0x1 (@ 0x4)
	"SCF_R321_HERI_DOWN", // = 64 0x80000000 (@ 0x8)
	"SCF_R329_ASHLEY_HELP", // = 65 0x40000000 (@ 0x8)
	"SCF_R405_ADA_GAME_INIT", // = 66 0x20000000 (@ 0x8)
	"SCF_R31C_TOWER_EXPLODE", // = 67 0x10000000 (@ 0x8)
	"SCF_R206_ASHLEY_GAME", // = 68 0x8000000 (@ 0x8)
	"SCF_R201_SET_3OBJ", // = 69 0x4000000 (@ 0x8)
	"SCF_R229_IN", // = 70 0x2000000 (@ 0x8)
	"SCF_R225_IN", // = 71 0x1000000 (@ 0x8)
	"SCF_R226_IN", // = 72 0x800000 (@ 0x8)
	"SCF_R300_00", // = 73 0x400000 (@ 0x8)
	"SCF_R303_IN", // = 74 0x200000 (@ 0x8)
	"SCF_R304_00", // = 75 0x100000 (@ 0x8)
	"SCF_R30C_ASHLEY_SCREAM", // = 76 0x80000 (@ 0x8)
	"SCF_R309_GET_KEY", // = 77 0x40000 (@ 0x8)
	"SCF_R30C_SAVE_ASHLEY", // = 78 0x20000 (@ 0x8)
	"SCF_R317_KNIFE_BATTLE", // = 79 0x10000 (@ 0x8)
	"SCF_R31A_IN", // = 80 0x8000 (@ 0x8)
	"SCF_R31B_U3", // = 81 0x4000 (@ 0x8)
	"SCF_R31C_IN", // = 82 0x2000 (@ 0x8)
	"SCF_R31C_OPEN_DOOR", // = 83 0x1000 (@ 0x8)
	"SCF_R330_END_OPE", // = 84 0x800 (@ 0x8)
	"SCF_R332_KEY_GET", // = 85 0x400 (@ 0x8)
	"SCF_ST3_COUNT_DOWN_DIE", // = 86 0x200 (@ 0x8)
	"SCF_KEY_LOCK", // = 87 0x100 (@ 0x8)
	"SCF_R204_MERCHANT_MOVE", // = 88 0x80 (@ 0x8)
	"SCF_OMAKE_MAP", // = 89 0x40 (@ 0x8)
	"SCF_R100_OFFICERS_FOUND", // = 90 0x20 (@ 0x8)
	"SCF_R21A_R21B_IN", // = 91 0x10 (@ 0x8)
	"SCF_ADA_SHOP_1ST", // = 92 0x8 (@ 0x8)
	"SCF_ADA_SHOP_2ND", // = 93 0x4 (@ 0x8)
	"SCF_ASHLEY_PARA_OFF", // = 94 0x2 (@ 0x8)
	"SCF_UNK_5F", // = 95 0x1 (@ 0x8)
	"SCF_UNK_60", // = 96 0x80000000 (@ 0xC)
	"SCF_UNK_61", // = 97 0x40000000 (@ 0xC)
	"SCF_UNK_62", // = 98 0x20000000 (@ 0xC)
	"SCF_UNK_63", // = 99 0x10000000 (@ 0xC)
	"SCF_UNK_64", // = 100 0x8000000 (@ 0xC)
	"SCF_UNK_65", // = 101 0x4000000 (@ 0xC)
	"SCF_UNK_66", // = 102 0x2000000 (@ 0xC)
	"SCF_UNK_67", // = 103 0x1000000 (@ 0xC)
	"SCF_UNK_68", // = 104 0x800000 (@ 0xC)
	"SCF_UNK_69", // = 105 0x400000 (@ 0xC)
	"SCF_UNK_6A", // = 106 0x200000 (@ 0xC)
	"SCF_UNK_6B", // = 107 0x100000 (@ 0xC)
	"SCF_UNK_6C", // = 108 0x80000 (@ 0xC)
	"SCF_UNK_6D", // = 109 0x40000 (@ 0xC)
	"SCF_UNK_6E", // = 110 0x20000 (@ 0xC)
	"SCF_UNK_6F", // = 111 0x10000 (@ 0xC)
	"SCF_UNK_70", // = 112 0x8000 (@ 0xC)
	"SCF_UNK_71", // = 113 0x4000 (@ 0xC)
	"SCF_UNK_72", // = 114 0x2000 (@ 0xC)
	"SCF_UNK_73", // = 115 0x1000 (@ 0xC)
	"SCF_UNK_74", // = 116 0x800 (@ 0xC)
	"SCF_UNK_75", // = 117 0x400 (@ 0xC)
	"SCF_UNK_76", // = 118 0x200 (@ 0xC)
	"SCF_UNK_77", // = 119 0x100 (@ 0xC)
	"SCF_UNK_78", // = 120 0x80 (@ 0xC)
	"SCF_UNK_79", // = 121 0x40 (@ 0xC)
	"SCF_UNK_7A", // = 122 0x20 (@ 0xC)
	"SCF_UNK_7B", // = 123 0x10 (@ 0xC)
	"SCF_UNK_7C", // = 124 0x8 (@ 0xC)
	"SCF_UNK_7D", // = 125 0x4 (@ 0xC)
	"SCF_UNK_7E", // = 126 0x2 (@ 0xC)
	"SCF_UNK_7F", // = 127 0x1 (@ 0xC)
	"SCF_UNK_80", // = 128 0x80000000 (@ 0x10)
	"SCF_UNK_81", // = 129 0x40000000 (@ 0x10)
	"SCF_UNK_82", // = 130 0x20000000 (@ 0x10)
	"SCF_UNK_83", // = 131 0x10000000 (@ 0x10)
	"SCF_UNK_84", // = 132 0x8000000 (@ 0x10)
	"SCF_UNK_85", // = 133 0x4000000 (@ 0x10)
	"SCF_UNK_86", // = 134 0x2000000 (@ 0x10)
	"SCF_UNK_87", // = 135 0x1000000 (@ 0x10)
	"SCF_UNK_88", // = 136 0x800000 (@ 0x10)
	"SCF_UNK_89", // = 137 0x400000 (@ 0x10)
	"SCF_UNK_8A", // = 138 0x200000 (@ 0x10)
	"SCF_UNK_8B", // = 139 0x100000 (@ 0x10)
	"SCF_UNK_8C", // = 140 0x80000 (@ 0x10)
	"SCF_UNK_8D", // = 141 0x40000 (@ 0x10)
	"SCF_UNK_8E", // = 142 0x20000 (@ 0x10)
	"SCF_UNK_8F", // = 143 0x10000 (@ 0x10)
	"SCF_UNK_90", // = 144 0x8000 (@ 0x10)
	"SCF_UNK_91", // = 145 0x4000 (@ 0x10)
	"SCF_UNK_92", // = 146 0x2000 (@ 0x10)
	"SCF_UNK_93", // = 147 0x1000 (@ 0x10)
	"SCF_UNK_94", // = 148 0x800 (@ 0x10)
	"SCF_UNK_95", // = 149 0x400 (@ 0x10)
	"SCF_UNK_96", // = 150 0x200 (@ 0x10)
	"SCF_UNK_97", // = 151 0x100 (@ 0x10)
	"SCF_UNK_98", // = 152 0x80 (@ 0x10)
	"SCF_UNK_99", // = 153 0x40 (@ 0x10)
	"SCF_UNK_9A", // = 154 0x20 (@ 0x10)
	"SCF_UNK_9B", // = 155 0x10 (@ 0x10)
	"SCF_UNK_9C", // = 156 0x8 (@ 0x10)
	"SCF_UNK_9D", // = 157 0x4 (@ 0x10)
	"SCF_UNK_9E", // = 158 0x2 (@ 0x10)
	"SCF_UNK_9F", // = 159 0x1 (@ 0x10)
	"SCF_UNK_A0", // = 160 0x80000000 (@ 0x14)
	"SCF_UNK_A1", // = 161 0x40000000 (@ 0x14)
	"SCF_UNK_A2", // = 162 0x20000000 (@ 0x14)
	"SCF_UNK_A3", // = 163 0x10000000 (@ 0x14)
	"SCF_UNK_A4", // = 164 0x8000000 (@ 0x14)
	"SCF_UNK_A5", // = 165 0x4000000 (@ 0x14)
	"SCF_UNK_A6", // = 166 0x2000000 (@ 0x14)
	"SCF_FILE_07_GET", // = 167 0x1000000 (@ 0x14)
	"SCF_FILE_08_GET", // = 168 0x800000 (@ 0x14)
	"SCF_UNK_A9", // = 169 0x400000 (@ 0x14)
	"SCF_UNK_AA", // = 170 0x200000 (@ 0x14)
	"SCF_UNK_AB", // = 171 0x100000 (@ 0x14)
	"SCF_UNK_AC", // = 172 0x80000 (@ 0x14)
	"SCF_UNK_AD", // = 173 0x40000 (@ 0x14)
	"SCF_UNK_AE", // = 174 0x20000 (@ 0x14)
	"SCF_UNK_AF", // = 175 0x10000 (@ 0x14)
	"SCF_UNK_B0", // = 176 0x8000 (@ 0x14)
	"SCF_UNK_B1", // = 177 0x4000 (@ 0x14)
	"SCF_UNK_B2", // = 178 0x2000 (@ 0x14)
	"SCF_UNK_B3", // = 179 0x1000 (@ 0x14)
	"SCF_UNK_B4", // = 180 0x800 (@ 0x14)
	"SCF_UNK_B5", // = 181 0x400 (@ 0x14)
	"SCF_UNK_B6", // = 182 0x200 (@ 0x14)
	"SCF_UNK_B7", // = 183 0x100 (@ 0x14)
	"SCF_R316_IN", // = 184 0x80 (@ 0x14)
	"SCF_R102_MEET_MERCHANT", // = 185 0x40 (@ 0x14)
	"SCF_PS2_ADA_MAP", // = 186 0x20 (@ 0x14)
	"SCF_R502_DOOR", // = 187 0x10 (@ 0x14)
	"SCF_R502_GREEN_GEMS", // = 188 0x8 (@ 0x14)
	"SCF_R502_EMBLEM", // = 189 0x4 (@ 0x14)
	"SCF_R508_ENTER", // = 190 0x2 (@ 0x14)
	"SCF_R508_STEEL_KEY", // = 191 0x1 (@ 0x14)
	"SCF_R508_BOOT_GONDOLA", // = 192 0x80000000 (@ 0x18)
	"SCF_R507_EVENT", // = 193 0x40000000 (@ 0x18)
	"SCF_R50B_KEY", // = 194 0x20000000 (@ 0x18)
};

const char* Flags_KEY_LOCK_Names[] = {
	"KYF_NULL", // = 0 0x80000000 (@ 0x0)
	"KYF_R100_IRON_DOOR", // = 1 0x40000000 (@ 0x0)
	"KYF_R101_IRON_DOOR", // = 2 0x20000000 (@ 0x0)
	"KYF_R118_TO_R117_DOOR", // = 3 0x10000000 (@ 0x0)
	"KYF_R113_TO_R11C_DOOR", // = 4 0x8000000 (@ 0x0)
	"KYF_R21E_DOOR", // = 5 0x4000000 (@ 0x0)
	"KYF_R105_TO_R101_DOOR", // = 6 0x2000000 (@ 0x0)
	"KYF_R118_TO_R117_INLOCK", // = 7 0x1000000 (@ 0x0)
	"KYF_R10F_TO_R200_DOOR", // = 8 0x800000 (@ 0x0)
	"KYF_R104_TO_R107_DOOR", // = 9 0x400000 (@ 0x0)
	"KYF_R20D_TO_R206_DOOR", // = 10 0x200000 (@ 0x0)
	"KYF_R11D_IRON_DOOR", // = 11 0x100000 (@ 0x0)
	"KYF_R11E_TO_R10F_DOOR", // = 12 0x80000 (@ 0x0)
	"KYF_R206_TO_R211_DOOR", // = 13 0x40000 (@ 0x0)
	"KYF_R203_TO_R201_DOOR", // = 14 0x20000 (@ 0x0)
	"KYF_R11D_TO_R10F_DOOR", // = 15 0x10000 (@ 0x0)
	"KYF_R201_DOOR", // = 16 0x8000 (@ 0x0)
	"KYF_R201_TO_R210_DOOR", // = 17 0x4000 (@ 0x0)
	"KYF_R306_TO_R308_DOOR", // = 18 0x2000 (@ 0x0)
	"KYF_R30C_DOOR", // = 19 0x1000 (@ 0x0)
	"KYF_R306_TO_R303_DOOR", // = 20 0x800 (@ 0x0)
	"KYF_R30D_TO_R30F_DOOR", // = 21 0x400 (@ 0x0)
	"KYF_R31C_TO_R320_DOOR", // = 22 0x200 (@ 0x0)
	"KYF_R306_TO_R30B_DOOR", // = 23 0x100 (@ 0x0)
	"KYF_ST1_00", // = 24 0x80 (@ 0x0)
	"KYF_ST1_01", // = 25 0x40 (@ 0x0)
	"KYF_ST1_02", // = 26 0x20 (@ 0x0)
	"KYF_ST1_03", // = 27 0x10 (@ 0x0)
	"KYF_ST1_04", // = 28 0x8 (@ 0x0)
	"KYF_ST1_05", // = 29 0x4 (@ 0x0)
	"KYF_ST1_06", // = 30 0x2 (@ 0x0)
	"KYF_ST1_07", // = 31 0x1 (@ 0x0)
	"KYF_ST1_08", // = 32 0x80000000 (@ 0x4)
	"KYF_ST1_09", // = 33 0x40000000 (@ 0x4)
	"KYF_ST1_10", // = 34 0x20000000 (@ 0x4)
	"KYF_ST1_12", // = 35 0x10000000 (@ 0x4)
	"KYF_ST1_13", // = 36 0x8000000 (@ 0x4)
	"KYF_ST1_14", // = 37 0x4000000 (@ 0x4)
	"KYF_ST1_15", // = 38 0x2000000 (@ 0x4)
	"KYF_ST1_16", // = 39 0x1000000 (@ 0x4)
	"KYF_ST1_17", // = 40 0x800000 (@ 0x4)
	"KYF_ST1_18", // = 41 0x400000 (@ 0x4)
	"KYF_ST1_19", // = 42 0x200000 (@ 0x4)
	"KYF_ST1_20", // = 43 0x100000 (@ 0x4)
	"KYF_ST1_21", // = 44 0x80000 (@ 0x4)
	"KYF_ST1_22", // = 45 0x40000 (@ 0x4)
	"KYF_ST1_23", // = 46 0x20000 (@ 0x4)
	"KYF_ST1_24", // = 47 0x10000 (@ 0x4)
	"KYF_ST1_25", // = 48 0x8000 (@ 0x4)
	"KYF_ST1_26", // = 49 0x4000 (@ 0x4)
	"KYF_ST2_00", // = 50 0x2000 (@ 0x4)
	"KYF_ST2_01", // = 51 0x1000 (@ 0x4)
	"KYF_ST2_02", // = 52 0x800 (@ 0x4)
	"KYF_ST2_03", // = 53 0x400 (@ 0x4)
	"KYF_ST2_04", // = 54 0x200 (@ 0x4)
	"KYF_ST2_05", // = 55 0x100 (@ 0x4)
	"KYF_ST2_06", // = 56 0x80 (@ 0x4)
	"KYF_ST2_07", // = 57 0x40 (@ 0x4)
	"KYF_ST2_08", // = 58 0x20 (@ 0x4)
	"KYF_ST2_09", // = 59 0x10 (@ 0x4)
	"KYF_ST2_10", // = 60 0x8 (@ 0x4)
	"KYF_ST2_11", // = 61 0x4 (@ 0x4)
	"KYF_ST2_12", // = 62 0x2 (@ 0x4)
	"KYF_ST2_13", // = 63 0x1 (@ 0x4)
	"KYF_ST2_14", // = 64 0x80000000 (@ 0x8)
	"KYF_ST2_15", // = 65 0x40000000 (@ 0x8)
	"KYF_ST2_16", // = 66 0x20000000 (@ 0x8)
	"KYF_ST2_17", // = 67 0x10000000 (@ 0x8)
	"KYF_ST2_18", // = 68 0x8000000 (@ 0x8)
	"KYF_ST2_19", // = 69 0x4000000 (@ 0x8)
	"KYF_ST2_20", // = 70 0x2000000 (@ 0x8)
	"KYF_ST2_21", // = 71 0x1000000 (@ 0x8)
	"KYF_ST2_22", // = 72 0x800000 (@ 0x8)
	"KYF_ST2_23", // = 73 0x400000 (@ 0x8)
	"KYF_ST2_24", // = 74 0x200000 (@ 0x8)
	"KYF_ST2_25", // = 75 0x100000 (@ 0x8)
	"KYF_ST2_26", // = 76 0x80000 (@ 0x8)
	"KYF_ST2_27", // = 77 0x40000 (@ 0x8)
	"KYF_ST2_28", // = 78 0x20000 (@ 0x8)
	"KYF_ST2_29", // = 79 0x10000 (@ 0x8)
	"KYF_ST2_30", // = 80 0x8000 (@ 0x8)
	"KYF_ST2_31", // = 81 0x4000 (@ 0x8)
	"KYF_ST2_32", // = 82 0x2000 (@ 0x8)
	"KYF_ST2_33", // = 83 0x1000 (@ 0x8)
	"KYF_ST2_34", // = 84 0x800 (@ 0x8)
	"KYF_ST2_35", // = 85 0x400 (@ 0x8)
	"KYF_ST2_36", // = 86 0x200 (@ 0x8)
	"KYF_ST2_37", // = 87 0x100 (@ 0x8)
	"KYF_ST2_38", // = 88 0x80 (@ 0x8)
	"KYF_ST2_39", // = 89 0x40 (@ 0x8)
	"KYF_ST3_00", // = 90 0x20 (@ 0x8)
	"KYF_ST3_01", // = 91 0x10 (@ 0x8)
	"KYF_ST3_02", // = 92 0x8 (@ 0x8)
	"KYF_ST3_03", // = 93 0x4 (@ 0x8)
	"KYF_ST3_04", // = 94 0x2 (@ 0x8)
	"KYF_ST3_05", // = 95 0x1 (@ 0x8)
	"KYF_ST3_06", // = 96 0x80000000 (@ 0xC)
	"KYF_ST3_07", // = 97 0x40000000 (@ 0xC)
	"KYF_ST3_08", // = 98 0x20000000 (@ 0xC)
	"KYF_ST3_09", // = 99 0x10000000 (@ 0xC)
	"KYF_ST3_10", // = 100 0x8000000 (@ 0xC)
	"KYF_ST3_11", // = 101 0x4000000 (@ 0xC)
	"KYF_ST3_12", // = 102 0x2000000 (@ 0xC)
	"KYF_ST3_13", // = 103 0x1000000 (@ 0xC)
	"KYF_ST3_14", // = 104 0x800000 (@ 0xC)
	"KYF_ST3_15", // = 105 0x400000 (@ 0xC)
	"KYF_ST3_16", // = 106 0x200000 (@ 0xC)
	"KYF_ST3_17", // = 107 0x100000 (@ 0xC)
	"KYF_ST3_18", // = 108 0x80000 (@ 0xC)
	"KYF_ST3_19", // = 109 0x40000 (@ 0xC)
	"KYF_ST3_20", // = 110 0x20000 (@ 0xC)
	"KYF_ST3_21", // = 111 0x10000 (@ 0xC)
	"KYF_ST3_22", // = 112 0x8000 (@ 0xC)
	"KYF_ST3_23", // = 113 0x4000 (@ 0xC)
	"KYF_ST3_24", // = 114 0x2000 (@ 0xC)
	"KYF_ST3_25", // = 115 0x1000 (@ 0xC)
	"KYF_ST3_26", // = 116 0x800 (@ 0xC)
	"KYF_ST3_27", // = 117 0x400 (@ 0xC)
	"KYF_ST3_28", // = 118 0x200 (@ 0xC)
	"KYF_ST3_29", // = 119 0x100 (@ 0xC)
	"KYF_ST3_30", // = 120 0x80 (@ 0xC)
	"KYF_ST3_31", // = 121 0x40 (@ 0xC)
	"KYF_ST3_32", // = 122 0x20 (@ 0xC)
	"KYF_ST3_33", // = 123 0x10 (@ 0xC)
	"KYF_ST3_34", // = 124 0x8 (@ 0xC)
	"KYF_ST4_00", // = 125 0x4 (@ 0xC)
	"KYF_ST4_01", // = 126 0x2 (@ 0xC)
	"KYF_ST4_02", // = 127 0x1 (@ 0xC)
	"KYF_ST4_03", // = 128 0x80000000 (@ 0x10)
	"KYF_ST5_00", // = 129 0x40000000 (@ 0x10)
	"KYF_ST5_01", // = 130 0x20000000 (@ 0x10)
	"KYF_ST5_02", // = 131 0x10000000 (@ 0x10)
	"KYF_ST5_03", // = 132 0x8000000 (@ 0x10)
	"KYF_ST5_04", // = 133 0x4000000 (@ 0x10)
	"KYF_ST5_05", // = 134 0x2000000 (@ 0x10)
	"KYF_ST5_06", // = 135 0x1000000 (@ 0x10)
	"KYF_ST5_07", // = 136 0x800000 (@ 0x10)
	"KYF_ST5_08", // = 137 0x400000 (@ 0x10)
	"KYF_ST5_09", // = 138 0x200000 (@ 0x10)
	"KYF_ST5_10", // = 139 0x100000 (@ 0x10)
	"KYF_ST5_11", // = 140 0x80000 (@ 0x10)
	"KYF_ST5_12", // = 141 0x40000 (@ 0x10)
	"KYF_ST5_13", // = 142 0x20000 (@ 0x10)
	"KYF_ST5_14", // = 143 0x10000 (@ 0x10)
	"KYF_ST5_15", // = 144 0x8000 (@ 0x10)
	"KYF_ST5_16", // = 145 0x4000 (@ 0x10)
	"KYF_ST5_17", // = 146 0x2000 (@ 0x10)
	"KYF_ST5_18", // = 147 0x1000 (@ 0x10)
	"KYF_ST5_19", // = 148 0x800 (@ 0x10)
	"KYF_MAX", // = 149 0x400 (@ 0x10)
};

const char* Flags_EXTRA_Names[] = {
	"EXT_COSTUME", // 0x80000000 (@ 0x0)
	"EXT_HARD_MODE", // 0x40000000 (@ 0x0)
	"EXT_GET_SW500", // 0x20000000 (@ 0x0)
	"EXT_GET_TOMPSON", // 0x10000000 (@ 0x0)
	"EXT_GET_ADA", // 0x8000000 (@ 0x0)
	"EXT_GET_HUNK", // 0x4000000 (@ 0x0)
	"EXT_GET_KLAUSER", // 0x2000000 (@ 0x0)
	"EXT_GET_WESKER", // 0x1000000 (@ 0x0)
	"EXT_GET_OMAKE_ADA_GAME", // 0x800000 (@ 0x0)
	"EXT_GET_OMAKE_ETC_GAME", // 0x400000 (@ 0x0)
	"EXT_ASHLEY_ARMOR", // 0x200000 (@ 0x0)
	"EXT_GET_PS2_ADA_GAME", // 0x100000 (@ 0x0)
	"EXT_COSTUME_MAFIA", // 0x80000 (@ 0x0)
	"EXT_GET_LASER", // 0x40000 (@ 0x0)
	"EXT_GET_ADA_TOMPSON", // 0x20000 (@ 0x0)
	"EXT_GET_ADAS_REPORT", // 0x10000 (@ 0x0)
};

const char* Flags_CONFIG_Names[] = {
	"CFG_AIM_REVERSE", // 0x80000000 (@ 0x0)
	"CFG_WIDE_MODE", // 0x40000000 (@ 0x0)
	"CFG_LOCK_ON", // 0x20000000 (@ 0x0)
	"CFG_BONUS_GET", // 0x10000000 (@ 0x0)
	"CFG_VIBRATION", // 0x8000000 (@ 0x0)
	"CFG_KNIFE_MODE", // 0x4000000 (@ 0x0)
};

const char* Flags_DISP_Names[] = {
	"DPF_EM", // 0x80000000 (@ 0x0)
	"DPF_PL", // 0x40000000 (@ 0x0)
	"DPF_SUBCHAR", // 0x20000000 (@ 0x0)
	"DPF_OBJ", // 0x10000000 (@ 0x0)
	"DPF_SCR", // 0x8000000 (@ 0x0)
	"DPF_ESP", // 0x4000000 (@ 0x0)
	"DPF_SHADOW", // 0x2000000 (@ 0x0)
	"DPF_WATER", // 0x1000000 (@ 0x0)
	"DPF_MIRROR", // 0x800000 (@ 0x0)
	"DPF_CTRL", // 0x400000 (@ 0x0)
	"DPF_CINESCO", // 0x200000 (@ 0x0)
	"DPF_FILTER", // 0x100000 (@ 0x0)
	"DPF_GLB_ILM", // 0x80000 (@ 0x0)
	"DPF_CAST_SHADOW", // 0x40000 (@ 0x0)
	"DPF_CLOTH", // 0x20000 (@ 0x0)
	"DPF_COCKPIT", // 0x10000 (@ 0x0)
	"DPF_SELF_SHADOW", // 0x8000 (@ 0x0)
	"DPF_FOG", // 0x4000 (@ 0x0)
	"DPF_ID_SYSTEM", // 0x2000 (@ 0x0)
	"DPF_ACTBTN", // 0x1000 (@ 0x0)
	"DPF_MESSAGE", // 0x800 (@ 0x0)
	"DPF_TEX_RENDER", // 0x400 (@ 0x0)
	"DPF_EFFECT_VU1", // 0x200 (@ 0x0)
	"DPF_17",
	"DPF_18",
	"DPF_19",
	"DPF_1A",
	"DPF_1B",
	"DPF_1C",
	"DPF_1D",
	"DPF_1E",
	"DPF_1F"
};

const char* Flags_ROOM_SAVE_Names[] = {
	"RSF_00",
	"RSF_01",
	"RSF_02",
	"RSF_03",
	"RSF_04",
	"RSF_05",
	"RSF_06",
	"RSF_07",
	"RSF_08",
	"RSF_09",
	"RSF_10",
	"RSF_11",
	"RSF_12",
	"RSF_13",
	"RSF_14",
	"RSF_15",
	"RSF_16",
	"RSF_17",
	"RSF_18",
	"RSF_19",
	"RSF_20",
	"RSF_21",
	"RSF_22",
	"RSF_23",
	"RSF_24",
	"RSF_25",
	"RSF_26",
	"RSF_27",
	"RSF_28",
	"RSF_29",
	"RSF_30",
	"RSF_31",
};

const char* Flags_ROOM_SAVE_r226_Names[] =
{
	"RSF_00",
	"RSF_01",
	"RSF_EVENT_ROBO_WALK_PASSAGE_START",
	"RSF_EVENT_ROBO_WALK_PASSAGE_END",
	"RSF_EVENT_DOOR_OPEN",
	"RSF_EVENT_ROBO_WALK_BRIDGE_START",
	"RSF_EVENT_ROBO_WALK_BRIDGE_END",
	"RSF_EVENT_PASSAGE_SWITCH00",
	"RSF_EVENT_PASSAGE_SWITCH01",
	"RSF_EVENT_ROBO_START",
	"RSF_EVENT_ROBO_WATCH",
	"RSF_EMRESET00",
	"RSF_BRIDGE_DW",
	"RSF_ROBO_FALL",
	"RSF_EVENT_TOWER_LOOK",
	"RSF_EMRESET_3F",
	"RSF_EMRESET_1F",
	"RSF_CONTINUE_POINT_SET",
	"RSF_EMSET_179",
	"RSF_EMSET_181",
	"RSF_EMSET_END",
	"RSF_21",
	"RSF_22",
	"RSF_23",
	"RSF_24",
	"RSF_25",
	"RSF_26",
	"RSF_27",
	"RSF_28",
	"RSF_29",
	"RSF_30",
	"RSF_31"
};

const char* Flags_ROOM_Names[] = {
	"RMF_0",
	"RMF_1",
	"RMF_2",
	"RMF_3",
	"RMF_4",
	"RMF_5",
	"RMF_6",
	"RMF_7",
	"RMF_8",
	"RMF_9",
	"RMF_10",
	"RMF_11",
	"RMF_12",
	"RMF_13",
	"RMF_14",
	"RMF_15",
	"RMF_16",
	"RMF_17",
	"RMF_18",
	"RMF_19",
	"RMF_20",
	"RMF_21",
	"RMF_22",
	"RMF_23",
	"RMF_24",
	"RMF_25",
	"RMF_26",
	"RMF_27",
	"RMF_28",
	"RMF_29",
	"RMF_30",
	"RMF_31",
	"RMF_32",
	"RMF_33",
	"RMF_34",
	"RMF_35",
	"RMF_36",
	"RMF_37",
	"RMF_38",
	"RMF_39",
	"RMF_40",
	"RMF_41",
	"RMF_42",
	"RMF_43",
	"RMF_44",
	"RMF_45",
	"RMF_46",
	"RMF_47",
	"RMF_48",
	"RMF_49",
	"RMF_50",
	"RMF_51",
	"RMF_52",
	"RMF_53",
	"RMF_54",
	"RMF_55",
	"RMF_56",
	"RMF_57",
	"RMF_58",
	"RMF_59",
	"RMF_60",
	"RMF_61",
	"RMF_62",
	"RMF_63",
	"RMF_64",
	"RMF_65",
	"RMF_66",
	"RMF_67",
	"RMF_68",
	"RMF_69",
	"RMF_70",
	"RMF_71",
	"RMF_72",
	"RMF_73",
	"RMF_74",
	"RMF_75",
	"RMF_76",
	"RMF_77",
	"RMF_78",
	"RMF_79",
	"RMF_80",
	"RMF_81",
	"RMF_82",
	"RMF_83",
	"RMF_84",
	"RMF_85",
	"RMF_86",
	"RMF_87",
	"RMF_88",
	"RMF_89",
	"RMF_90",
	"RMF_91",
	"RMF_92",
	"RMF_93",
	"RMF_94",
	"RMF_95",
	"RMF_96",
	"RMF_97",
	"RMF_98",
	"RMF_99",
	"RMF_100",
	"RMF_101",
	"RMF_102",
	"RMF_103",
	"RMF_104",
	"RMF_105",
	"RMF_106",
	"RMF_107",
	"RMF_108",
	"RMF_109",
	"RMF_110",
	"RMF_111",
	"RMF_112",
	"RMF_113",
	"RMF_114",
	"RMF_115",
	"RMF_116",
	"RMF_117",
	"RMF_118",
	"RMF_119",
	"RMF_120",
	"RMF_121",
	"RMF_122",
	"RMF_123",
	"RMF_124",
	"RMF_125",
	"RMF_126",
	"RMF_127"
};

const char* Flags_ROOM_r226_Names[] = {
	"RMF_BOBO_SWITCH_EXEC_FRONT",
	"RMF_BOBO_SWITCH_EXEC_BACK",
	"RMF_PILLAR_ESCAPE_ON",
	"RMF_PILLAR_ESCAPE_ING",
	"RMF_PILLAR_ESCAPE_LAST",
	"RMF_PILLAR_SET_6",
	"RMF_PILLAR_SET_7",
	"RMF_PILLAR_SET_8",
	"RMF_PILLAR_SET_9",
	"RMF_PILLAR_SET_10",
	"RMF_PILLAR_SET_11",
	"RMF_PILLAR_SET_12",
	"RMF_PILLAR_SET_13",
	"RMF_13",
	"RMF_14",
	"RMF_BOBO_DOOR_PUNCH",
	"RMF_BOBO_SWITCH_FRONT",
	"RMF_BOBO_SWITCH_BACK",
	"RMF_BRIDGE_ST_00",
	"RMF_BRIDGE_ST_01",
	"RMF_BRIDGE_ST_02",
	"RMF_BRIDGE_ST_03",
	"RMF_BRIDGE_ST_04",
	"RMF_BRIDGE_ST_05",
	"RMF_BRIDGE_DIE_00",
	"RMF_BRIDGE_DIE_01",
	"RMF_BRIDGE_DIE_02",
	"RMF_BRIDGE_DIE_03",
	"RMF_BRIDGE_DIE_04",
	"RMF_BRIDGE_DIE_05",
	"RMF_BRIDGE_ON_AVOID",
	"RMF_BRIDGE_ON_SAFE",
	"RMF_PLAYER_DIE_PASSAGE_SET",
	"RMF_PLAYER_DIE_BRIDGE_SET",
	"RMF_PLAYER_DIE_ING",
	"RMF_BGM_ON",
	"RMF_ROBO_DOOR_BREAK",
	"RMF_37",
	"RMF_38",
	"RMF_39",
	"RMF_40",
	"RMF_41",
	"RMF_42",
	"RMF_43",
	"RMF_44",
	"RMF_45",
	"RMF_46",
	"RMF_47",
	"RMF_48",
	"RMF_49",
	"RMF_50",
	"RMF_51",
	"RMF_52",
	"RMF_53",
	"RMF_54",
	"RMF_55",
	"RMF_56",
	"RMF_57",
	"RMF_58",
	"RMF_59",
	"RMF_60",
	"RMF_61",
	"RMF_62",
	"RMF_63",
	"RMF_FLAG_EMRESET00",
	"RMF_BRIDGE_ON_00",
	"RMF_BRIDGE_ON_01",
	"RMF_BRIDGE_ON_02",
	"RMF_BRIDGE_ON_03",
	"RMF_BRIDGE_ON_04",
	"RMF_BRIDGE_ON_05",
	"RMF_EMSET_00",
	"RMF_EMSET_01",
	"RMF_73",
	"RMF_74",
	"RMF_75",
	"RMF_76",
	"RMF_77",
	"RMF_78",
	"RMF_79",
	"RMF_80",
	"RMF_81",
	"RMF_82",
	"RMF_83",
	"RMF_84",
	"RMF_85",
	"RMF_86",
	"RMF_87",
	"RMF_88",
	"RMF_89",
	"RMF_90",
	"RMF_91",
	"RMF_92",
	"RMF_93",
	"RMF_94",
	"RMF_95",
	"RMF_96",
	"RMF_97",
	"RMF_98",
	"RMF_99",
	"RMF_100",
	"RMF_101",
	"RMF_102",
	"RMF_103",
	"RMF_104",
	"RMF_105",
	"RMF_106",
	"RMF_107",
	"RMF_108",
	"RMF_109",
	"RMF_110",
	"RMF_111",
	"RMF_112",
	"RMF_113",
	"RMF_114",
	"RMF_115",
	"RMF_116",
	"RMF_117",
	"RMF_118",
	"RMF_119",
	"RMF_120",
	"RMF_121",
	"RMF_122",
	"RMF_123",
	"RMF_124",
	"RMF_125",
	"RMF_126",
	"RMF_127"
};

// make sure to sync any changes here with EItemId (item.h)
const char* ITEM_TYPE_Names[] = {
	"Unk0",
	"Wep",
	"Ammo",
	"Wep", // ITEM_TYPE_THROWABLE
	"Unk4",
	"Treasure",
	"Consumable",
	"Key Item",
	"Treasure", // ITEM_TYPE_TREASURE_MERCS
	"Mod",
	"File",
	"Misc", // ITEM_TYPE_TREASURE_MAP
	"Gem",
	"Bottlecap",
	"Key Item" // ITEM_TYPE_IMPORTANT
};

const char* EItemId_Names[] = {
	"Magnum Ammo", // Bullet_45in_H
	"Hand Grenade",
	"Incendiary Grenade",
	"Matilda", // VP70
	"Handgun Ammo", // Bullet_9mm_H
	"First Aid Spray",
	"Green Herb", // Herb_G
	"Rifle Ammo", // Bullet_223in
	"Chicken Egg", // Hen_Egg
	"Brown Chicken Egg", // Iodine_Egg
	"Gold Chicken Egg", // Golden_Egg
	"Luis_Drug",
	"Plaga Sample", // Parasite_Sample
	"Krauser_Knife",
	"Flash Grenade", // Light_Grenade
	"Salazar_Crest",
	"Bowgun",
	"Bowgun Bolts",
	"Green Herb (X2)", // Herb_G_G
	"Green Herb (X3)", // Herb_G_G_G
	"Mixed Herbs (G+R)", // Herb_G_R
	"Mixed Herbs (G+R+Y)", // Herb_G_R_Y
	"Mixed Herbs (G+Y)", // Herb_G_Y
	"Rocket Launcher (Special)", // Ada_RPG
	"Shotgun Shells", // Bullet_12gg
	"Red Herb", // Herb_R
	"Handcannon Ammo", // Bullet_5in
	"Key_Sand_Clock",
	"Yellow Herb", // Herb_Y
	"Piece_Of_Slate",
	"Golden_Gem",
	"Silver_Gem",
	"TMP Ammo", // Bullet_9mm_M
	"Punisher (FN57)", // FN57
	"Scope (P.R.L. 412)", // Scope_New_Weapon (shows as Punisher w/ Silencer ingame??)
	"Handgun", // Ruger
	"Ruger_SA",
	"Red9", // Mauser
	"Mauser_ST",
	"Blacktail", // XD9
	"New_Weapon_SC",
	"Broken Butterfly", // Civilian
	"Killer7", // Gov
	"Ada_New_Weapon",
	"Shotgun",
	"Striker",
	"Rifle", // S_Field
	"Rifle (semi-auto)", // HK_Sniper
	"TMP", // Styer
	"R513_Key_0a",
	"Styer_St",
	"R513_Key_0b",
	"Chicago Typewriter", // Thompson
	"Rocket Launcher", // RPG7
	"Mine Thrower", // Mine
	"Hand Cannon", // SW500
	"Knife",
	"Cupric_Gem",
	"Moon_Spall_1",
	"Cult_Key",
	"Cult_Crest",
	"False_Eye",
	"Custom TMP", // Krauser_Machine_Gun
	"Silencer (Handgun)", // Silencer_9mm
	"Punisher (Item_40)", // "Item_40" in re4vr
	"P.R.L. 412", // "New_Weapon" in re4vr
	"Stock (Red9)", // Stock_Mauser
	"Stock (TMP)", // Stock_Styer
	"Scope (Rifle)", // Scope_Sniper
	"Scope (semi-auto rifle)", // Scope_HK_Sniper
	"Mine-Darts", // Bullet_Mine_A
	"Shotgun (Ada)",
	"File_13",
	"File_14",
	"File_15",
	"File_16",
	"File_17",
	"File_18",
	"File_19",
	"File_20",
	"File_21",
	"HK_Sniper_Thermo",
	"Krauser's Bow",
	"Chicago Typewriter (Ada)", // Ada_Machine_Gun
	"Treasure_Map_2",
	"Treasure_Map_3",
	"Ore_White",
	"Ore_Black",
	"Pearl_Pendant",
	"Brass_Fob_Watch",
	"Silver_Whistle",
	"Gold_Dish",
	"Platinum_Cup",
	"Crystal_Mask",
	"Beer_Stein",
	"Agate_Green",
	"Garnett_Red",
	"Amber_Yellow",
	"Beer_Stein_G",
	"Beer_Stein_R",
	"Beer_Stein_Y",
	"Beer_Stein_G_R",
	"Beer_Stein_G_Y",
	"Beer_Stein_R_Y",
	"Beer_Stein_G_R_Y",
	"Moon_Spall_2",
	"Chicago Typewriter Ammo", // Bullet_45in_M
	"S_Field_Sc",
	"HK_Sniper_Sc",
	"Infinite Launcher", // Omake_RPG
	"Pagan_Grail_1",
	"Pagan_Grail_2",
	"Pagan_Rod",
	"Ingot_Bar",
	"Arrows", // Bullet_Arrow
	"Time_Bonus",
	"R327_Card_Key",
	"Point_Bonus",
	"Key_Green_Gem",
	"Ruby",
	"Gold_Box_S_Std",
	"Gold_Box_L_Std",
	"Moon_Crest",
	"Seal_Key",
	"Attache_Case_S",
	"Attache_Case_M",
	"Attache_Case_L",
	"Attache_Case_O",
	"Golden_Sword",
	"Key_To_R50b",
	"Dragon_Dream",
	"Key_To_R30c",
	"Key_To_R308",
	"Crest_A",
	"Crest_B",
	"Crest_C",
	"Key_To_Jet_Ski",
	"Smelly_Pendant",
	"Smelly_Fob_Watch",
	"Key_To_Shrine",
	"Key_To_Barrier",
	"Dynamite",
	"Key_To_Control_Room",
	"Gold_Bracelet",
	"Perfume_Bottle",
	"Pearl_Ruby_Mirror",
	"Key_To_R30b",
	"Plush_Chessboard",
	"Riot Gun",
	"Black Bass",
	"Sand_Clock",
	"Black Bass (L)",
	"Evil_Gem",
	"S_Field_Thermo",
	"Pot_Holed_Crown",
	"Kingdom_Heart",
	"Royal_Crest",
	"Crown_Heart",
	"Crown_Crest",
	"Crown_Heart_Crest",
	"Item_A0",
	"Spinel_02",
	"Pedestal",
	"Key_To_Salon",
	"Crest_Right",
	"Crest_Left",
	"Crest_Full",
	"Key_To_201",
	"Mixed Herbs (R+Y)", // Herb_R_Y
	"Treasure_Map",
	"Scope (Mine Thrower)", // Scope_Mine
	"Mine_SC",
	"File_01",
	"File_02",
	"File_03",
	"File_04",
	"File_05",
	"File_06",
	"File_07",
	"File_08",
	"File_09",
	"File_10",
	"File_11",
	"File_12",
	"Lantern",
	"Lantern_Stone_G",
	"Lantern_Stone_R",
	"Lantern_Stone_Y",
	"Lantern_G",
	"Lantern_R",
	"Lantern_Y",
	"Lantern_G_R",
	"Lantern_G_Y",
	"Lantern_R_Y",
	"Lantern_G_R_Y",
	"Dungeon_Key",
	"Silver_Sword",
	"Infrared Scope", // Scope_Thermo
	"Mask",
	"Mask_Stone_G",
	"Mask_Stone_R",
	"Mask_Stone_Y",
	"Mask_G",
	"Mask_R",
	"Mask_Y",
	"Mask_G_R",
	"Mask_G_Y",
	"Mask_R_Y",
	"Mask_G_R_Y",
	"Cat_Statue",
	"Cat_Stone_G",
	"Cat_Stone_R",
	"Cat_Stone_Y",
	"Cat_Statue_G",
	"Cat_Statue_R",
	"Cat_Statue_Y",
	"Cat_Statue_G_R",
	"Cat_Statue_G_Y",
	"Cat_Statue_R_Y",
	"Cat_Statue_G_R_Y",
	"Bottle_Cap_01",
	"Bottle_Cap_02",
	"Bottle_Cap_03",
	"Bottle_Cap_04",
	"Bottle_Cap_05",
	"Bottle_Cap_06",
	"Bottle_Cap_07",
	"Bottle_Cap_08",
	"Bottle_Cap_09",
	"Bottle_Cap_10",
	"Bottle_Cap_11",
	"Bottle_Cap_12",
	"Bottle_Cap_13",
	"Bottle_Cap_14",
	"Bottle_Cap_15",
	"Bottle_Cap_16",
	"Bottle_Cap_17",
	"Bottle_Cap_18",
	"Bottle_Cap_19",
	"Bottle_Cap_20",
	"Bottle_Cap_21",
	"Bottle_Cap_22",
	"Bottle_Cap_23",
	"Bottle_Cap_24",
	"File_22",
	"File_23",
	"File_24",
	"File_25",
	"File_26",
	"File_27",
	"File_28",
	"File_29",
	"File_30",
	"File_31",
	"Tactical Vest",
	"Any"
};

// Descriptions of the known effects of game flags can be added here
// Even if effect isn't useful or not fully understood, it should be documented here
// Trainer FlagEditor page will display these descriptions when hovering over flags
// and will also gray out any flags that are missing descriptions from this section (& optionally filter them out)
// If flag has been reimplemented by RE4T that can also be mentioned in the description ("re4_tweaks reimplementation")
// The flag will then be colored gold in the FlagEditor to show that it's been added by us
std::unordered_map<int, std::string> Flags_DEBUG_Descriptions = {
	{int(Flags_DEBUG::DBG_SAT_DISP), "Displays SAT collision polygons (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_EAT_DISP), "Displays EAT collision polygons (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_NO_DEATH2), "Invulnerability (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_EM_WEAK), "Weakens enemies, making them die in one or two hits (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_INF_BULLET), "Prevents ammo from being removed after firing (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_EM_NO_ATK), "Prevents enemies from becoming hostile to player (Em10 only) (re4_tweaks reimplementation)"},
	{int(Flags_DEBUG::DBG_NO_PARASITE), "Prevents Em10 enemies from spawning parasites (? untested)"},
	{int(Flags_DEBUG::DBG_EM_NO_DEATH), "Prevents Em25 enemies from dying  (? untested)"},
	{int(Flags_DEBUG::DBG_DBG_CAM), "Prevents camera from being changed during CameraMove (? untested)"},
	{int(Flags_DEBUG::DBG_LOG_OFF), "Prevents cLog messages from being printed (? untested)"},
	{int(Flags_DEBUG::DBG_TEST_MODE), "Enables certain debug-related code paths (? untested)"},
	{int(Flags_DEBUG::DBG_TEST_MODE_CK), "Enables certain debug-related code paths (? untested)"},
	{int(Flags_DEBUG::DBG_EFF_NUM_DISP), "Allows EFF debug messages to be printed (? untested)"},
	{int(Flags_DEBUG::DBG_FOG_FAR_GREEN), "Changes fog background color to green (? untested)"},
	{int(Flags_DEBUG::DBG_ROOMJMP), "Disables certain code during room jump (? untested)"},
	{int(Flags_DEBUG::DBG_CINESCO_OFF), "Disables cinesco processing (? untested)"},
	{int(Flags_DEBUG::DBG_SLOW_MODE), "Changes GXCopyDisp call during Render_done (? untested)"},
	{int(Flags_DEBUG::DBG_PL_LOCK_FOLLOW), "Actives weapon auto-track (? untested)"},
	{int(Flags_DEBUG::DBG_NO_SCE_EXE), "Disables room/scenario code from executing (? untested)"},
	{int(Flags_DEBUG::DBG_EVENT_TOOL), "Allows event related debug messages to be shown (? untested)"},
	{int(Flags_DEBUG::DBG_DOOR_SET_MODE), "Changes memory access patterns? (? untested)"},
	{int(Flags_DEBUG::DBG_WARN_LEVEL_LOW), "Debug message level to low? (? untested)"},
	{int(Flags_DEBUG::DBG_EMW_ERR_NO_DISP), "Prevents cEmWrap debug messages from being printed (? untested)"},
	{int(Flags_DEBUG::DBG_BGM_STOP), "Prevents BGM audio from playing (? untested)"}
};

std::unordered_map<int, std::string> Flags_STOP_Descriptions = {
	{int(Flags_STOP::SPF_CAMERA), "Pause camera processing"},
	{int(Flags_STOP::SPF_EM), "Pause enemy (Em) processing"},
	{int(Flags_STOP::SPF_PL), "Pause player processing"},
	{int(Flags_STOP::SPF_OBJ), "Pause equipped enemy objects"},
	{int(Flags_STOP::SPF_SUBCHAR), "Pause sub-character (Ashley) processing"},
	{int(Flags_STOP::SPF_ESP), "Pause effect processing"},
	{int(Flags_STOP::SPF_ESP_AREA), "Pause EspArea processing (? untested)"},
	{int(Flags_STOP::SPF_WATER), "Pause water processing (? untested)"},
	{int(Flags_STOP::SPF_EVT), "Skip events/cutscenes (? untested)"},
	{int(Flags_STOP::SPF_SCE), "Pause scenario/room processing"},
	{int(Flags_STOP::SPF_LIGHT), "Pause light processing"},
	{int(Flags_STOP::SPF_BLOCK), "Pause block-area processing"},
	{int(Flags_STOP::SPF_ACTBTN), "Pause action-button display"},
	{int(Flags_STOP::SPF_ID_SYSTEM), "Pause UI updating"},
	{int(Flags_STOP::SPF_SCE_AT), "Pauses AEV events from activating"},
};

std::unordered_map<int, std::string> Flags_STATUS_Descriptions = {
	{int(Flags_STATUS::STA_SUSPEND), "Suspends all object/actor processing (except objects marked NO_SUSPEND)"},
	{int(Flags_STATUS::STA_THERMO_GRAPH), "Toggles thermal scope effect"},
	{int(Flags_STATUS::STA_ITEM_GET), "Simplifies game scene for item pickup screen"},
	{int(Flags_STATUS::STA_NO_FENCE), "Disables fence jump command"},
	{int(Flags_STATUS::STA_SLOW), "Toggles slow-motion game effect"},
	{int(Flags_STATUS::STA_KLAUSER_TRANSFORM), "Changes camera to act like transformed Krauser"},
};

std::unordered_map<int, std::string> Flags_SYSTEM_Descriptions = {
	{int(Flags_SYSTEM::SYS_SOFT_RESET), "Resets game back to main menu"},
	{int(Flags_SYSTEM::SYS_TRANS_STOP), "Pauses rendering updates on game objects"},
	{int(Flags_SYSTEM::SYS_SCREEN_STOP), "Pauses screen frame updates"},
};

std::unordered_map<int, std::string> Flags_ITEM_SET_Descriptions = {
};

std::unordered_map<int, std::string> Flags_SCENARIO_Descriptions = {
};

std::unordered_map<int, std::string> Flags_KEY_LOCK_Descriptions = {
};

std::unordered_map<int, std::string> Flags_EXTRA_Descriptions = {
	{int(Flags_EXTRA::EXT_COSTUME), "Unlocks \"Special\" costume for Leon, \"Extra\" menu on main menu, and makes menu background scroll"},
	{int(Flags_EXTRA::EXT_COSTUME_MAFIA), "Unlocks \"Special 2\" / Mafia costume for Leon"},
	{int(Flags_EXTRA::EXT_HARD_MODE), "Unlocks \"Professional\" difficulty mode"},
	{int(Flags_EXTRA::EXT_GET_ADAS_REPORT), "Unlocks \"Ada's Report\" availability"},
	{int(Flags_EXTRA::EXT_GET_ADA), "Unlocks Ada for \"The Mercenaries\" game mode"},
	{int(Flags_EXTRA::EXT_GET_HUNK), "Unlocks HUNK for \"The Mercenaries\" game mode"},
	{int(Flags_EXTRA::EXT_GET_KLAUSER), "Unlocks Krauser for \"The Mercenaries\" game mode"},
	{int(Flags_EXTRA::EXT_GET_WESKER), "Unlocks Wesker for \"The Mercenaries\" game mode"},

	{int(Flags_EXTRA::EXT_GET_SW500), "Makes Handcannon weapon available from merchant during main game mode"},
	{int(Flags_EXTRA::EXT_GET_TOMPSON), "Makes Chicago Typewriter weapon available from merchant during main game mode"},
	{int(Flags_EXTRA::EXT_GET_LASER), "Makes P.R.L 412 weapon available from merchant during main game mode"},
	{int(Flags_EXTRA::EXT_GET_ADA_TOMPSON), "Makes Chicago Typewriter weapon available during \"Separate Ways\" game mode"},
};

std::unordered_map<int, std::string> Flags_CONFIG_Descriptions = {
	{int(Flags_CONFIG::CFG_AIM_REVERSE), "Toggles inverted aiming"},
	{int(Flags_CONFIG::CFG_WIDE_MODE), "Toggles wide-screen mode (? untested)"},
	{int(Flags_CONFIG::CFG_LOCK_ON), "Toggles auto-aiming mode (? untested)"},
	{int(Flags_CONFIG::CFG_KNIFE_MODE), "Toggles knife availability (? untested)"},
	{int(Flags_CONFIG::CFG_VIBRATION), "Toggles controller vibration"},
};

std::unordered_map<int, std::string> Flags_DISP_Descriptions = {
	{int(Flags_DISP::DPF_EM), "Toggles enemy (Em) rendering"},
	{int(Flags_DISP::DPF_PL), "Toggles player rendering"},
	{int(Flags_DISP::DPF_SUBCHAR), "Toggles sub-character (Ashley) rendering"},
	{int(Flags_DISP::DPF_OBJ), "Toggles equipment rendering"},
	{int(Flags_DISP::DPF_SCR), "Toggles scenery rendering"},
	{int(Flags_DISP::DPF_ESP), "Toggles effect rendering"},
	{int(Flags_DISP::DPF_TEX_RENDER), "Toggles TexRender effect rendering"},
	{int(Flags_DISP::DPF_FILTER), "Toggles post-process rendering"},
	{int(Flags_DISP::DPF_GLB_ILM), "Toggles global illumination rendering"},
	{int(Flags_DISP::DPF_COCKPIT), "Toggles the in-game HUD display"},
	{int(Flags_DISP::DPF_FOG), "Toggles fog rendering"},
	{int(Flags_DISP::DPF_ID_SYSTEM), "Toggles UI rendering"},
	{int(Flags_DISP::DPF_MESSAGE), "Toggles key prompt rendering"},
	{int(Flags_DISP::DPF_WATER), "Toggles water rendering"},
	{int(Flags_DISP::DPF_CLOTH), "Toggles cloth rendering"},
	{int(Flags_DISP::DPF_SHADOW), "Toggles shadow rendering"},
	// {int(Flags_DISP::DPF_CTRL), "Toggles cCtrl rendering"}, // does nothing? 
};

std::unordered_map<int, std::string> Flags_ROOM_SAVE_Descriptions = {
};

std::unordered_map<int, std::string> Flags_ROOM_Descriptions = {
};
