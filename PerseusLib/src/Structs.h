//
// Created by shirome on 19/04/2022.
// Added json support by pie on 25/5/2024.
//

#ifndef ANDROID_HOOKING_PATCHING_TEMPLATE_MAIN_STRUCTS_H
#define ANDROID_HOOKING_PATCHING_TEMPLATE_MAIN_STRUCTS_H

#include "lua/lstate.h"
#include "Includes/json.hpp"

using ordered_json = nlohmann::ordered_json;

typedef struct Il2CppDomain
{
    void* domain;
    void* setup;
    void* default_context;
    const char* friendly_name;
    uint32_t domain_id;
} Il2CppDomain;

typedef struct Il2CppAssembly
{
    void* imageIndex;
    void* customAttributeIndex;
    int32_t referencedAssemblyStart;
    int32_t referencedAssemblyCount;
    void* aname;
} Il2CppAssembly;

typedef struct Il2CppImage
{
    const char* name;
    void* assemblyIndex;
    void* typeStart;
    uint32_t typeCount;
    void* entryPointIndex;
    void* nameToClassHashTable;
    uint32_t token;
} Il2CppImage;

typedef void(*Il2CppMethodPointer)();

typedef struct MethodInfo
{
    Il2CppMethodPointer* methodPointer;
    void* invoker_method;
    const char* name;
    void* declaring_type;
    const void* return_type;
    const void* parameters;

    union
    {
        const void* rgctx_data;
        const void* methodDefinition;
    };

    union
    {
        const void* genericMethod;
        const void* genericContainer;
    };

    void* customAttributeIndex;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint8_t parameters_count;
    uint8_t is_generic : 1;
    uint8_t is_inflated : 1;
} MethodInfo;

// define il2cpp functions
Il2CppDomain* (*il2cpp_domain_get)();
Il2CppAssembly* (*il2cpp_domain_assembly_open)(void* domain, char* name);
Il2CppImage* (*il2cpp_assembly_get_image)(void* assembly);
void* (*il2cpp_class_from_name)(void* image, char* namespaze, char* name);
MethodInfo* (*il2cpp_class_get_method_from_name)(void* klass, char* name, int argsCount);
Il2CppString* (*il2cpp_string_new)(char* str);

// mod structures
struct Config {
    struct AAircraft {
        bool Enabled = false;
        int Accuracy = -1;
        int AccuracyGrowth = -1;
        int AttackPower = -1;
        int AttackPowerGrowth = -1;
        int CrashDamage = -1;
        int Hp = -1;
        int HpGrowth = -1;
        int Speed = -1;
    };

    struct EEnemies {
        bool Enabled = false;
        int AntiAir = -1;
        int AntiAirGrowth = -1;
        int AntiSubmarine = -1;
        int Armor = -1;
        int ArmorGrowth = -1;
        int Cannon = -1;
        int CannonGrowth = -1;
        int Evasion = -1;
        int EvasionGrowth = -1;
        int Hit = -1;
        int HitGrowth = -1;
        int Hp = -1;
        int HpGrowth = -1;
        int Luck = -1;
        int LuckGrowth = -1;
        int Reload = -1;
        int ReloadGrowth = -1;
        bool RemoveBuffs = false;
        bool RemoveEquipment = false;
        bool RemoveSkill = false;
        int Speed = -1;
        int SpeedGrowth = -1;
        int Torpedo = -1;
        int TorpedoGrowth = -1;
    };

    struct MMisc {
        bool Enabled = false;
        bool ExerciseGodmode = false;
        bool FastStageMovement = false;
        bool Skins = false;
        int AutoRepeatLimit = -1;
        bool ChatCommands = false;
        bool RemoveBBAnimation = false;
        bool RemoveMoraleWarning = false;
        bool RemoveHardModeStatLimit = false;
        bool RemoveNSFWArts = false;
        bool AllBlueprintsConvertible = false;
    };

    struct WWeapons {
        bool Enabled = false;
        int Damage = -1;
        int ReloadMax = -1;
    };
    
    struct SSpoof {
        bool Enabled = false;

        std::string name = "";
        std::string id = "";
        double lv = 0;
        
        // internal values
        int maxCoins = 0;
        int maxOil = 0;
        int lvInt = 0;
        double lvFrac = 0;
        int lvMax = 0;
        int spoofXp = 0;
    };


    AAircraft Aircraft;
    EEnemies Enemies;
    WWeapons Weapons;
    SSpoof Spoof;
    MMisc Misc;
    bool Valid = true;
};

auto _default_raw = OBFUSCATE( R"r({
    "OriginalRepo": "github.com/Egoistically/Perseus",
    "PieRepo": "github.com/4pii4/PiePerseus",

    "Aircraft": {
        "Enabled": false,
        "Accuracy": -1,
        "AccuracyGrowth": -1,
        "AttackPower": -1,
        "AttackPowerGrowth": -1,
        "CrashDamage": -1,
        "Hp": -1,
        "HpGrowth": -1,
        "Speed": -1
    },

    "Enemies": {
        "Enabled": false,
        "AntiAir": -1,
        "AntiAirGrowth": -1,
        "AntiSubmarine": -1,
        "Armor": -1,
        "ArmorGrowth": -1,
        "Cannon": -1,
        "CannonGrowth": -1,
        "Evasion": -1,
        "EvasionGrowth": -1,
        "Hit": -1,
        "HitGrowth": -1,
        "Hp": -1,
        "HpGrowth": -1,
        "Luck": -1,
        "LuckGrowth": -1,
        "Reload": -1,
        "ReloadGrowth": -1,
        "RemoveBuffs": false,
        "RemoveEquipment": false,
        "RemoveSkill": false,
        "Speed": -1,
        "SpeedGrowth": -1,
        "Torpedo": -1,
        "TorpedoGrowth": -1
    },

    "Weapons": {
        "Enabled": false,
        "Damage": -1,
        "ReloadMax": -1
    },
    
    "Spoof": {
        "Enabled": false,
        "Name": "",
        "Id": "",
        "Lv": 0
    },

    "Misc": {
        "Enabled": false,
        "ExerciseGodmode": false,
        "FastStageMovement": false,
        "Skins": false,
        "AutoRepeatLimit": -1,
        "ChatCommands":  false,
        "RemoveBBAnimation": false,
        "RemoveMoraleWarning": false,
        "RemoveHardModeStatLimit": false,
        "RemoveNSFWArts": false,
        "AllBlueprintsConvertible": false
    }
}
)r");

auto _default = ordered_json::parse(static_cast<std::string>(_default_raw));

#endif //ANDROID_HOOKING_PATCHING_TEMPLATE_MAIN_STRUCTS_H
