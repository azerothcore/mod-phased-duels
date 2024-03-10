/*
Credits:
Rochet2
Tommy

There were few more, but couldnt find them

Release:
http://emudevs.com/showthread.php/3413-Phased-duel-(pets-work)?p=24109
Original source:
http://emudevs.com/showthread.php/2282-phase-out-dueling-error?p=15483&viewfull=1#post15483
*/
#include "Configuration/Config.h"
#include "Object.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "GridNotifiers.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "SpellAuraEffects.h"

class PhasedDueling : public PlayerScript
{
public:
    PhasedDueling(): PlayerScript("PhasedDueling") {}

    void OnLogin(Player* pPlayer) override
    {
        if (sConfigMgr->GetOption<bool>("PhasedDuelsAnnounce.Enable", true))
        {
            ChatHandler(pPlayer->GetSession()).SendSysMessage("This server is running the |cff4CFF00PhasedDuels |rmodule.");
        }
    }

    void OnDuelStart(Player* firstplayer, Player* secondplayer) override
    {
        if (sConfigMgr->GetOption<bool>("PhasedDuels.Enable", true))
        {
            Map* map = firstplayer->GetMap();
            if (map->IsDungeon())
                return;

            // Duel flag is used as duel center point
            GameObject* go = map->GetGameObject(firstplayer->GetGuidValue(PLAYER_DUEL_ARBITER));
            if (!go)
                return;

            // Get players from 100 yard radius ( duel radius is 40-50 yd )
            std::list<Player*> playerList;
            Acore::AnyPlayerInObjectRangeCheck checker(go, 100.0f);
            Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(go, playerList, checker);
            go->VisitNearbyWorldObject(100.0f, searcher);

            // insert players' phases to used phases, ignore GMs
            uint32 usedPhases = 0;
            if (!playerList.empty())
                for (std::list<Player*>::const_iterator it = playerList.begin(); it != playerList.end(); ++it)
                    if (!(*it)->IsGameMaster())
                        usedPhases |= (*it)->GetPhaseMask();
            // loop all unique phases
            for (uint32 phase = 2; phase <= UINT_MAX / 2; phase *= 2)
            {
                // If phase in use, skip
                if (usedPhases & phase)
                    continue;

                // Phase players & pets, dont update visibility yet
                firstplayer->SetPhaseMask(phase, false);
                secondplayer->SetPhaseMask(phase, false);
                // Phase duel flag
                go->SetPhaseMask(phase, true);
                // Update visibility here so pets will be phased and wont despawn
                firstplayer->UpdateObjectVisibility();
                secondplayer->UpdateObjectVisibility();
                return;
            }

            // Couldnt find free unique phase
            firstplayer->GetSession()->SendNotification("There are no free phases");
            secondplayer->GetSession()->SendNotification("There are no free phases");
        }
    }

    // Restore phases
    void OnDuelEnd(Player* firstplayer, Player* secondplayer, DuelCompleteType /* type */) override // unusued param type
    {
        if (sConfigMgr->GetOption<bool>("PhasedDueling.Enable", true))
        {
            // Phase players, dont update visibility yet
            firstplayer->SetPhaseMask(GetNormalPhase(firstplayer), false);
            secondplayer->SetPhaseMask(GetNormalPhase(secondplayer), false);
            // Update visibility here so pets will be phased and wont despawn
            firstplayer->UpdateObjectVisibility();
            secondplayer->UpdateObjectVisibility();

            if (sConfigMgr->GetOption<bool>("SetMaxHP.Enable", true))
            {
                firstplayer->SetHealth(firstplayer->GetMaxHealth());
                secondplayer->SetHealth(secondplayer->GetMaxHealth());
            }

            if (sConfigMgr->GetOption<bool>("ResetCoolDowns.Enable", true))
            {
                firstplayer->RemoveAllSpellCooldown();
                secondplayer->RemoveAllSpellCooldown();
            }

            if (sConfigMgr->GetOption<bool>("RestorePower.Enable", true))
            {
                if (!sConfigMgr->GetOption<bool>("RetorePowerForRogueOrWarrior.Enable", true))
                {
                    if (firstplayer->getClass() == CLASS_ROGUE || firstplayer->getClass() == CLASS_WARRIOR)
                        return;

                    if (secondplayer->getClass() == CLASS_ROGUE || secondplayer->getClass() == CLASS_WARRIOR)
                        return;
                }

                firstplayer->SetPower(firstplayer->getPowerType(), firstplayer->GetMaxPower(firstplayer->getPowerType()));
                secondplayer->SetPower(secondplayer->getPowerType(), secondplayer->GetMaxPower(secondplayer->getPowerType()));
            }

            if (sConfigMgr->GetOption<bool>("ReviveOrRestorPetHealth.Enable", true))
            {
                Pet* pet1 = firstplayer->GetPet();
                Pet* pet2 = secondplayer->GetPet();

                if (!pet1 || !pet2)
                    return;

                if (!pet1->IsAlive()  || !pet2->IsAlive())
                {
                    if (firstplayer->getClass() == CLASS_HUNTER || secondplayer->getClass() == CLASS_HUNTER)
                    {
                        pet1->SetPower(POWER_HAPPINESS, pet1->GetMaxPower(POWER_HAPPINESS));
                        pet2->SetPower(POWER_HAPPINESS, pet2->GetMaxPower(POWER_HAPPINESS));
                    }
                    pet1->setDeathState(ALIVE);
                    pet2->setDeathState(ALIVE);
                }

                pet1->SetHealth(pet1->GetMaxHealth());
                pet2->SetHealth(pet2->GetMaxHealth());


            }
        }
    }

    // Attempt in storing the player phase with spell phases included.
    uint32 GetNormalPhase(Player* player) const
    {
        if (player->IsGameMaster())
            return uint32(PHASEMASK_ANYWHERE);

        // GetPhaseMaskForSpawn copypaste
        uint32 phase = PHASEMASK_NORMAL;
        Player::AuraEffectList const& phases = player->GetAuraEffectsByType(SPELL_AURA_PHASE);
        if (!phases.empty())
            phase = phases.front()->GetMiscValue();
        if (uint32 n_phase = phase & ~PHASEMASK_NORMAL)
            return n_phase;

        return PHASEMASK_NORMAL;
    }
};

void AddPhasedDuelsScripts()
{
    new PhasedDueling();
}
