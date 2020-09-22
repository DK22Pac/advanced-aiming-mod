#include "plugin.h"
#include "game_sa\common.h"
#include "game_sa\CCam.h"
#include "CRunningScript.h"

using namespace plugin;

class AdvancedAimingMod {
public:
	enum eWeaponAimOffset {
		AIM_OFFSET_WEAPON_DEFAULT,
		AIM_OFFSET_WEAPON_COLT45,
		AIM_OFFSET_WEAPON_SILENCED,
		AIM_OFFSET_WEAPON_DESERTEAGLE,
		AIM_OFFSET_WEAPON_CHROMEGUN,
		AIM_OFFSET_WEAPON_SAWNOFF,
		AIM_OFFSET_WEAPON_SHOTGSPA,
		AIM_OFFSET_WEAPON_MICROUZI,
		AIM_OFFSET_WEAPON_MP5,
		AIM_OFFSET_WEAPON_TEC9,
		AIM_OFFSET_WEAPON_AK47,
		AIM_OFFSET_WEAPON_M4,
		AIM_OFFSET_WEAPON_CUNTGUN,
		AIM_OFFSET_WEAPON_ROCKETLA,
		AIM_OFFSET_WEAPON_HEATSEEK,
		AIM_OFFSET_WEAPON_FLAME
	};

	struct tAimingCamData {
		float f0, f1, f2, f3, f4, f5, f6;
	};

	struct tAimingOffsetData {
		int weaponId;
		CVector offset;
	};

	static tAimingCamData gData[4];

	static std::list<tAimingOffsetData*> offsets;

	static unsigned int updateSets;

	static bool holdingCommandLastFrame;

	static unsigned int keyId1;
	static unsigned int keyId2;
	static unsigned int buttonId1;
	static unsigned int buttonId2;

	static bool rightSide;

	static CVector defaultOffset;

	static CVector GetAimingOffsetData(int weaponId) {
		// Find weapon
		for (auto data : offsets) {
			if (data->weaponId == weaponId) return data->offset;
		}
		// Find default settings
		for (auto data : offsets) {
			if (data->weaponId == -1) return data->offset;
		}
		// Fall off
		CVector offset;
		offset.x = 0.0f; offset.y = 0.0f; offset.z = 0.0f;
		return offset;
	}

	static void ReadSettings() {
		char line[512];
		char dummy[128];
		float fVal;
		FILE *file = fopen(PLUGIN_PATH("aimingSets.dat"), "r");
		fgets(line, 512, file);
		sscanf(line, "%s %d", dummy, &updateSets);
		fgets(line, 512, file);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4B4);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4B8);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, &fVal);
		patch::SetFloat(0x521632, fVal);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4B0);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4AC);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4A8);
		fgets(line, 512, file);
		sscanf(line, "%s %f", dummy, (float *)0x8CC4A0);
		fgets(line, 512, file);
		fgets(line, 512, file);
		sscanf(line, "%s %d %d", dummy, &keyId1, &keyId2);
		fgets(line, 512, file);
		sscanf(line, "%s %d %d", dummy, &buttonId1, &buttonId2);
		fgets(line, 512, file);
		for (int i = 0; i < 4; i++) {
			fgets(line, 512, file);
			sscanf(line, "%s %f %f %f %f %f %f %f", dummy, &gData[i].f0, &gData[i].f1, &gData[i].f2, &gData[i].f3, &gData[i].f4,
				   &gData[i].f5, &gData[i].f6);
		}
		memcpy((void *)0x8CC4C0, gData, 112);
		fgets(line, 512, file);
		CVector offset;
		int weaponId = 0;
		for (int i = 0; i < 16; i++) {
			fgets(line, 512, file);
			if (sscanf(line, "%d %f %f %f", &weaponId, &offset.x, &offset.y, &offset.z) == 4) {
				tAimingOffsetData *data = new tAimingOffsetData;
				data->weaponId = weaponId;
				data->offset = offset;
				offsets.push_back(data);
			}
		}
		fclose(file);
	}

	static void _fastcall MyProcess_AimWeapon(CCam *cam, int, CVector const &vec, float arg3, float arg4, float arg5) {

		// check for commands

		if (buttonId1 != buttonId2) {
			if (buttonId2 != 0 && reinterpret_cast<CRunningScript*>(0)->GetPadState(0, buttonId2))
				rightSide = false;
			else if (buttonId1 != 0 && reinterpret_cast<CRunningScript*>(0)->GetPadState(0, buttonId1))
				rightSide = true;
		}
		else {
			if (buttonId1 != 0) {
				if (reinterpret_cast<CRunningScript*>(0)->GetPadState(0, buttonId1)) {
					if (!holdingCommandLastFrame) {
						rightSide = !rightSide;
						holdingCommandLastFrame = true;
					}
				}
				else holdingCommandLastFrame = false;
			}
		}

		if (keyId1 != keyId2) {
			if (keyId2 != 0 && KeyPressed(keyId2))
				rightSide = false;
			else if (keyId1 != 0 && KeyPressed(keyId1))
				rightSide = true;
		}
		else {
			if (keyId1 != 0) {
				if (KeyPressed(keyId1)) {
					if (!holdingCommandLastFrame) {
						rightSide = !rightSide;
						holdingCommandLastFrame = true;
					}
				}
				else holdingCommandLastFrame = false;
			}
		}

		if (updateSets)
			ReadSettings();

		CPed *playa = FindPlayerPed();
		if (playa && !playa->m_nPedFlags.bInVehicle) {

			CVector offset = GetAimingOffsetData(playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nType);
			
			if (offset.x != 0 || offset.y != 0 || offset.z != 0) {

				// if rightSide flag enabled
				if (rightSide)
					offset.x *= -3.0f;

				CallMethod<0x521500, CCam *, CVector const&>(cam, playa->TransformFromObjectSpace(offset), arg3, arg4, arg5);
			}
			else
				CallMethod<0x521500, CCam *, CVector const&>(cam, vec, arg3, arg4, arg5);
		}
		else
			CallMethod<0x521500, CCam *, CVector const&>(cam, vec, arg3, arg4, arg5);
	}

	AdvancedAimingMod() {
		// inject our function
		patch::RedirectCall(0x527A95, MyProcess_AimWeapon);
		// read settings
		ReadSettings();
	}
};

AdvancedAimingMod::tAimingCamData AdvancedAimingMod::gData[4];

std::list<AdvancedAimingMod::tAimingOffsetData*> AdvancedAimingMod::offsets;

CVector AdvancedAimingMod::defaultOffset;

unsigned int AdvancedAimingMod::updateSets;

bool AdvancedAimingMod::rightSide = false;

bool AdvancedAimingMod::holdingCommandLastFrame;

unsigned int AdvancedAimingMod::keyId1;
unsigned int AdvancedAimingMod::keyId2;
unsigned int AdvancedAimingMod::buttonId1;
unsigned int AdvancedAimingMod::buttonId2;

AdvancedAimingMod plg;
