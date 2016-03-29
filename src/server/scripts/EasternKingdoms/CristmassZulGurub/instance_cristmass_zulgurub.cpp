/*
* Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "CristmassZulGurub.h"

class instance_cristmass_zulgurub : public InstanceMapScript
{
public:
    instance_cristmass_zulgurub() : InstanceMapScript("instance_cristmass_zulgurub", 309) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_cristmass_zulgurub_InstanceMapScript(map);
    }

    struct instance_cristmass_zulgurub_InstanceMapScript : public InstanceScript
    {
        instance_cristmass_zulgurub_InstanceMapScript(Map* map) : InstanceScript(map) {}
        std::set<ObjectGuid> guids;
        uint32 m_auiEncounter[MAX_TYPES];

        void Initialize()
        {
            SetBossNumber(encounternumber);
            //LoadDoorData(doorData);
            guids.clear();
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
            case NPC_DOOM_LORD:
                doomlordGUID = creature->GetGUID();
                break;
            case NPC_PIT_LORD:
                pitlordGUID = creature->GetGUID();
                break;
            case NPC_JUBEKA_SHADOWBREAKER:
                jubekaGUID = creature->GetGUID();
                break;
            case NPC_KANRETHAD_EBONLOCKE:
                kanretadGUID = creature->GetGUID();
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
            case BOSS_DOOM_LORD:
                return doomlordGUID;
            case BOSS_PIT_LORD:
                return pitlordGUID;
            case BOSS_JUBEKA:
                return jubekaGUID;
            case BOSS_KANRETHAD:
                return kanretadGUID;
            default:
                break;
            }
            return emptyGUID;
        }

        uint64 GetData64(uint32 type)
        {
            switch (type)
            {
            case BOSS_DOOM_LORD:
                return doomlordGUID;
            case BOSS_PIT_LORD:
                return pitlordGUID;
            case BOSS_JUBEKA:
                return jubekaGUID;
            case BOSS_KANRETHAD:
                return kanretadGUID;
            }

            return 0;
        }

        std::string GetSaveData()
        {
            std::ostringstream saveStream;
            saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' '
                << m_auiEncounter[6] << ' ' << m_auiEncounter[7] << ' ' << m_auiEncounter[8] << ' '
                << m_auiEncounter[9] << ' ' << m_auiEncounter[10] << ' ' << m_auiEncounter[11] << ' '
                << m_auiEncounter[12] << ' ' << m_auiEncounter[13];
            return saveStream.str();
        }

        bool isWipe()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();

            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    if (Player* plr = i->GetSource())
                        if (plr->IsAlive() && !plr->isGameMaster())
                            return false;
                }
            }
            return true;
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            return true;
        }
    protected:
        ObjectGuid doomlordGUID;
        ObjectGuid pitlordGUID;
        ObjectGuid jubekaGUID;
        ObjectGuid kanretadGUID;
        ObjectGuid emptyGUID;
        ObjectGuid emptyCreature;

        void Load(const char* chrIn)
        {
            if (!chrIn)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(chrIn);
            std::istringstream loadStream(chrIn);

            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
                >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
                >> m_auiEncounter[12] >> m_auiEncounter[13];

            // Do not load an encounter as "In Progress" - reset it instead.
            for (uint8 i = 0; i < MAX_TYPES; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    m_auiEncounter[i] = NOT_STARTED;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_cristmass_zulgurub ()
{
    new instance_cristmass_zulgurub();
}