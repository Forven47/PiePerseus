#pragma once

#include "../Structs.h"

extern struct Config config;

#define tostr(x) (static_cast<std::string>(x))
#define stdstr(x) (static_cast<std::string>(OBFUSCATE(x)))
#define STR(str) il2cpp_string_new(OBFUSCATE(str))

Il2CppString *stdstr2ilstr(std::string s);
void luaMessageBox(lua_State *L, std::string msg);
void luaToast(lua_State *L, std::string msg);
void percyLog(const char *fmt, ...);
void crash();
std::vector<std::string> split(const std::string &s, char delim);
void parseLv(lua_State *L, double input); // fractional cmder level
double stod_safe(std::string s, bool &success);

const char *helpmsg = OBFUSCATE(R"r(Commands help:
    kill/crash: stops the game
    spoof name/lv/id: toggles the specified spoof
    spoof name/lv/id <value>: changes the value of the specified spoof
)r");

double random(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

double stod_safe(std::string s, bool& success) {
    double d;
    std::istringstream stream(s);
    
    stream >> d;

    if (!stream.fail() && stream.eof()) {
        success = true;
        return d;
    } else {
        success = false;
        return 0;
    }
}

void parseLv(lua_State *L, double input) {
    if (input <= 0) {
        // leave at zero, no spoof
        // just fill in the proper resource caps
        
        // call getProxy(PlayerProxy)
        lua_getglobal(L, STR("getProxy"));
        lua_getglobal(L, STR("PlayerProxy"));
        lua_pcall(L, 1, 1, 0);
        
        // call PlayerProxy:getRawData()
        lua_getfield(L, -1, STR("getRawData"));
        lua_insert(L, -2);
        lua_pcall(L, 1, 1, 0);
        
        // assign values
        lua_getfield(L, -1, STR("getLevelMaxGold"));
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 1, 0);
        config.Spoof.maxCoins = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, STR("getLevelMaxOil"));
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 1, 0);
        config.Spoof.maxOil = lua_tonumber(L, -1);
        lua_pop(L, 1);
        
        return;
    } 
    double levelInput;
    if (input == -1.0) {
        // randomise
        levelInput = random(10.0, 125.0);
    } else {  
        levelInput = input;
    }
    // parse integer portion into an int
    double levelInt;
    config.Spoof.lvFrac = std::modf(levelInput, &levelInt);
    config.Spoof.lvInt = static_cast<int>(levelInt);
    
    // convert fractional portion into an appropriate int:
    // call getConfigFromLevel1 on spoofed level to get xp in level...
    lua_getglobal(L, STR("getConfigFromLevel1"));
    lua_getglobal(L, STR("pg"));
    lua_getfield(L, -1, STR("user_level"));
    lua_remove(L, -2);
    lua_pushnumber(L, config.Spoof.lvInt);
    lua_pcall(L, 2, 1, 0);
    lua_getfield(L, -1, STR("exp_interval"));
    
    // convert fraction of level covered to integer xp
    config.Spoof.lvMax = static_cast<int>(lua_tonumber(L, -1));
    config.Spoof.spoofXp = std::floor(config.Spoof.lvFrac * config.Spoof.lvMax);
    lua_pop(L, 1);
    
    // save max_gold and max_oil
    lua_getfield(L, -1, STR("max_gold"));
    config.Spoof.maxCoins = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, STR("max_oil"));
    config.Spoof.maxOil = lua_tonumber(L, -1);
    lua_pop(L, 1);
    
    // emulate a random level of guild tech coin/oil expansion
    int guildTechLvl;
    if (config.Spoof.lv < 30) {
        guildTechLvl = std::floor(random(0.0, 6.0));
    } else if (config.Spoof.lv < 60) {
        guildTechLvl = std::floor(random(0.0, 16.0));
    } else if (config.Spoof.lv < 90) {
        guildTechLvl = std::floor(random(10.0, 36.0));
    } else {
        guildTechLvl = std::floor(random(15.0, 51.0));
    }
    config.Spoof.maxCoins += guildTechLvl * 300;
    config.Spoof.maxOil += guildTechLvl * 50;
    
    // pop result of getConfigFromLevel1
    lua_pop(L, 1);

    percyLog(OBFUSCATE("input is %lf"), levelInput);
    percyLog(OBFUSCATE("parsed into level %lf %d:%lf"), config.Spoof.lv, config.Spoof.lvInt, config.Spoof.lvFrac);
    percyLog(OBFUSCATE("%d/%d xp out of"), config.Spoof.spoofXp, config.Spoof.lvMax);
    percyLog(OBFUSCATE("%d max coins"), config.Spoof.maxCoins);
    percyLog(OBFUSCATE("%d max oil"), config.Spoof.maxOil);
}

void handleCommand(lua_State *L, std::string cmd) {
    percyLog(OBFUSCATE("command: %s"), cmd.c_str());
    std::vector<std::string> args = split(cmd, ' ');

    if (args[0] == stdstr("kill") || args[0] == stdstr("crash")) {
        crash();
    } else if (args[0] == stdstr("spoof")) {
        switch (args.size()) {
        case 1: {
            return;
        }

        case 2: {
            if (args[1] == stdstr("name")) {
                config.Spoof.name = "";
                luaToast(L, stdstr("disabling name spoof, go to another page with name to update it"));
            }
            else if (args[1] == stdstr("id")) {
                config.Spoof.id = "";
                luaToast(L, stdstr("disabling id spoof, go to another page with id to update it"));
            }
            else if (args[1] == stdstr("lv")) {
                config.Spoof.lv = -1;
                parseLv(L, config.Spoof.lv);
                luaToast(L, stdstr("disabling lv spoof, spend/get coins to update it"));
            }
            break;
        }
                
        case 3: {
            if (args[1] == stdstr("name")) {
                config.Spoof.name = args[2];
                luaToast(L, stdstr("go to another page with name to update it"));
            } else if (args[1] == stdstr("id")) {
                config.Spoof.id = args[2];
                luaToast(L, stdstr("go to another page with name to update it"));
            } else if (args[1] == stdstr("lv")) {
                bool success;
                double d;
                d = stod_safe(args[2], success);

                if (!success) {
                    luaToast(L, stdstr("invalid number"));
                    return;
                }
                
                config.Spoof.lv = d;
                parseLv(L, config.Spoof.lv);
                luaToast(L, stdstr("go spend/get some coins for the MAX texts to update it"));
            }
            break;
        }
                
        default: {
            luaToast(L, stdstr("invalid number of arguments, try .help"));
            break;
        }
        };
    } else if (args[0] == stdstr("toast") && args.size() == 2) {
        luaToast(L, args[1]);
    } else if (args[0] == stdstr("msgbox") && args.size() == 2) {
        luaMessageBox(L, args[1]);
    } else if (args[0] == stdstr("help")) {
        luaMessageBox(L, std::string(helpmsg));
    } else {
        luaToast(L, stdstr("unknown command, try .help"));
    }
}